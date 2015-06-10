#ifndef APOLLO_PROJECT_TRACK_DISPLAY_H
#define APOLLO_PROJECT_TRACK_DISPLAY_H

#include "CompoundControl.h"
#include "ScrollArea.h"

#include "CollapseList.h"
#include "Range.h"

class Cursor;

class AudioBufferNode;
//class CollapseList;
class AudioTrackControl;
class MidiTrackControl;

class ProjectTrackDisplay : public ScrollArea
{
protected:
	std::vector<AudioBufferNode*>	trackNodes;
	std::vector<AudioTrackControl*> audioTrackControls;
	std::vector<MidiTrackControl*>	midiTrackControls;

	std::vector<ElementTree>		trackTrees;
	CollapseList					*trackList = nullptr;

	Range<float>					trackRange;	//The range of the actual tracks (minus padding)
									

	float							sampleToAbsolute,
									absoluteToSample,
									cursor_pos;

	Cursor							*cursor;
	
	virtual void onSizeChanged(AVec d_size) override;

	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onScroll(APoint m_pos, AVec d_scroll, bool direct) override;

public:
	ProjectTrackDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, Cursor *track_cursor);
	virtual ~ProjectTrackDisplay();

	static const GuiPropFlags	PROP_FLAGS;
	static const float			TRACK_HEIGHT,
								X_OFFSET_STEP,
								SAMPLES_PER_PIXEL,
								CURSOR_FOLLOW_OFFSET;
	
	void addTrack(AudioBufferNode *atn);
	void removeTrack(AudioBufferNode *atn);

	bool isPlaying() const;
	void play();
	void stop();

	void setCursor(Cursor *new_cursor);
	Cursor* getCursor();

	void updateCursorPos();

	//virtual void update(const Time &dt) override;

	virtual void draw(GlInterface &gl) override;
};


#endif	//APOLLO_PROJECT_TRACK_DISPLAY_H