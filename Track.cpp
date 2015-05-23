#include "Track.h"
#include "Container.h"
#include "Waveform.h"

#include <iostream>


///// TRACK

Track::Track()
	: cursor(nullptr), repeat(false)
{ }

Track::Track(Time *p_cursor, bool repeat_)
	: cursor(p_cursor), repeat(repeat_)
{ }

void Track::setCursor(Time *p_cursor)
{
	cursor = p_cursor;
}

Time* Track::getCursor()
{
	return cursor;
}

void Track::setRepeat(bool repeat_)
{
	repeat = repeat_;
}

bool Track::isRepeating()
{
	return repeat;
}

void Track::setRange(TimeRange new_range)
{
	range = new_range;
}

void Track::setStart(s_time new_start)
{
	range.start = new_start;
}

void Track::setEnd(s_time new_end)
{
	range.end = new_end;
}


///// MOD TRACK
ModTrack::ModTrack()
	: Track()
{
	resetPoints();
}

ModTrack::ModTrack(Time *p_cursor, bool repeat_)
	: Track(p_cursor, repeat_)
{
	resetPoints();
}

void ModTrack::modify(SampleInfo &info, s_time t)
{
	info.frequency += 0.01*sin(t*0.001);
	//info.frequency += 20*sin(t*0.000002);
	//info.amplitude = 16000;

	//if(info.frequency < 0)
	//	info.frequency = 0;

	//Cool Sounds!
	////1:
	//double x = 16000*sin(t*t*0.00002);
	//if(x < 0) x = 0;
	//info.amplitude = x;
	////2:
	//double x = 190;
	//if(x < 0) x = 0;
	//info.frequency = t%3000;
	////3:
	//info.frequency += 0.01*sin(t*0.001);
}

void ModTrack::resetPoints()
{
	points.clear();
	points.push_back(TrackPoint(0, 0, INTERP_LINEAR));
	points.push_back(TrackPoint(1000000, 0, INTERP_LINEAR));
}

void ModTrack::addPoint(TrackPoint p)
{
	points.push_back(p);
}

s_time ModTrack::getLength()
{
	return points[points.size() - 1].time;
}

///// BASE TRACK
BaseTrack::BaseTrack()
	: Track()
{ }

BaseTrack::BaseTrack(Time *p_cursor, bool repeat_)
	: Track(p_cursor, repeat_)
{ }

///// MIDI TRACK
MidiTrack::MidiTrack()
	: BaseTrack()
{ }

