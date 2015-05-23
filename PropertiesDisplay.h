#ifndef APOLLO_PROPERTIES_DISPLAY_H
#define APOLLO_PROPERTIES_DISPLAY_H

#include "Container.h"
#include "ScrollArea.h"

#include "Label.h"
#include "Buttons.h"
#include "Checkbox.h"

//class TimeMapNode;

class AudioDataDisplay;
class WaveControl;
class ProjectTrackDisplay;

class Node;

class PropDisplay;
class AudioTrackPropDisplay;
class MidiTrackPropDisplay;
class AudioModTrackPropDisplay;
class MidiModTrackPropDisplay;
class RenderPropDisplay;
class SpeakerPropDisplay;
//class TimeMapPropDisplay;
class InstrumentPropDisplay;

class PropertiesDisplay : public Container
{
private:
	Node *activeNode = nullptr;
	
	//ScrollArea *scroll = nullptr;
	std::vector<GuiElement*> displays;

	//Updates children elements to reflect current active node
	void updateDisplay();

public:
	PropertiesDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, ProjectTrackDisplay *ptd);
	virtual ~PropertiesDisplay();

	static const GuiPropFlags PROP_FLAGS;
	static const AVec PADDING;

	void setActiveNode(Node *node);
	Node* getActiveNode() const;

	virtual void draw(GlInterface &gl) override;
};


class PropDisplay : public ScrollArea
{
protected:
	Node *node = nullptr;
	Label titleLabel;

	void setTitle(std::string title);
	
	virtual void onSizeChanged(AVec d_size) override;

public:
	PropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, std::string title);
	virtual ~PropDisplay() = default;

	static const GuiStateFlags STATE_FLAGS;
	static const GuiPropFlags PROP_FLAGS;

	void setNode(Node *n);

	virtual void update() { };
};



//Displays properties for an AudioTrackNode
class AudioTrackPropDisplay : public PropDisplay
{
private:
	ProjectTrackDisplay		*trackDisp = nullptr;
	ImageButton				recordToggle;
	Checkbox				displayCheck;

public:
	AudioTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd);
	virtual ~AudioTrackPropDisplay() = default;

	void toggleRecord();
	void displayChanged();
};

//Displays properties for a MidiTrackNode
class MidiTrackPropDisplay : public PropDisplay
{
private:
	ProjectTrackDisplay		*trackDisp = nullptr;
	ImageButton				recordToggle;

public:
	MidiTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd);
	virtual ~MidiTrackPropDisplay() = default;
	
	void toggleRecord();
};

//Displays properties for an AudioModBufferNode
class AudioModTrackPropDisplay : public PropDisplay
{
private:

public:
	AudioModTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~AudioModTrackPropDisplay() = default;
};

//Displays properties for a MidiModBufferNode
class MidiModTrackPropDisplay : public PropDisplay
{
private:

public:
	MidiModTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~MidiModTrackPropDisplay() = default;
};

//Displays properties for a RenderNode
class RenderPropDisplay : public PropDisplay
{
private:
	AudioDataDisplay *bufferDisp = nullptr;

public:
	RenderPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~RenderPropDisplay();

	virtual void update() override;
};

//Displays properties for a SpeakerNode
class SpeakerPropDisplay : public PropDisplay
{
private:
	AudioDataDisplay *bufferDisp = nullptr;

public:
	SpeakerPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~SpeakerPropDisplay();
	
	virtual void update() override;
};

//Displays properties for a MidiDeviceNode
class MidiDevicePropDisplay : public PropDisplay
{
private:

public:
	MidiDevicePropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~MidiDevicePropDisplay() = default;
};

//Displays properties for a TimeMapNode
/*
class TimeMapPropDisplay : public PropDisplay
{
private:

public:
	TimeMapPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~TimeMapPropDisplay() = default;
};
*/
	
//Displays properties for a WaveformNode
class InstrumentPropDisplay : public PropDisplay
{
private:
	WaveControl *waveControl = nullptr;

public:
	InstrumentPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size);
	virtual ~InstrumentPropDisplay();

	virtual void update() override;

};



#endif	//APOLLO_PROPERTIES_DISPLAY_H