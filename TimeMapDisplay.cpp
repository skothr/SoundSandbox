#include "TimeMapDisplay.h"

#include "AUtility.h"
#include "Timing.h"
#include "NodeBaseTypes.h"
#include "TrackNodes.h"
#include "TimeMapNode.h"

#include "AudioDataDisplay.h"
#include "MidiDataDisplay.h"

//#include "IONodes.h"

/////TIME MAP DISPLAY/////
const float TimeMapDisplay::SAMPLES_PER_PIXEL = 100.0f;	//Samples per pixel at zoom = 1.0f
const float TimeMapDisplay::TRACK_HEIGHT = 225.0f;
const AVec TimeMapDisplay::TRACK_PADDING(5.0f, 5.0f);

const float TimeMapDisplay::CURSOR_FOLLOW_PADDING = 100.0f;

const float TimeMapDisplay::SCROLL_MULT = 10.0f;
const AVec TimeMapDisplay::ZOOM_STEP(0.01f, 0.0f);

TimeMapDisplay::TimeMapDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, TimeMapNode *tm_node)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		ScrollArea(parent_, a_pos, a_size, s_flags),
		node(tm_node), trackInput(tm_node ? (tm_node->connectors[tm_node->INPUTS.AUDIO_ID]) : nullptr)
{
	setAllBgStateColors(Color(0.8f, 0.8f, 0.8f, 1.0f));
	setMinZoom(AVec(0.3f, 1.0f));
	setMaxZoom(AVec(SAMPLES_PER_PIXEL, 1.0f));	//Max zoom is 1 sample per pixel

	setZoomPivot(size*(1.0f/2.0f));

	setAllBgStateColors(Color(3.0f, 2.0f, 1.0f, 1.0f));

	initTracks();
}

TimeMapDisplay::~TimeMapDisplay()
{
	for(auto tc : tracks)
		AU::safeDelete(tc.second);

	tracks.clear();
}

//ASSUMES NO TRACK CONTROLS YET
void TimeMapDisplay::initTracks()
{
	if(node)
	{
		//Pixels per chunk
		const float v_ppc = (float)AUDIO_CHUNK_SIZE/(float)SAMPLES_PER_PIXEL,
					a_ppc = transform.virtualToAbsoluteVec(AVec(v_ppc, 0)).x;
		const AVec	t_track_padding = correctVirtualVec(TRACK_PADDING),
					t_tc_padding = correctVirtualVec(TrackControl::PADDING);
		
		AVec a_zoom = getZoom();

		//Get cursor
		cursor = node->getCursor();

		const std::unordered_map<NCID, AudioMidiPair> *c_buffers = node->getConnectedBuffers();
		
		//Add track controls
		float y_pos = t_track_padding.y;
		for(auto b : *c_buffers)
		{
			AudioMidiPair bp = b.second;

			APoint t_pos(t_track_padding.x, y_pos);
			AVec t_size(bp.audio->getLength()*v_ppc, TRACK_HEIGHT);

			TrackControl *new_tc = new TrackControl(this, t_pos, t_size, DEFAULT_STATE, bp.audio, bp.midi);

			tracks.emplace(bp.audio, new_tc);
			trackOrder.push_back(new_tc);
			addChild(new_tc, false);

			//Adjust y_pos
			y_pos += TRACK_HEIGHT + t_track_padding.y;
		}

		//Update widths of tracks to match AudioTrackNode lengths, and recalculate max offset
		c_time max_length = 0;
		for(auto tc : tracks)
		{
			c_time l = tc.second->getAudioNode()->getLength();
			max_length = (l > max_length ? l : max_length);
			
			tc.second->setWidth(l*v_ppc + t_tc_padding.x*2.0f);
		}
		
		APoint max_offset(max_length*a_ppc + (TRACK_PADDING.x + TrackControl::PADDING.x)*2.0f, y_pos);
		max_offset -= size;

		//Clamp to >= 0 and apply zoom
		max_offset.x *= (max_offset.x >= 0.0f);
		max_offset.y *= (max_offset.y >= 0.0f);

		setMaxViewOffset(max_offset);

		//Clear added/removed buffers
		node->getAddedBuffers()->clear();
		node->getRemovedBuffers()->clear();
	}
}