MidiTrack::MidiTrack(Time *p_cursor, Waveform *wave, bool repeat_)
	: BaseTrack(p_cursor, repeat_), waveform(wave)
{
	bool up = true;
	for(int i = 0; i < NUM_MIDI_NOTES; i++)
	{
		/*
		int ind = i % 8;
		if(!up) ind = 7 - ind;
		static const int base_note = 69;

		switch(ind)
		{
		case 0:
			ind = 0; up = true; break;
		case 1:
			ind = 2; break;
		case 2:
			ind = 4; break;
		case 3:
			ind = 5; break;
		case 4:
			ind = 7; break;
		case 5:
			ind = 9; break;
		case 6:
			ind = 11; break;
		case 7:
			ind = 12; up = false; break;
		}

		ind += base_note;
		*/
		notes.push_back(MidiNote(i, 50, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(floor(25.0 + 25.0*sin((float)i*0.5)), 100, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 12, 20, i*5000 + 2500, (i + 1)*5000 + 2500));
		//notes.push_back(MidiNote(69 + i + 4, 20, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 6, 20, i*5000, (i + 1)*5000));
		//notes.push_back(MidiNote(69 + i + 8, 20, i*5000, (i + 1)*5000));
	}
	
	notes.push_back(MidiNote(45, 100, 0, 1000000));
	//notes.push_back(MidiNote(75, 20, 0, 1000000));
	//notes.push_back(MidiNote(85, 20, 0, 1000000));
}

void MidiTrack::setWaveform(Waveform *wave)
{
	waveform = wave;
}

s_time MidiTrack::getLength()
{
	return notes[notes.size() - 1].end;
}

void MidiTrack::getChunk(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate)
{
	SampleInfo info;
	info.sample_rate = sample_rate;

	s_time cursor = start;
	s_time last_cursor = -1;
	int data_offset = 0;

	//Clear data
	for(s_time t = 0; t < num_samples; t++)
		data[t] = 0;

	s_time end = start + num_samples;

	//Add track modifiers to these mods
	std::vector<Modifier*> all_mods(mods.begin(), mods.end());
	all_mods.insert(all_mods.end(), modifiers.begin(), modifiers.end());

	for(unsigned int i = 0; i < notes.size(); i++)
	{
		if(notes[i].end >= start && notes[i].start <= end)
		{
			//TODO: Calculate phase using distance from note.start and start
			info.phase = 0;

			info.frequency = getFrequency(notes[i].index);
			info.amplitude = 160.0*notes[i].velocity;

			s_time chunk_start = notes[i].start > start ? notes[i].start : start,
				chunk_end = notes[i].end < end ? notes[i].end : end;

			int samples = chunk_end - chunk_start;

			waveform->sampleChunkAdd(chunk_start, info, samples, data + notes[i].start - start, all_mods);
		}
	}
}

void MidiTrack::getChunkAdd(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate)
{
	SampleInfo info;
	info.sample_rate = sample_rate;

	s_time cursor = start;
	s_time last_cursor = -1;
	int data_offset = 0;

	s_time end = start + num_samples;

	//Add track modifiers to these mods
	std::vector<Modifier*> all_mods(mods.begin(), mods.end());
	all_mods.insert(all_mods.end(), modifiers.begin(), modifiers.end());

	for(unsigned int i = 0; i < notes.size(); i++)
	{
		if(notes[i].end >= start && notes[i].start <= end)
		{
			//TODO: Calculate phase using distance from note.start and start
			info.phase = 0;

			info.frequency = getFrequency(notes[i].index);
			info.amplitude = 160.0*notes[i].velocity;

			s_time chunk_start = notes[i].start > start ? notes[i].start : start,
				chunk_end = notes[i].end < end ? notes[i].end : end;

			int samples = chunk_end - chunk_start;

			waveform->sampleChunkAdd(chunk_start, info, samples, data + notes[i].start - start, all_mods);
		}
	}
}

///// AUDIO TRACK
ModTrack m;

AudioTrack::AudioTrack(int sample_rate)
	: BaseTrack(), sampleRate(sample_rate), dataPadding(0)
{ }

AudioTrack::AudioTrack(Time *p_cursor, int sample_rate, bool repeat_)
	: BaseTrack(p_cursor, repeat_), sampleRate(sample_rate), dataPadding(0)
{ }

AudioTrack::~AudioTrack()
{ }

void AudioTrack::rangeTest()
{
	modifiers.push_back(&m);
	needUpdate.push_back(TimeRange(0, 1000000));
	assertUpdated(TimeRange(0, 1000000));
}


void AudioTrack::addChild(BaseTrack *childTrack)
{
	childTracks.push_back(childTrack);
}

void AudioTrack::updateRange(TimeRange range)
{
	int numSamples = range.end - range.start;

	//Make sure vector has enough space
	if(audioData.size() < range.end)
		audioData.resize(range.end);

	//Start with first track's audio (or set everything to 0)
	if(childTracks.size() > 0)
	{
		childTracks[0]->getChunk(audioData.data() + range.start, range.start, range.end - range.start, modifiers, sampleRate);
	}
	else
	{
		for(s_time t = range.start; t < range.end; t++)
			audioData[t] = 0;
	}

	//Combine with rest of child tracks
	for(unsigned int i = 1; i < childTracks.size(); i++)
	{
		childTracks[i]->getChunkAdd(audioData.data() + range.start, range.start, range.end - range.start, modifiers, sampleRate);
	}
}

void AudioTrack::assertUpdated(TimeRange range)
{
	std::vector<TimeRange> new_ranges;
	std::vector<int> updates_to_remove;

	for(unsigned int i = 0; i < needUpdate.size(); i++)
	{
		//If there is overlap
		if(range.end >= needUpdate[i].start && range.start <= needUpdate[i].end)
		{
			//Remove this range from needUpdate
			updates_to_remove.push_back(i);

			TimeRange update_range(-1, -1);

			//Determine low bounds to update
			if(range.start <= needUpdate[i].start)
				update_range.start = needUpdate[i].start;
			else
			{
				update_range.start = range.start;
				new_ranges.push_back(TimeRange(needUpdate[i].start, range.start));
			}

			//Determine high bounds to update
			if(range.end >= needUpdate[i].end)
				update_range.end = needUpdate[i].end;
			else
			{
				update_range.end = range.end;
				new_ranges.push_back(TimeRange(range.end, needUpdate[i].end));
			}
			
			updateRange(update_range);
		}
	}

	//Remove ranges from needUpdate
	for(unsigned int i = 0; i < updates_to_remove.size(); i++)
		needUpdate.erase(needUpdate.begin() + updates_to_remove[i]);

	//Add new update ranges
	needUpdate.reserve(needUpdate.size() + new_ranges.size());
	needUpdate.insert(needUpdate.end(), new_ranges.begin(), new_ranges.end());

}

s_time AudioTrack::getLength()
{
	return static_cast<s_time>(audioData.size() - dataPadding);
}

void AudioTrack::setSampleRate(int sample_rate)
{
	sampleRate = sample_rate;
}

void AudioTrack::appendData(std::vector<AudioSample> &new_data)
{
	audioData.resize(audioData.size() - dataPadding);
	audioData.reserve(audioData.size() + new_data.size());
	audioData.insert(audioData.end(), new_data.begin(), new_data.end());
}

void AudioTrack::getChunk(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate)
{
	//TODO: apply mods
	//TODO: interpolate for (sample_rate != -1)

	for(s_time t = start; t < start + num_samples; t++)
	{
		data[t - start] = audioData[t];
	}
}

void AudioTrack::getChunkAdd(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate)
{
	//TODO: apply mods
	//TODO: interpolate for (sample_rate != -1)

	for(s_time t = start; t < start + num_samples; t++)
	{
		data[t - start] += audioData[t];
	}
}

AudioSample* AudioTrack::getChunk(s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate)
{
	//Reserve necessary space in vector
	if(start >= audioData.size() - dataPadding)
	{
		return nullptr;
	}
	
	if(start + num_samples > audioData.size())
	{
		int padding = start + num_samples - audioData.size();
		audioData.resize(audioData.size() + padding, 0);
		dataPadding += padding;
	}
	
	assertUpdated(TimeRange(start, start + num_samples));
	
	return audioData.data() + start;
}


//////TRACK DISPLAY DEFINITIONS
/*
TrackDisplay::TrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, Track *AudioTrack, bool visible_)
	: Control(parent_, absolute_pos, absolute_size, visible_), hScroll(parent, absolute_pos, 0, absolute_size.x, true, visible_), audioTrack(AudioTrack), zoomMult(10.0f)
{
	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(audioTrack->getLength()/zoomMult - aSize.x);
}

TrackDisplay::TrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, Track *AudioTrack, bool visible_)
	: Control(parent_, relative_pos, relative_size, visible_), hScroll(parent, Point2i(), 0, 0, true, visible_), audioTrack(AudioTrack), zoomMult(1.0f)
{
	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setPosition(aPos + aSize - Vec2i(0, ScrollBar::DEFAULT_WIDTH));
	hScroll.setSize(Vec2i(aSize.x, hScroll.getSize().y));

	hScroll.setMaxOffset(audioTrack->getLength()/zoomMult - aSize.x);
}

void TrackDisplay::onPositionChanged()
{
	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setPosition(aPos + Vec2i(0, aSize.y - hScroll.getSize().y));
}

void TrackDisplay::onSizeChanged()
{
	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setSize(Vec2i(aSize.x, hScroll.getSize().y));
	hScroll.setMaxOffset(audioTrack->getLength()/zoomMult - aSize.x);
}

void TrackDisplay::setZoom(float new_zoom)
{
	zoomMult = new_zoom;
}


bool TrackDisplay::checkMouse(Point2i m_pos, Vec2i d_pos, bool disregard)
{
	Point2i aPos = pos.absoluteCoords(parent->getSize());
	//Vec2i newPosVec = m_pos - aPos;
	//Point2i newPos = Point2i(newPosVec.x, newPosVec.y);

	disregard = hScroll.checkMouse(m_pos, d_pos, disregard);
	m_pos.x += hScroll.getOffset();

	Control::checkMouse(m_pos, d_pos, disregard);

	return pointInside(m_pos);
}

void TrackDisplay::mouseDown(Point2i m_pos, MouseButton b)
{
	Point2i aPos = pos.absoluteCoords(parent->getSize());
	//Vec2i newPosVec = m_pos - aPos;
	//Point2i newPos = Point2i(newPosVec.x, newPosVec.y);

	hScroll.mouseDown(m_pos, b);
	m_pos.x += hScroll.getOffset();

	Control::mouseDown(m_pos, b);

	return;
}

void TrackDisplay::mouseUp(Point2i m_pos, MouseButton b)
{
	Point2i aPos = pos.absoluteCoords(parent->getSize());
	//Vec2i newPosVec = m_pos - aPos;
	//Point2i newPos = Point2i(newPosVec.x, newPosVec.y);

	hScroll.mouseUp(m_pos, b);
	
	m_pos.x += hScroll.getOffset();
	Control::mouseUp(m_pos, b);

	return;
}

bool TrackDisplay::mouseScroll(Point2i m_pos, Vec2f d_scroll, bool disregard)
{
	if(pointInside(m_pos) && !disregard)
	{
		scroll(d_scroll);
		disregard = true;
	}

	//disregard = Control::mouseScroll(m_pos, d_scroll, disregard);

	return disregard;
}

void TrackDisplay::scroll(Vec2f d_scroll)
{
	hScroll.setOffset(hScroll.getOffset() + d_scroll.y*ScrollArea::SCROLL_MULT);
}

void TrackDisplay::draw(float dt)
{
	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	Point2f glPos = pos.glCoords(parent_size);
	Vec2f glSize = toGlVec(aSize - Vec2i(0, hScroll.getSize().y), parent_size);

	
	hScroll.setMaxOffset(audioTrack->getLength()/zoomMult - aSize.x);

	//Draw background
	glBegin(GL_TRIANGLE_STRIP);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex2f(glPos.x, glPos.y);
		glVertex2f(glPos.x + glSize.x, glPos.y);
		glVertex2f(glPos.x, glPos.y + glSize.y);
		glVertex2f(glPos.x + glSize.x, glPos.y + glSize.y);
	glEnd();
	
	int num_samples = aSize.x*zoomMult;
	s_time start_time = hScroll.getOffset()*zoomMult;
	AudioSample *data = audioTrack->getChunk(start_time, num_samples + 1);

	glColor3f(1.0f, 0.0f, 1.0f);
	if(data)
	{
		float x_mult = 1.0f/num_samples;

		for(unsigned int i = 0; i < num_samples - 1; i++)
		{
			float x0 = i*x_mult,
				  x1 = (i + 1)*x_mult;

			glBegin(GL_LINES);
				glVertex2f(glPos.x + glSize.x*x0, glPos.y + (glSize.y/2.0f)*(1.0f - data[i]/16000.0f));		//TODO: global max amplitude
				glVertex2f(glPos.x + glSize.x*x1, glPos.y + (glSize.y/2.0f)*(1.0f - data[i + 1]/16000.0f));
			glEnd();
		}
	}
	else
	{
		//Show flat line
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex2f(glPos.x, glPos.y + glSize.y/2.0f);
			glVertex2f(glPos.x + glSize.x, glPos.y + glSize.y/2.0f);
		glEnd();
	}

	s_time cursor_pos = audioTrack->getCursor()->getTime();

	if(cursor_pos >= start_time && cursor_pos <= start_time + num_samples)
	{
		cursor_pos -= start_time;
		glPos = toGlPoint(aPos + Vec2i(cursor_pos/zoomMult, 0), parent_size);
		//Draw vertical line where cursor is
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex2f(glPos.x, glPos.y);
			glVertex2f(glPos.x, glPos.y + glSize.y);
		glEnd();
	}

	//Reset color
	glColor3f(1.0f, 1.0f, 1.0f);

	//Draw scrollbar
	hScroll.draw(dt);
}
*/

/////AUDIO TRACK DISPLAY

AudioTrackDisplay::AudioTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, AudioTrack *AudioTrack, bool visible_)
	: TimeAlignedDisplay(parent_, absolute_pos, absolute_size, visible_), track(AudioTrack)
{
	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

AudioTrackDisplay::AudioTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, AudioTrack *AudioTrack, bool visible_)
	: TimeAlignedDisplay(parent_, relative_pos, relative_size, visible_), track(AudioTrack)
{
	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

void AudioTrackDisplay::onSizeChanged()
{
	TimeAlignedDisplay::onSizeChanged();
	Vec2i aSize = size.absoluteCoords(parent->getSize());
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}


void AudioTrackDisplay::draw(float dt)
{
	TimeAlignedDisplay::draw(dt);

	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	Point2f glPos = pos.glCoords(parent_size);
	Vec2f glSize = toGlVec(aSize - Vec2i(0, hScroll.getSize().y), parent_size);

	//TODO: get rid of this somehow
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);

	int num_samples = aSize.x*zoomMult;
	s_time start_time = hScroll.getOffset()*zoomMult;
	AudioSample *data = track->getChunk(start_time, num_samples + 1, std::vector<Modifier*>());//getChunk(start_time, num_samples + 1);

	glColor3f(1.0f, 0.0f, 1.0f);
	if(data)
	{
		float x_mult = 1.0f/num_samples;

		for(unsigned int i = 0; i < num_samples - 1; i++)
		{
			float x0 = i*x_mult,
				  x1 = (i + 1)*x_mult;

			glBegin(GL_LINES);
				glVertex2f(glPos.x + glSize.x*x0, glPos.y + (glSize.y/2.0f)*(1.0f - data[i]/16000.0f));		//TODO: global max amplitude
				glVertex2f(glPos.x + glSize.x*x1, glPos.y + (glSize.y/2.0f)*(1.0f - data[i + 1]/16000.0f));
			glEnd();
		}
	}
	else
	{
		//Show flat line
		glBegin(GL_LINES);
			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex2f(glPos.x, glPos.y + glSize.y/2.0f);
			glVertex2f(glPos.x + glSize.x, glPos.y + glSize.y/2.0f);
		glEnd();
	}
	
	if(track->getCursor())
	{
		s_time cursor_pos = track->getCursor()->getTime();

		if(cursor_pos >= start_time && cursor_pos <= start_time + num_samples)
		{
			cursor_pos -= start_time;
			glPos = toGlPoint(aPos + Vec2i(cursor_pos/zoomMult, 0), parent_size);
			//Draw vertical line where cursor is
			glBegin(GL_LINES);
				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex2f(glPos.x, glPos.y);
				glVertex2f(glPos.x, glPos.y + glSize.y);
			glEnd();
		}
	}

	//Reset color
	glColor3f(1.0f, 1.0f, 1.0f);

	//Draw scrollbar
	hScroll.draw(dt);
}




