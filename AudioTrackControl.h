#ifndef APOLLO_TRACK_CONTROL_H
#define APOLLO_TRACK_CONTROL_H

#include "CompoundControl.h"

class AudioBufferNode;
class AudioDataDisplay;
class Cursor;

class AudioTrackControl : public CompoundControl
{
protected:
	AudioBufferNode		*node;
	AudioDataDisplay	*dataDisp = nullptr;

	float				samplesPerPixel;

	Cursor				*cursor = nullptr;

	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;

public:
	AudioTrackControl(ParentElement *parent_, APoint a_pos, float height, GuiStateFlags s_flags, AudioBufferNode *t_node, float samples_per_pixel);
	virtual ~AudioTrackControl();

	static const GuiPropFlags	PROP_FLAGS;
	static const AVec			PADDING;

	void setCursor(Cursor *p_cursor);

	void setTrackNode(AudioBufferNode *new_node);

	void setSamplesPerPixel(float samples_per_pixel);

	virtual void draw(GlInterface &gl) override;
};

#endif	//APOLLO_TRACK_CONTROL_H