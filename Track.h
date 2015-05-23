#ifndef APOLLO_TRACK_H
#define APOLLO_TRACK_H

#include <inttypes.h>
#include <vector>

#include "Timing.h"
#include "Control.h"
#include "Samplers.h"
#include "Audio.h"
#include "Modifier.h"
#include "MIDI.h"

//#define TRACK_CHUNK_SIZE 512

struct TrackPoint 
{
	s_time time;
	double value;
	InterpolationType interp;	//Interpolation for after this point
	std::vector<float> *data;

	TrackPoint(s_time t, double val, InterpolationType interp_, std::vector<float> *data_ = nullptr)
		: time(t), value(val), interp(interp_), data(data_)
	{ }
};


//Look up:
// --> Pure data
//    --> Max MSP

class Track
{
protected:
	Time *cursor;
	Track *parent;
	
	TimeRange range;
	bool repeat;

public:
	Track();
	Track(Time *p_cursor, bool repeat_);

	void setCursor(Time *p_cursor);
	Time* getCursor();
	void setRepeat(bool repeat_);
	bool isRepeating();

	void setRange(TimeRange new_range);
	void setStart(s_time new_start);
	void setEnd(s_time new_end);

	virtual s_time getLength() = 0;
};

class ModTrack : public Track, public Modifier
{
protected:
	std::vector<TrackPoint> points;

public:
	ModTrack();
	ModTrack(Time *p_cursor, bool repeat_ = false);

	void resetPoints();
	void addPoint(TrackPoint p);

	std::vector<TrackPoint> getPoints() { return points; }

	void modify(SampleInfo &info, s_time t);

	s_time getLength();
};

class BaseTrack : public Track
{
protected:
	std::vector<Modifier*> modifiers;

public:
	BaseTrack();
	BaseTrack(Time *p_cursor, bool repeat_);

	virtual s_time getLength() = 0;

	virtual void getChunk(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate) = 0;
	virtual void getChunkAdd(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate) = 0;
};

class MidiTrack : public BaseTrack
{
protected:
	std::vector<MidiNote> notes;
	Waveform *waveform;

public:
	MidiTrack();
	MidiTrack(Time *p_cursor, Waveform *wave, bool repeat_ = false);

	void setWaveform(Waveform *wave);

	s_time getLength();
	std::vector<MidiNote> getNotes() { return notes; }

	void getChunk(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate);
	void getChunkAdd(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate);
};


class AudioTrack : public BaseTrack
{
protected:
	std::vector<BaseTrack*> childTracks;
	std::vector<AudioSample> audioData;
	std::vector<TimeRange> needUpdate;
	int sampleRate;
	int dataPadding;

	void updateRange(TimeRange range);

public:
	AudioTrack(int sample_rate);
	AudioTrack(Time *p_cursor, int sample_rate, bool repeat_ = false);
	~AudioTrack();
	void rangeTest();

	void addChild(BaseTrack *childTrack);

	void assertUpdated(TimeRange range);

	s_time getLength();

	void setSampleRate(int sample_rate);

	void appendData(std::vector<AudioSample> &new_data);
	void getChunk(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate = -1);
	void getChunkAdd(AudioSample *data, s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate = -1);

	AudioSample* getChunk(s_time start, int num_samples, const std::vector<Modifier*> &mods, int sample_rate = -1);
};

/*
class TrackDisplay : public Control
{
private:
	Track *audioTrack;
	ScrollBar hScroll;

	float zoomMult;	//Ratio of samples to number of pixels

public:
	TrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, Track *track, bool visible_ = true);
	TrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, Track *track, bool visible_ = true);

	void onPositionChanged();
	void onSizeChanged();

	void setZoom(float new_zoom);
	
	bool checkMouse(Point2i m_pos, Vec2i d_pos, bool disregard);
	void mouseDown(Point2i m_pos, MouseButton b);
	void mouseUp(Point2i m_pos, MouseButton b);
	bool mouseScroll(Point2i m_pos, Vec2f d_scroll, bool disregard);
	void scroll(Vec2f d_scroll);

	void draw(float dt);
};
*/

class AudioTrackDisplay : public TimeAlignedDisplay
{
private:
	AudioTrack *track;

public:
	AudioTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, AudioTrack *track_, bool visible_ = true);
	AudioTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, AudioTrack *track_, bool visible_ = true);
	
	void onSizeChanged();

	void draw(float dt);
};

class MidiTrackDisplay : public TimeAlignedDisplay
{
private:
	MidiTrack *track;
public:
	MidiTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, MidiTrack *track_, bool visible_ = true);
	MidiTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, MidiTrack *track_, bool visible_ = true);

	void onSizeChanged();

	void draw(float dt);
};

class ModTrackDisplay : public TimeAlignedDisplay
{
private:
	ModTrack *track;
public:
	ModTrackDisplay(Container *parent_, Point2i absolute_pos, Vec2i absolute_size, ModTrack *track_, bool visible_ = true);
	ModTrackDisplay(Container *parent_, Point2f relative_pos, Vec2f relative_size, ModTrack *track_, bool visible_ = true);
	
	void onSizeChanged();

	void draw(float dt);
};

#endif	//APOLLO_AudioTrack_H