void TimeMapDisplay::updateTracks()
{
	if(node)
	{
		//Pixels per chunk
		const float v_ppc = (float)AUDIO_CHUNK_SIZE/(float)SAMPLES_PER_PIXEL,
					a_ppc = transform.virtualToAbsoluteVec(AVec(v_ppc, 0)).x;

		const AVec	t_track_padding = correctVirtualVec(TRACK_PADDING),
					t_tc_padding = correctVirtualVec(TrackControl::PADDING);

		AVec a_zoom = getZoom();

		//Get cursor
		cursor = node->getCursor();

		//Get added and removed buffers since last time
		std::vector<AudioMidiPair>	*added = node->getAddedBuffers(),
									*removed = node->getRemovedBuffers();

		//Remove specified node track controls
		for(auto bp : *removed)
		{
			auto tc_iter = tracks.find(bp.audio);
			
			//Only remove if track exists
			if(tc_iter != tracks.end())
			{
				TrackControl *tc = tc_iter->second;

				unsigned int index;
				for(index = 0; index < trackOrder.size(); index++)
				{
					if(trackOrder[index] == tc)
					{
						//Move tracks below removed track up to compensate for removed track
						for(unsigned int i = index + 1; i < trackOrder.size(); i++)
							trackOrder[i]->move(AVec(0.0f, -(TRACK_HEIGHT + t_track_padding.y)));
						break;
					}
				}

				removeChild(tc, false);
				delete tc;
				tracks.erase(bp.audio);
				trackOrder.erase(trackOrder.begin() + index);
			}
		}
		removed->clear();

		//Add specified node track controls
		float y_pos = t_track_padding.y + (TRACK_HEIGHT + t_track_padding.y)*tracks.size();
		for(auto bp : *added)
		{
			//Only add if tracks doesnt already have it
			if(tracks.find(bp.audio) == tracks.end())
			{
				APoint t_pos(t_track_padding.x, y_pos);
				AVec t_size(bp.audio->getLength()*v_ppc, TRACK_HEIGHT);

				TrackControl *new_tc = new TrackControl(this, t_pos, t_size, DEFAULT_STATE, bp.audio, bp.midi);

				tracks.emplace(bp.audio, new_tc);
				trackOrder.push_back(new_tc);
				addChild(new_tc, false);

				//Adjust y_pos
				y_pos += TRACK_HEIGHT + t_track_padding.y;
			}
		}
		added->clear();

		//Update widths of tracks to match AudioTrackNode lengths, and recalculate max offset
		c_time max_length = 0;
		for(auto tc : tracks)
		{
			float l = tc.second->getAudioNode()->getLength();

			tc.second->setWidth(l*v_ppc + t_tc_padding.x*2.0f);
			tc.second->setX(t_track_padding.x);

			max_length = (l > max_length ? l : max_length);
		}
		
		APoint max_offset(max_length*a_ppc + (TRACK_PADDING.x + TrackControl::PADDING.x)*2.0f, y_pos);
		max_offset -= size;
		
		//Clamp to >= 0
		max_offset.x *= (max_offset.x > 0.0f);
		max_offset.y *= (max_offset.y > 0.0f);

		setMaxViewOffset(max_offset);
	}
}

s_time TimeMapDisplay::getCursorPos()
{
	return (cursor ? cursor->getSampleRange().start : 0);
}

Cursor* TimeMapDisplay::getCursor()
{
	return cursor;
}

TimeMapNode* TimeMapDisplay::getNode()
{
	return node;
}


void TimeMapDisplay::setNode(TimeMapNode *tmn)
{
	if(node != tmn)
	{
		node = tmn;
		
		//Clear tracks
		clearChildren();
		for(auto tc : tracks)
			AU::safeDelete(tc.second);
		tracks.clear();
		trackOrder.clear();

		trackInput = (node ? node->connectors[node->INPUTS.AUDIO_ID] : nullptr);

		initTracks();
	}
	//else same node, dont do anything
}

AVec TimeMapDisplay::correctVirtualVec(const AVec &v) const
{
	return transform.absoluteToVirtualVec(v);
}

void TimeMapDisplay::onSizeChanged(AVec d_size)
{
	setZoomPivot(size*(1.0f/2.0f));
	moveViewOffset(d_size*(-1.0f/2.0f));
}

void TimeMapDisplay::onScroll(APoint m_pos, AVec d_scroll, bool direct)
{
	if(direct)
	{
		if(Keyboard::keyDown(Keys::K_LALT))
		{
			RVec a_zoom = getZoom();
			AVec mult = AVec(1.0f, 1.0f) - ZOOM_STEP*(d_scroll.x + d_scroll.y);
			setZoom(RVec(a_zoom.x*mult.x, a_zoom.y*mult.y));
			//moveZoom(ZOOM_STEP*(d_scroll.x + d_scroll.y));

			updateTracks();
		}
		else
		{
			moveViewOffset(AVec(SCROLL_MULT*(d_scroll.x + d_scroll.y), 0.0f));

			//Dont follow cursor after manual view offset
			followCursor = false;
		}
	}
}