/////MIDI TRACK DISPLAY

MidiTrackDisplay::MidiTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, MidiTrack *track_, bool visible_)
	: TimeAlignedDisplay(parent_, absolute_pos, absolute_size, visible_), track(track_)
{
	zoomMult = 50.0f;

	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

MidiTrackDisplay::MidiTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, MidiTrack *track_, bool visible_)
	: TimeAlignedDisplay(parent_, relative_pos, relative_size, visible_), track(track_)
{
	zoomMult = 50.0f;

	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

void MidiTrackDisplay::onSizeChanged()
{
	TimeAlignedDisplay::onSizeChanged();
	Vec2i aSize = size.absoluteCoords(parent->getSize());
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

//Helper function to convert hue, saturation, value --> rgb.
// All values between 0.0f and 1.0f
void hsv2rgb(float h, float s, float v, float &r, float &g, float &b)
{
	int i = floor(h*6);
	float f = h*6 - i;
	float p = v*(1.0f - s);
	float q = v*(1.0f - f*s);
	float t = v*(1.0f - (1.0f - f)*s);

	switch(i % 6)
	{
	case 0:
		r = v; g = t; b = p; break;
	case 1:
		r = q; g = v; b = p; break;
	case 2:
		r = p; g = v; b = t; break;
	case 3:
		r = p; g = q; b = v; break;
	case 4:
		r = t; g = p; b = v; break;
	case 5:
		r = v; g = p; b = q; break;
	}
}

void MidiTrackDisplay::draw(float dt)
{
	TimeAlignedDisplay::draw(dt);

	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	Point2f glPos = pos.glCoords(parent_size);
	Vec2f glSize = toGlVec(aSize - Vec2i(0, hScroll.getSize().y), parent_size);

	//TODO: get rid of this somehow
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);

	int num_samples = aSize.x*zoomMult;
	s_time start_time = hScroll.getOffset()*zoomMult;
	std::vector<MidiNote> notes = track->getNotes();

	for(unsigned int i = 0; i < notes.size(); i++)
	{
		if(notes[i].end >= start_time && notes[i].start <= start_time + num_samples)
		{
			//Hue/saturation/value
			float	h = (5.0f/6.0f)*notes[i].index/NUM_MIDI_NOTES,
					s = notes[i].velocity/127.0f,
					v = s*0.25f + 0.75f,
					r, g, b;
			hsv2rgb(h, s, v, r, g, b);
			glColor3f(r, g, b);
			glBegin(GL_LINES);
				glVertex2f(glPos.x + glSize.x*(notes[i].start - start_time)/num_samples, glPos.y + glSize.y*(1.0f - notes[i].index/(float)NUM_MIDI_NOTES));		//TODO: global max amplitude
				glVertex2f(glPos.x + glSize.x*(notes[i].end - start_time)/num_samples, glPos.y + glSize.y*(1.0f - notes[i].index/(float)NUM_MIDI_NOTES));
			glEnd();
		}
	}

	if(track->getCursor())
	{
		s_time cursor_pos = track->getCursor()->getTime();

		if(cursor_pos >= start_time && cursor_pos <= start_time + num_samples)
		{
			cursor_pos -= start_time;
			glPos = toGlPoint(aPos + Vec2i(cursor_pos/zoomMult, 0), parent_size);
			//Draw vertical line where cursor is
			glBegin(GL_LINES);
				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex2f(glPos.x, glPos.y);
				glVertex2f(glPos.x, glPos.y + glSize.y);
			glEnd();
		}
	}

	//Reset color
	glColor3f(1.0f, 1.0f, 1.0f);

	//Draw scrollbar
	hScroll.draw(dt);
}



/////MOD TRACK DISPLAY

ModTrackDisplay::ModTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, ModTrack *track_, bool visible_)
	: TimeAlignedDisplay(parent_, absolute_pos, absolute_size, visible_), track(track_)
{
	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

ModTrackDisplay::ModTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, ModTrack *track_, bool visible_)
	: TimeAlignedDisplay(parent_, relative_pos, relative_size, visible_), track(track_)
{
	Vec2i parent_size = parent->getSize();
	Vec2i aSize = size.absoluteCoords(parent_size);

	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}

