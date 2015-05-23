#include "PropertiesDisplay.h"

#include "Waveform.h"
#include "WaveControl.h"
#include "ScrollArea.h"
#include "Node.h"
#include "IONodes.h"
#include "TrackNodes.h"
#include "TimeMapNode.h"
#include "RenderNode.h"

#include "AudioDataDisplay.h"
#include "ProjectTrackDisplay.h"

#include "SimpleContainers.h"

#include "AUtility.h"

/////PROPERTIES DISPLAY/////
const GuiPropFlags PropertiesDisplay::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;
const AVec PropertiesDisplay::PADDING(5.0f, 5.0f);

PropertiesDisplay::PropertiesDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		Container(GuiProps(s_flags, PROP_FLAGS)),
		displays(toIndex(NType::COUNT), nullptr)
{
	//std::vector<GuiElement*> children;

	//scroll = new ScrollArea(this, APoint(0, 0), size);

	//attachElement(scroll, this, Side::LEFT, 0);
	//attachElement(scroll, this, Side::TOP, 0);
	//attachElement(scroll, this, Side::RIGHT, 0);
	//attachElement(scroll, this, Side::BOTTOM, 0);

	displays[toIndex(NType::NONE)] = new Area(this, APoint(0, 0), a_size, DEFAULT_STATE);
	displays[toIndex(NType::AUDIO_TRACK)] = new AudioTrackPropDisplay(this, APoint(0, 0), a_size, ptd);
	displays[toIndex(NType::MIDI_TRACK)] = new MidiTrackPropDisplay(this, APoint(0, 0), a_size, ptd);
	displays[toIndex(NType::AUDIO_MOD_TRACK)] = new AudioModTrackPropDisplay(this, APoint(0, 0), a_size);
	displays[toIndex(NType::MIDI_MOD_TRACK)] = new MidiModTrackPropDisplay(this, APoint(0, 0), a_size);
	displays[toIndex(NType::RENDER)] = new RenderPropDisplay(this, APoint(0, 0), a_size);
	displays[toIndex(NType::SPEAKER)] = new SpeakerPropDisplay(this, APoint(0, 0), a_size);
	//displays[toIndex(NType::TIME_MAP)] = new TimeMapPropDisplay(this, APoint(0, 0), a_size);
	displays[toIndex(NType::INSTRUMENT)] = new InstrumentPropDisplay(this, APoint(0, 0), a_size);
	displays[toIndex(NType::MIDI_DEVICE)] = new MidiDevicePropDisplay(this, APoint(0, 0), a_size);

	//Warn if not all node types accounted for
	if(!displays[displays.size() - 1])
		std::cout << "\n\n-----------------------------------\n									\
					      WARNING: NOT ALL NODE TYPES ACCOUNTED FOR IN PROPERTIES DISPLAY!\n	\
						  -----------------------------------\n\n\n";

	for(unsigned int i = 0; i < displays.size(); i++)
	{
		if(displays[i])
		{
			displays[i]->attachTo(this, AttachSide::LEFT, 0);
			displays[i]->attachTo(this, AttachSide::TOP, 0);
			displays[i]->attachTo(this, AttachSide::RIGHT, 0);
			displays[i]->attachTo(this, AttachSide::BOTTOM, 0);
			displays[i]->hide();
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
	for(unsigned int i = 0; i < displays.size(); i++)
		AU::safeDelete(displays[i]);

	displays.clear();
}

void PropertiesDisplay::updateDisplay()
{
	//removeAllBody();
	//clearChildren();

	if(activeNode && activeNode->getType() < NType::COUNT && displays[toIndex(activeNode->getType())])
	{
		dynamic_cast<PropDisplay*>(displays[toIndex(activeNode->getType())])->setNode(activeNode);
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
		ScrollArea(parent_, a_pos, a_size, STATE_FLAGS), titleLabel(this, APoint(), DEFAULT_STATE, title, 25)
{
	onSizeChanged(AVec());
}

void PropDisplay::setTitle(std::string title)
{
	titleLabel.setText(title);
	onSizeChanged(AVec());
}

void PropDisplay::onSizeChanged(AVec d_size)
{
	titleLabel.centerAround(APoint(size.x/2.0f, 30));
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



/////AUDIO BUFFER PROP DISPLAY/////
AudioTrackPropDisplay::AudioTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "AUDIO TRACK"),
		recordToggle(this, APoint(5, 100), DEFAULT_STATE, "Record"), trackDisp(ptd),
		displayCheck(this, APoint(5, 150), DEFAULT_STATE, "DISPLAY")
{
	recordToggle.setClickFunction(std::bind(&AudioTrackPropDisplay::toggleRecord, this));
	displayCheck.setCallback(std::bind(&AudioTrackPropDisplay::displayChanged, this));
}

void AudioTrackPropDisplay::toggleRecord()
{
	/*
	AudioTrackNode *at_node = dynamic_cast<AudioTrackNode*>(node);
	if(at_node && tmn)
	{
		bool now_recording = !at_node->isRecording();

		at_node->setRecording(now_recording ? tmn->getCursor() : nullptr);
		tmn->setPlaying(now_recording);
	}
	*/
}

void AudioTrackPropDisplay::displayChanged()
{
	if(trackDisp && node)
	{
		std::cout << displayCheck.getValue() << "\n";

		if(displayCheck.getValue())
			trackDisp->addTrack(dynamic_cast<AudioTrackNode*>(node));
		else
			trackDisp->removeTrack(dynamic_cast<AudioTrackNode*>(node));
	}
}



/////MIDI BUFFER PROP DISPLAY/////
MidiTrackPropDisplay::MidiTrackPropDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, ProjectTrackDisplay *ptd)
	: GuiElement(parent_, a_pos, a_size, GuiProps(STATE_FLAGS, PROP_FLAGS)),
		PropDisplay(parent_, a_pos, a_size, "MIDI TRACK"),
		recordToggle(this, APoint(5, 100), DEFAULT_STATE, "Record"), trackDisp(ptd)
{
	recordToggle.setClickFunction(std::bind(&MidiTrackPropDisplay::toggleRecord, this));
}

void MidiTrackPropDisplay::toggleRecord()
{
	/*
	MidiTrackNode *mt_node = dynamic_cast<MidiTrackNode*>(node);
	if(mt_node && tmn)
	{
		bool now_recording = !mt_node->isRecording();

		mt_node->setRecording(now_recording ? tmn->getCursor() : nullptr);
		tmn->setPlaying(now_recording);
	}
	*/
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
		bufferDisp->setData((AudioData*)(n->getBuffer()), true, true, false, -1.0f);
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