void TimeMapDisplay::update(const Time &dt)
{
	if(node)// && node->isPlaying())
	{
		//Get corrected padding
		const float	t_padding = correctVirtualVec(TRACK_PADDING + TrackControl::PADDING).x,
					t_cfpadding = correctVirtualVec(AVec(CURSOR_FOLLOW_PADDING, 0.0f)).x;
		
		//Get position of the left/right edges in virtual space
		//APoint v_offset = getViewOffset();
		float	l_edge = t_cfpadding,//(v_offset.x > t_padding) ? v_offset.x : t_padding - v_offset.x,
				r_edge = transform.absoluteToVirtualPoint(AVec(size.x - CURSOR_FOLLOW_PADDING, 0.0f)).x;

		//Get cursor pos in virtual space
		const float	cursor_track_pos = (float)cursor->getSampleRange().start/SAMPLES_PER_PIXEL,	//Cursor pos relative to the audio display, in this virtual space
					cursor_pos = t_padding + cursor_track_pos;	//Actual cursor pos (still virtual space)

		//Snap display to cursor
		AVec d_offset(0, 0);
		if(cursor_pos >= r_edge)
		{
			//Shift view offset up if followCursor is true
			d_offset.x = (cursor_pos - r_edge)*followCursor;
		}
		/*
		else if(cursor_pos < l_edge)
		{
			//Shift view offset down if followCursor is true
			d_offset.x = (cursor_pos - l_edge)*followCursor;
		}
		*/

		moveViewOffset(d_offset);
	}
	else
		followCursor = true;
	
	updateTracks();
}

void TimeMapDisplay::draw(GlInterface &gl)
{
	if(node && visible && isolateViewport(gl))
	{
		update(0.0f);
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}



/////TRACK CONTROL/////
const GuiPropFlags TrackControl::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;

AVec TrackControl::PADDING = 5.0f;

TrackControl::TrackControl(TimeMapDisplay *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, AudioTrackNode *audio, MidiTrackNode *midi)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		tmd_parent(parent_), audioNode(audio), midiNode(midi)
{
	setAllBgStateColors(Color(0.3f, 0.3f, 0.3f, 1.0f));
	
	//Get correct padding to use (so its consistent in parents absolute space)
	AVec	t_padding = tmd_parent->correctVirtualVec(PADDING);

	std::vector<GuiElement*> children;
	float h = (size.y - 3.0f*t_padding.y)/2.0f;

	AVec b_disp_size = AVec(size.x - t_padding.x*2.0f, h);

	//Add audio display
	if(audioNode)
	{
		audioDisp = new AudioDataDisplay(this, APoint(t_padding), b_disp_size, DEFAULT_STATE, nullptr, GraphProps::NONE);
		audioDisp->setData((AudioAmpData*)(audioNode->getData()), false, false, true -1.0f);
		//audioDisp->setCursor(tmd_parent->getCursor(), 0.0);
		children.push_back(audioDisp);
	}

	if(midiNode)
	{
		midiDisp = new MidiDataDisplay(this, APoint(t_padding.x, t_padding.y*2.0f + h), b_disp_size, DEFAULT_STATE, nullptr, GraphProps::NONE);
		midiDisp->setData(midiNode->getData());
		//midiDisp->setCursor(tmd_parent->getCursor());
		children.push_back(midiDisp);
	}

	init(&children, nullptr);
}

TrackControl::~TrackControl()
{
	AU::safeDelete(audioDisp);
	AU::safeDelete(midiDisp);
}

void TrackControl::onSizeChanged(AVec d_size)
{
	//Get correct padding to use (so its consistent in parents absolute space)
	AVec	t_padding = tmd_parent->correctVirtualVec(PADDING);

	float h = (size.y - 3.0f*t_padding.y)*(1.0f/2.0f);
	AVec b_disp_size = AVec(size.x - t_padding.x*2.0f, h);

	if(audioDisp)
	{
		audioDisp->setPos(APoint(t_padding));
		audioDisp->setSize(b_disp_size);
	}

	if(midiDisp)
	{
		midiDisp->setPos(APoint(t_padding.x, t_padding.y*2.0f + h));
		midiDisp->setSize(b_disp_size);
	}
	
	//std::cout << b_disp_size << ", " << size << "\n";

}

AudioTrackNode* TrackControl::getAudioNode()
{
	return audioNode;
}

MidiTrackNode* TrackControl::getMidiNode()
{
	return midiNode;
}


void TrackControl::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}