void ModTrackDisplay::onSizeChanged()
{
	TimeAlignedDisplay::onSizeChanged();
	Vec2i aSize = size.absoluteCoords(parent->getSize());
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);
}


void ModTrackDisplay::draw(float dt)
{
	TimeAlignedDisplay::draw(dt);

	Vec2i parent_size = parent->getSize();
	Point2i aPos = pos.absoluteCoords(parent_size);
	Vec2i aSize = size.absoluteCoords(parent_size);

	Point2f glPos = pos.glCoords(parent_size);
	Vec2f glSize = toGlVec(aSize - Vec2i(0, hScroll.getSize().y), parent_size);

	//TODO: get rid of this somehow
	hScroll.setMaxOffset(track->getLength()/zoomMult - aSize.x);

	int num_samples = aSize.x*zoomMult;
	s_time start_time = hScroll.getOffset()*zoomMult;

	std::vector<TrackPoint> points = track->getPoints();

	glColor3f(1.0f, 0.0f, 1.0f);
	float x_mult = 1.0f/num_samples;

	for(unsigned int i = 0; i < points.size() - 1; i++)
	{
		glBegin(GL_LINES);
			glVertex2f(glPos.x + glSize.x*points[i].time, glPos.y + glSize.y*(1.0f - points[i].value));		//TODO: global max amplitude
			glVertex2f(glPos.x + glSize.x*points[i + 1].time, glPos.y + glSize.y*(1.0f - points[i + 1].value));
		glEnd();
		glBegin(GL_POINTS);
			glVertex2f(glPos.x + glSize.x*points[i].time, glPos.y + glSize.y*(1.0f - points[i].value));
		glEnd();
	}
	
	if(track->getCursor())
	{
		s_time cursor_pos = track->getCursor()->getTime();

		if(cursor_pos >= start_time && cursor_pos <= start_time + num_samples)
		{
			cursor_pos -= start_time;
			glPos = toGlPoint(aPos + Vec2i(cursor_pos/zoomMult, 0), parent_size);
			//Draw vertical line where cursor is
			glBegin(GL_LINES);
				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex2f(glPos.x, glPos.y);
				glVertex2f(glPos.x, glPos.y + glSize.y);
			glEnd();
		}
	}

	//Reset color
	glColor3f(1.0f, 1.0f, 1.0f);

	//Draw scrollbar
	hScroll.draw(dt);
}