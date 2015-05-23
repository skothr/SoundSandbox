#include "ProjectTrackDisplay.h"

#include "TrackNodes.h"
#include "AudioTrackControl.h"
#include "MidiTrackControl.h"
//#include "CollapseList.h"
#include "Cursor.h"
#include "Keyboard.h"
#include "ScrollBar.h"

/////PROJECT TRACK DISPLAY/////
const GuiPropFlags	ProjectTrackDisplay::PROP_FLAGS				= PFlags::HARD_BACK;
const float			ProjectTrackDisplay::TRACK_HEIGHT			= 100.0f,
					ProjectTrackDisplay::X_OFFSET_STEP			= 100.0f,
					ProjectTrackDisplay::SAMPLES_PER_PIXEL		= 50.0f,
					ProjectTrackDisplay::CURSOR_FOLLOW_OFFSET	= 200.0f;

ProjectTrackDisplay::ProjectTrackDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Cursor *track_cursor)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		ScrollArea(parent_, a_pos, a_size, s_flags),
		cursor(track_cursor), trackRange(X_OFFSET_STEP, maxViewOffset.x + a_size.x - X_OFFSET_STEP),
		sampleToAbsolute(1.0f/SAMPLES_PER_PIXEL), absoluteToSample(SAMPLES_PER_PIXEL),
		cursor_pos((track_cursor ? track_cursor->getSampleRange().start/SAMPLES_PER_PIXEL : 0.0f))
{
	trackList = new CollapseList(this, APoint(0.0f, 0.0f), size, DEFAULT_STATE, { });

	trackList->attachTo(this, Side::LEFT, 5.0f);
	trackList->attachTo(this, Side::TOP, 5.0f);

	trackList->updateResources();

	trackList->setOffset(X_OFFSET_STEP - 5.0f - AudioTrackControl::PADDING.x);

	setAllBgStateColors(Color(0.0f, 0.0f, 0.0f, 1.0f));
}

ProjectTrackDisplay::~ProjectTrackDisplay()
{
	//clearChildren();
	if(trackList)
		delete trackList;
	trackList = nullptr;
}

void ProjectTrackDisplay::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	APoint v_m_pos = transform.absoluteToVirtualPoint(m_pos);
	if(v_m_pos.x < X_OFFSET_STEP && valid(b & MouseButton::LEFT))
	{
		cursor->setTimeSamples(0);
	}
}

void ProjectTrackDisplay::onSizeChanged(AVec d_size)
{
	//ScrollArea::onSizeChanged(d_size);

	AVec v_size = transform.absoluteToVirtualVec(size);
	trackRange.end = maxViewOffset.x + v_size.x - X_OFFSET_STEP;
}

void ProjectTrackDisplay::onScroll(APoint m_pos, AVec d_scroll, bool direct)
{
	ScrollArea::onScroll(m_pos, d_scroll, direct);

	if(Keyboard::keyDown(Keys::K_LALT) || Keyboard::keyDown(Keys::K_LALT))
	{
		//sampleToAbsolute = getZoom().x*SAMPLES_PER_PIXEL;
		sampleToAbsolute *= 1.0f - 0.01f*(d_scroll.x + d_scroll.y);
		absoluteToSample = 1.0f/sampleToAbsolute;

		for(auto tc : audioTrackControls)
			tc->setSamplesPerPixel(absoluteToSample);
		
		for(auto tc : midiTrackControls)
			tc->setSecondsPerPixel(absoluteToSample/cursor->getSampleRate());
	}
}

