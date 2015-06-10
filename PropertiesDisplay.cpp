#include "PropertiesDisplay.h"

#include "Waveform.h"
#include "WaveControl.h"
#include "ScrollArea.h"
#include "Node.h"
#include "SpeakerNode.h"
#include "MicrophoneNode.h"
#include "InstrumentNode.h"
#include "MidiDeviceNode.h"
#include "TimeMapNode.h"
#include "RenderNode.h"

#include "AudioDataDisplay.h"
#include "ProjectTrackDisplay.h"

#include "SimpleContainers.h"

#include "AudioBufferNode.h"
#include "MidiBufferNode.h"

#include "AUtility.h"

/////PROPERTIES DISPLAY/////
const GuiPropFlags PropertiesDisplay::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;
const AVec PropertiesDisplay::PADDING(5.0f, 5.0f);

PropertiesDisplay::PropertiesDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		Container(GuiProps(s_flags, PROP_FLAGS))
{
	//std::vector<GuiElement*> children;

	//scroll = new ScrollArea(this, APoint(0, 0), size);

	//attachElement(scroll, this, Side::LEFT, 0);
	//attachElement(scroll, this, Side::TOP, 0);
	//attachElement(scroll, this, Side::RIGHT, 0);
	//attachElement(scroll, this, Side::BOTTOM, 0);

	displays.reserve(toIndex(NodeType::COUNT));

	//NONE
	displays.push_back(std::unique_ptr<PropDisplay>(new PropDisplay(this, APoint(0, 0), a_size, "(No Node Selected)")));
	//STATIC_AUDIO_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new StaticAudioBufferPropDisplay(this, APoint(0, 0), a_size, ptd)));
	//STATIC_MIDI_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new StaticMidiBufferPropDisplay(this, APoint(0, 0), a_size, ptd)));
	//DYNAMIC_AUDIO_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new DynamicAudioBufferPropDisplay(this, APoint(0, 0), a_size, ptd)));
	//DYNAMIC_MIDI_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new DynamicMidiBufferPropDisplay(this, APoint(0, 0), a_size, ptd)));
	//STATIC_MOD_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new AudioModTrackPropDisplay(this, APoint(0, 0), a_size)));
	//DYNAMIC_MOD_BUFFER
	displays.push_back(std::unique_ptr<PropDisplay>(new MidiModTrackPropDisplay(this, APoint(0, 0), a_size)));
	//READ
	displays.push_back(std::unique_ptr<PropDisplay>(new PropDisplay(this, APoint(0, 0), a_size, "TODO: Read Node")));
	//WRITE
	displays.push_back(std::unique_ptr<PropDisplay>(new PropDisplay(this, APoint(0, 0), a_size, "TODO: Write Node")));
	//RENDER
	displays.push_back(std::unique_ptr<PropDisplay>(new RenderPropDisplay(this, APoint(0, 0), a_size)));
	//SPEAKER
	displays.push_back(std::unique_ptr<PropDisplay>(new SpeakerPropDisplay(this, APoint(0, 0), a_size)));
	//MICROPHONE
	displays.push_back(std::unique_ptr<PropDisplay>(new MicrophonePropDisplay(this, APoint(0, 0), a_size)));
	//INSTRUMENT
	displays.push_back(std::unique_ptr<PropDisplay>(new InstrumentPropDisplay(this, APoint(0, 0), a_size)));
	//MIDI_DEVICE
	displays.push_back(std::unique_ptr<PropDisplay>(new MidiDevicePropDisplay(this, APoint(0, 0), a_size)));

	//Warn if not all node types accounted for
	if(!displays[displays.size() - 1])
		std::cout << "\n\n-----------------------------------\n									\
					      WARNING: NOT ALL NODE TYPES ACCOUNTED FOR IN PROPERTIES DISPLAY!\n	\
						  -----------------------------------\n\n\n";

	for(auto &d : displays)
	{
		if(d)
		{
			d->attachTo(this, AttachSide::LEFT, 0);
			d->attachTo(this, AttachSide::TOP, 0);
			d->attachTo(this, AttachSide::RIGHT, 0);
			d->attachTo(this, AttachSide::BOTTOM, 0);
			d->hide();
		}
	}

	//scroll->addBody(displays[toIndex(NType::NONE)]);
	
	//children.push_back(scroll);
	//init(&children, nullptr);

	setAllBgStateColors(Color(0.1f, 0.2f, 0.3f, 1.0f));

	updateDisplay();
}

PropertiesDisplay::~PropertiesDisplay()
{
	//for(auto &d : displays)
	//{
	//	if(d)
	//		delete d;
	//	d = nullptr;
	//}

	displays.clear();
}

void PropertiesDisplay::updateDisplay()
{
	//removeAllBody();
	//clearChildren();

	if(activeNode && activeNode->getType() < NType::COUNT && displays[toIndex(activeNode->getType())])
	{
		dynamic_cast<PropDisplay*>(displays[toIndex(activeNode->getType())].get())->setNode(activeNode);
		displays[toIndex(activeNode->getType())]->show();
	}
	else
		displays[toIndex(NType::NONE)]->show();
}

