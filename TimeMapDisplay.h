#ifndef APOLLO_TIMEMAP_DISPLAY_H
#define APOLLO_TIMEMAP_DISPLAY_H

//#include "Control.h"
//#include "Container.h"
#include "CompoundControl.h"
#include "ScrollArea.h"

#include "Audio.h"

//#include "Timing.h";
//#include "DataDisplay.h"

#include <unordered_map>
#include <vector>

class TimeMapNode;
template<typename T> class TrackNode;
class NodeConnector;
class AudioTrackNode;
class MidiTrackNode;

class Cursor;

class TrackControl;
class AudioDataDisplay;
class MidiDataDisplay;


class TimeMapDisplay : public ScrollArea
{
private:
	TimeMapNode					*node = nullptr;
	NodeConnector				*trackInput = nullptr;
	std::unordered_map<AudioTrackNode*, TrackControl*>	tracks;
	std::vector<TrackControl*>							trackOrder;	//Track controls in the order that theyre
																	//	shown in the display
	
	//Determines if the display will follow the TimeMapNode cursor as it's playing.
	//	This gets turned to false when the user adjusts the view manually, and then back to true
	//	once the node stops playing.
	bool						followCursor = true;

	Cursor						*cursor = nullptr;

public:
	TimeMapDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, TimeMapNode *tm_node);
	virtual ~TimeMapDisplay();

	//static const GuiPropFlags PROP_FLAGS;

	static const float SAMPLES_PER_PIXEL;		//Number of samples per pixel at zoom = 1
	static const float TRACK_HEIGHT;			//Height of a track control at zoom = 1
	static const AVec TRACK_PADDING;			//Padding on top/bottom/sides of tracks at zoom = 1

	static const float CURSOR_FOLLOW_PADDING;	//Distance cursor has to be to edge that the view offset
												//	will start following it

	static const float SCROLL_MULT;
	static const AVec ZOOM_STEP;

	void updateTracks();
	void initTracks();

	s_time getCursorPos();
	Cursor* getCursor();
	TimeMapNode* getNode();
	void setNode(TimeMapNode *tmn);

	//Returns v in virtual space so it will be consistent in absolute space
	//	if zoom == 1.0, v is unchanged.
	AVec correctVirtualVec(const AVec &v) const;

	virtual void onSizeChanged(AVec d_size) override;

	virtual void onScroll(APoint m_pos, AVec d_scroll, bool direct) override;
	
	virtual void update(double dt) override;
	virtual void draw(GlInterface &gl) override;
};


class TrackControl : public CompoundControl
{
private:
	TimeMapDisplay *tmd_parent = nullptr;

	AudioDataDisplay *audioDisp = nullptr;
	MidiDataDisplay *midiDisp = nullptr;

	AudioTrackNode *audioNode = nullptr;
	MidiTrackNode *midiNode = nullptr;
	
	virtual void onSizeChanged(AVec d_size) override;

public:
	TrackControl(TimeMapDisplay *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, AudioTrackNode *audio, MidiTrackNode *midi);
	virtual ~TrackControl();

	static const GuiPropFlags PROP_FLAGS;

	static AVec PADDING;

	AudioTrackNode* getAudioNode();
	MidiTrackNode* getMidiNode();
	
	virtual void draw(GlInterface &gl) override;
};

#endif	//APOLLO_TIMEMAP_DISPLAY_H