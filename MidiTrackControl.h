#ifndef APOLLO_MIDI_TRACK_CONTROL_H
#define APOLLO_MIDI_TRACK_CONTROL_H

#include "CompoundControl.h"

class MidiTrackNode;
class MidiDataDisplay;

class Cursor;

class MidiTrackControl : public CompoundControl
{
protected:
	MidiTrackNode		*node;
	MidiDataDisplay		*dataDisp = nullptr;

	float				secondsPerPixel;

	Cursor				*cursor = nullptr;
	
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;

public:
	MidiTrackControl(ParentElement *parent_, APoint a_pos, float height, GuiStateFlags s_flags, MidiTrackNode *t_node, float seconds_per_pixel = -1.0f);
	virtual ~MidiTrackControl();

	static const GuiPropFlags	PROP_FLAGS;
	static const AVec			PADDING;

	void setCursor(Cursor *p_cursor);

	void setSecondsPerPixel(float seconds_per_pixel);

	void setTrackNode(MidiTrackNode *new_node);

	virtual void draw(GlInterface &gl) override;
};

#endif	//APOLLO_MIDI_TRACK_CONTROL_H