void PropertiesDisplay::setActiveNode(Node *node)
{
	bool update = (node != activeNode);

	if(update && activeNode && displays[toIndex(activeNode->getType())])
		displays[toIndex(activeNode->getType())]->hide();

	activeNode = node;

	if(update)
		updateDisplay();
}

Node* PropertiesDisplay::getActiveNode() const
{
	return activeNode;
}


////PROP DISPLAY BASE/////
const GuiStateFlags PropDisplay::STATE_FLAGS = DEFAULT_STATE;
const GuiPropFlags PropDisplay::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;

PropDisplay::PropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, std::string title)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		ScrollArea(parent_, a_pos, a_size, STATE_FLAGS),
		titleLabel(new Label(this, APoint(0.0f, 0.0f), DEFAULT_STATE, title, 25))
{
	onSizeChanged(AVec(0.0f, 0.0f));
}

void PropDisplay::setTitle(std::string title)
{
	titleLabel->setText(title);
	onSizeChanged(AVec(0.0f, 0.0f));
}

void PropDisplay::onSizeChanged(AVec d_size)
{
	titleLabel->centerAround(APoint(size.x/2.0f, 30));
}

void PropDisplay::setNode(Node *n)
{
	node = n;
	update();
}

void PropertiesDisplay::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}



/////STATIC AUDIO BUFFER PROP DISPLAY/////
StaticAudioBufferPropDisplay::StaticAudioBufferPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "AUDIO TRACK"),
		recordCheck(this, APoint(5, 100), DEFAULT_STATE, "RECORD"), trackDisp(ptd),
		displayCheck(this, APoint(5, 150), DEFAULT_STATE, "DISPLAY")
{
	recordCheck.setCallback(std::bind(&StaticAudioBufferPropDisplay::updateRecord, this));
	displayCheck.setCallback(std::bind(&StaticAudioBufferPropDisplay::displayChanged, this));
}

StaticAudioBufferPropDisplay::~StaticAudioBufferPropDisplay()
{

}


void StaticAudioBufferPropDisplay::updateRecord()
{
	StaticAudioBufferNode *a_node = dynamic_cast<StaticAudioBufferNode*>(node);
	if(a_node)
	{
		a_node->setRecording(isRecording() ? trackDisp->getCursor() : nullptr);
	}
}

void StaticAudioBufferPropDisplay::displayChanged()
{
	if(trackDisp && node)
	{
		std::cout << displayCheck.getValue() << "\n";

		if(displayCheck.getValue())
			trackDisp->addTrack(dynamic_cast<AudioBufferNode*>(node));
		else
			trackDisp->removeTrack(dynamic_cast<AudioBufferNode*>(node));
	}
}

bool StaticAudioBufferPropDisplay::isRecording() const
{
	return recordCheck.getValue();
}


/////DYNAMIC AUDIO BUFFER PROP DISPLAY/////
DynamicAudioBufferPropDisplay::DynamicAudioBufferPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "AUDIO TRACK"),
		trackDisp(ptd), displayCheck(this, APoint(5, 150), DEFAULT_STATE, "DISPLAY")
{
	displayCheck.setCallback(std::bind(&DynamicAudioBufferPropDisplay::displayChanged, this));
}

void DynamicAudioBufferPropDisplay::displayChanged()
{
	if(trackDisp && node)
	{
		std::cout << displayCheck.getValue() << "\n";

		if(displayCheck.getValue())
			trackDisp->addTrack(dynamic_cast<AudioBufferNode*>(node));
		else
			trackDisp->removeTrack(dynamic_cast<AudioBufferNode*>(node));
	}
}



/////STATIC MIDI BUFFER PROP DISPLAY/////
StaticMidiBufferPropDisplay::StaticMidiBufferPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "MIDI TRACK"),
		recordCheck(this, APoint(5, 100), DEFAULT_STATE, "RECORD"), trackDisp(ptd)
{
	recordCheck.setCallback(std::bind(&StaticMidiBufferPropDisplay::updateRecord, this));
}

void StaticMidiBufferPropDisplay::updateRecord()
{
	StaticMidiBufferNode *m_node = dynamic_cast<StaticMidiBufferNode*>(node);
	if(m_node)
	{
		m_node->setRecording(isRecording() ? trackDisp->getCursor() : nullptr);
	}
}

bool StaticMidiBufferPropDisplay::isRecording() const
{
	return recordCheck.getValue();
}


/////DYNAMIC MIDI BUFFER PROP DISPLAY/////
DynamicMidiBufferPropDisplay::DynamicMidiBufferPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "MIDI TRACK"), trackDisp(ptd)
{

}