void ProjectTrackDisplay::addTrack(AudioTrackNode *atn)
{
	AudioTrackControl	*atc = new AudioTrackControl(trackList, APoint(0.0f, 0.0f), TRACK_HEIGHT, DEFAULT_STATE_FLOAT, atn, absoluteToSample);
	atc->setCursor(cursor);
	trackNodes.push_back(atn);
	audioTrackControls.push_back(atc);

	ElementTree track_tree(atc, true);

	std::vector<ChildNodeTree> child_tracks = atn->getChildren();

	for(auto n : child_tracks)
	{
		if(n.node->getType() == NodeType::AUDIO_TRACK)
		{
			AudioTrackControl *audio = new AudioTrackControl(trackList, APoint(0.0f, 0.0f), TRACK_HEIGHT, DEFAULT_STATE_FLOAT, dynamic_cast<AudioTrackNode*>(n.node), absoluteToSample);
			
			audio->setCursor(cursor);

			ElementTree child_tree(audio, true);
			audioTrackControls.push_back(audio);

			track_tree.addChild(child_tree);
		}
		else if(n.node->getType() == NodeType::MIDI_TRACK)
		{
			MidiTrackControl *midi = new MidiTrackControl(trackList, APoint(0.0f, 0.0f), TRACK_HEIGHT, DEFAULT_STATE_FLOAT, dynamic_cast<MidiTrackNode*>(n.node), absoluteToSample/cursor->getSampleRate());
			
			midi->setCursor(cursor);

			ElementTree child_tree(midi, true);
			midiTrackControls.push_back(midi);

			track_tree.addChild(child_tree);
		}
	}

	trackTrees.push_back(track_tree);
	trackList->addTree(track_tree);

	updateScrollBars();
	updateResources();
}

void ProjectTrackDisplay::removeTrack(AudioTrackNode *atn)
{
	auto iter = std::find(trackNodes.begin(), trackNodes.end(), atn);
	if(iter != trackNodes.end())
	{
		unsigned int index = iter - trackNodes.begin();
		
		trackTrees.erase(trackTrees.begin() + index);
		trackList->removeTree(index);

		if(audioTrackControls[index])
			delete audioTrackControls[index];
		audioTrackControls.erase(audioTrackControls.begin() + index);

		trackNodes.erase(iter);
	}

	updateScrollBars();
	updateResources();
}

bool ProjectTrackDisplay::isPlaying() const
{
	return cursor && cursor->active;
}

void ProjectTrackDisplay::play()
{
	if(cursor)
		cursor->active = true;
}

void ProjectTrackDisplay::stop()
{
	if(cursor)
		cursor->active = false;
}

void ProjectTrackDisplay::setCursor(Cursor *new_cursor)
{
	cursor = new_cursor;
	cursor_pos = (cursor ? cursor->getSampleRange().start*sampleToAbsolute : 0.0f);
	
	for(auto tc : audioTrackControls)
		tc->setCursor(cursor);
		
	for(auto tc : midiTrackControls)
		tc->setCursor(cursor);
}

Cursor* ProjectTrackDisplay::getCursor()
{
	return cursor;
}

//void ProjectTrackDisplay::update(double dt)
//{
//}

void ProjectTrackDisplay::updateCursorPos()
{
	if(cursor)
	{
		float cfo = transform.absoluteToVirtualVec(AVec(CURSOR_FOLLOW_OFFSET, 0.0f)).x;

		float	v_cursor_pos = trackRange.start + cursor->getChunkRange().start*cursor->getChunkSize()*sampleToAbsolute;
		AVec	v_size = transform.absoluteToVirtualVec(size);
		APoint	view_offset = getViewOffset();

		if(isPlaying())
		{
			updateScrollBars();

			float new_view_offset = horizontal->getOffset();

			if((v_cursor_pos > cursor_pos) && (v_cursor_pos > view_offset.x + v_size.x - cfo))
			{
				new_view_offset += v_cursor_pos - (view_offset.x + v_size.x - cfo);
			}
			else if((v_cursor_pos < cursor_pos) && (v_cursor_pos < view_offset.x + cfo))
			{
				new_view_offset += v_cursor_pos - (view_offset.x + cfo);
			}

			horizontal->setOffset(new_view_offset);
			//setViewOffset(APoint(new_view_offset, view_offset.y));
			
			//updateScrollBars();
		}
		
		cursor_pos = v_cursor_pos;

		if(cursor_pos >= trackRange.end)
		{
			stop();
			cursor_pos = trackRange.end;
		}
	}
}

void ProjectTrackDisplay::draw(GlInterface &gl)
{
	updateCursorPos();

	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		if(cursor)
		{
			float cursor_x = trackRange.start + cursor->getChunkRange().start*cursor->getChunkSize()*sampleToAbsolute;

			APoint a_cursor_pos = transform.virtualToAbsolutePoint(APoint(cursor_x, 0.0f));//cursor_pos, 0.0f));
		
			gl.setColor(Color(0.8f, 0.8f, 0.2f, 1.0f));
			gl.drawLine(APoint(a_cursor_pos.x, 0.0f), APoint(a_cursor_pos.x, size.y));
		}

		restoreViewport(gl);
	}
}