/////AUDIO MOD BUFFER PROP DISPLAY/////
AudioModTrackPropDisplay::AudioModTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "AUDIO MOD TRACK")
{

}




/////MIDI MOD BUFFER PROP DISPLAY/////
MidiModTrackPropDisplay::MidiModTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "MIDI MOD TRACK")
{

}




/////RENDER PROP DISPLAY/////
RenderPropDisplay::RenderPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "RENDER")
{
	AVec PADDING(5.0f, 5.0f);

	bufferDisp = new AudioDataDisplay(this, PADDING, AVec(size.x - 2.0f*PADDING.x, 200), DEFAULT_STATE, nullptr, GraphProps::NONE, true, true, false, -1.0f); 
	bufferDisp->attachTo(this, Side::LEFT, PADDING.x);
	bufferDisp->attachTo(this, Side::RIGHT, PADDING.x);
	bufferDisp->attachTo(this, Side::TOP, PADDING.y + 50.0f);

	//addChild(bufferDisp, false);
}

RenderPropDisplay::~RenderPropDisplay()
{
	AU::safeDelete(bufferDisp);
}

void RenderPropDisplay::update()
{
	RenderNode *n = dynamic_cast<RenderNode*>(node);
	
	if(n)
		bufferDisp->setData((const AudioData*)(n->getBuffer()), true, true, false, -1.0f);
}



/////SPEAKER PROP DISPLAY/////
SpeakerPropDisplay::SpeakerPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "SPEAKER")
{
	AVec PADDING(5.0f, 5.0f);

	bufferDisp = new AudioDataDisplay(this, PADDING, AVec(size.x - 2.0f*PADDING.x, 200), DEFAULT_STATE, nullptr, GraphProps::NONE, true, true, false, -1.0f); 
	bufferDisp->attachTo(this, Side::LEFT, PADDING.x);
	bufferDisp->attachTo(this, Side::RIGHT, PADDING.x);
	bufferDisp->attachTo(this, Side::TOP, PADDING.y + 50.0f);

	//addChild(bufferDisp, false);
}

SpeakerPropDisplay::~SpeakerPropDisplay()
{
	AU::safeDelete(bufferDisp);
}
 
void SpeakerPropDisplay::update()
{
	SpeakerNode *n = dynamic_cast<SpeakerNode*>(node);
	
	if(n)
		bufferDisp->setData(n->getBuffer(), true, true, false, -1.0f);
}


/////MICROPHONE PROP DISPLAY/////
MicrophonePropDisplay::MicrophonePropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "SPEAKER")
{
	AVec PADDING(5.0f, 5.0f);

	bufferDisp = new AudioDataDisplay(this, PADDING, AVec(size.x - 2.0f*PADDING.x, 200), DEFAULT_STATE, nullptr, GraphProps::NONE, true, true, false, -1.0f); 
	bufferDisp->attachTo(this, Side::LEFT, PADDING.x);
	bufferDisp->attachTo(this, Side::RIGHT, PADDING.x);
	bufferDisp->attachTo(this, Side::TOP, PADDING.y + 50.0f);

	//addChild(bufferDisp, false);
}

MicrophonePropDisplay::~MicrophonePropDisplay()
{
	AU::safeDelete(bufferDisp);
}
 
void MicrophonePropDisplay::update()
{
	MicrophoneNode *n = dynamic_cast<MicrophoneNode*>(node);
	
	if(n)
		bufferDisp->setData(n->getBuffer(), true, true, false, -1.0f);
}


/////MIDI DEVICE PROP DISPLAY/////
MidiDevicePropDisplay::MidiDevicePropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "MIDI DEVICE")
{

}




/////TIME MAP PROP DISPLAY/////
/*
TimeMapPropDisplay::TimeMapPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "TIME MAP")
{

}
*/



/////WAVEFORM PROP DISPLAY/////

InstrumentPropDisplay::InstrumentPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "WAVEFORM")
{
	//std::vector<GuiElement*> children;

	//Initialize contents
	AVec PADDING(5.0f, 5.0f);
	waveControl = new WaveControl(this, PADDING, AVec(size.x - 2.0f*PADDING.x, 200), DEFAULT_STATE_FLOAT, nullptr);
	waveControl->attachTo(this, Side::LEFT, PADDING.x);
	waveControl->attachTo(this, Side::RIGHT, PADDING.x);
	waveControl->attachTo(this, Side::TOP, PADDING.y + 50.0f);
	//children.push_back(waveControl);

	//addChild(waveControl, false);

	//init(&children, nullptr);
}

InstrumentPropDisplay::~InstrumentPropDisplay()
{
	AU::safeDelete(waveControl);
}

void InstrumentPropDisplay::update()
{
	InstrumentNode *in = dynamic_cast<InstrumentNode*>(node);
	if(in)
	{
		WaveSynth *ws = dynamic_cast<WaveSynth*>(in->getInstrument());
		if(ws)
			waveControl->setWaveform(ws->getWaveform());
	}
}