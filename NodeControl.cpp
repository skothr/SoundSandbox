#include "NodeControl.h"

#include "NodeConnection.h"
#include "NodeElementContainer.h"
#include "Label.h"
#include "SpeakerNode.h"
#include "MicrophoneNode.h"
#include "InstrumentNode.h"
#include "MidiDeviceNode.h"
#include "InfoNodes.h"
#include "RenderNode.h"
#include "NodeGraphControl.h"
#include "NodeConnectionControl.h"

#include "AudioBufferNode.h"
#include "MidiBufferNode.h"
#include "ModBufferNode.h"

/////NODE CONTROL/////
const GuiStateFlags NodeControl::STATE_FLAGS = DEFAULT_STATE;
const GuiPropFlags NodeControl::PROP_FLAGS = PFlags::HARD_BACK;

const AVec	NodeControl::BRANCH_SIZE = AVec(100.0f, 50.0f),
			NodeControl::LEAF_SIZE = AVec(75.0f, 50.0f),
			NodeControl::DEFAULT_SIZE = NodeControl::BRANCH_SIZE;

NodeControl::NodeControl(NodeGraphControl *parent_, APoint g_pos, GuiStateFlags s_flags, Node *n)
	: GuiElement(parent_, g_pos, AVec(), GuiProps(s_flags, PROP_FLAGS)),
		NodeElement(parent_, s_flags),
		node(n), ngc_parent(parent_), connectorControls(toIndex(IOType::COUNT), nullptr),
		nameLabel(this, APoint(), DEFAULT_STATE, node->name, 15.0f)
{
	setAllBgStateColors(Color(1.0f, 0.0f, 1.0f, 1.0f));

	setBgStateColor(Color(0.25f, 0.25f, 0.25f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(0.4f, 0.4f, 0.4f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(0.4f, 0.4f, 0.4f, 1.0f), CS::DRAGGING);
	setBgStateColor(Color(0.4f, 0.4f, 0.4f, 1.0f), CS::DROP_HOVERING);
	
	setHeight(getNodeSize().y);
	setWidth(nameLabel.getSize().x + 8);

	nameLabel.centerAround(APoint(size*(1.0f/2.0f)));
	
	node->graphPos = pos;

	//Initialize connector controls
	//std::vector<NodeConnector*> con = ;
	for(auto &c : node->connectors)
		connectorControls[toIndex(c.second->ioType)] = new NodeConnectorControl(this, DEFAULT_STATE, c.second->getId());
}

NodeControl::~NodeControl()
{
	for(auto cc : connectorControls)
		if(cc) delete cc;
	connectorControls.clear();
}

Node* NodeControl::getNode()
{
	return node;
}

AVec NodeControl::getNodeSize()
{
	if(node)
	{
		switch(node->getType())
		{
		case NType::STATIC_AUDIO_BUFFER:
		case NType::STATIC_MIDI_BUFFER:
		case NType::DYNAMIC_AUDIO_BUFFER:
		case NType::DYNAMIC_MIDI_BUFFER:
		case NType::STATIC_MOD_BUFFER:
		case NType::DYNAMIC_MOD_BUFFER:
			return BRANCH_SIZE;
		case NType::RENDER:
			return BRANCH_SIZE;
		case NType::SPEAKER:
		case NType::MICROPHONE:
		case NType::INSTRUMENT:
			return LEAF_SIZE;
		}
	}

	return DEFAULT_SIZE;
}

APoint NodeControl::getConnectorPoint(NodeConnectionType io_type)
{
	AVec half_size = size*(1.0f/2.0f);
	switch(io_type)
	{
	case IOType::DATA_INPUT:
		return pos + AVec(0.0f, half_size.y);
	case IOType::DATA_OUTPUT:
		return pos + AVec(size.x, half_size.y);

	case IOType::INFO_INPUT:
		return pos + AVec(half_size.x, 0.0f);
	case IOType::INFO_OUTPUT:
		return pos + AVec(half_size.x, size.y);

	default:
		return APoint(0, 0);
	}
}

NodeConnector* NodeControl::getDefaultConnector()
{
	if(node)
	{
		NCID id = -1;

		switch(node->getType())
		{
		case NType::STATIC_AUDIO_BUFFER:
			id = dynamic_cast<StaticAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::STATIC_MIDI_BUFFER:
			id = dynamic_cast<StaticMidiBufferNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::DYNAMIC_AUDIO_BUFFER:
			id = dynamic_cast<DynamicAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::DYNAMIC_MIDI_BUFFER:
			id = dynamic_cast<DynamicMidiBufferNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::STATIC_MOD_BUFFER:
			id = dynamic_cast<StaticModBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::DYNAMIC_MOD_BUFFER:
			id = dynamic_cast<DynamicModBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::RENDER:
			id = dynamic_cast<RenderNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::MIDI_DEVICE:
			id = dynamic_cast<MidiDeviceNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::SPEAKER:
			id = dynamic_cast<SpeakerNode*>(node)->INPUTS.AUDIO_ID;
			break;
		case NType::MICROPHONE:
			id = dynamic_cast<MicrophoneNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::INSTRUMENT:
			id = dynamic_cast<InstrumentNode*>(node)->OUTPUTS.INSTRUMENT_ID;
			break;
		}

		return NodeConnector::getNC(id);
	}
	else
		return nullptr;
}


void NodeControl::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	NodeConnectionControl *movingConnection = ngc_parent->getMovingConnection();

	if(node && direct && movingConnection)//valid(b & (MouseButton::MIDDLE)))
	{
		NCID nc_id;

		bool need_output = isInput(movingConnection->getHangingNode()->ioType);
		//Get matching output
		switch(node->getType())
		{
			case NType::STATIC_AUDIO_BUFFER:
				nc_id = need_output ? dynamic_cast<StaticAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID
									: dynamic_cast<StaticAudioBufferNode*>(node)->INPUTS.AUDIO_ID;
				break;
			case NType::STATIC_MIDI_BUFFER:
				nc_id = need_output ? dynamic_cast<StaticMidiBufferNode*>(node)->OUTPUTS.MIDI_ID
									: dynamic_cast<StaticMidiBufferNode*>(node)->INPUTS.MIDI_ID;
				break;
			case NType::DYNAMIC_AUDIO_BUFFER:
				nc_id = need_output ? dynamic_cast<DynamicAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID
									: dynamic_cast<DynamicAudioBufferNode*>(node)->INPUTS.AUDIO_ID;
				break;
			case NType::DYNAMIC_MIDI_BUFFER:
				nc_id = need_output ? dynamic_cast<DynamicMidiBufferNode*>(node)->OUTPUTS.MIDI_ID
									: dynamic_cast<DynamicMidiBufferNode*>(node)->INPUTS.MIDI_ID;
				break;
			case NType::STATIC_MOD_BUFFER:
				nc_id = need_output ? dynamic_cast<StaticModBufferNode*>(node)->OUTPUTS.AUDIO_ID
									: dynamic_cast<StaticModBufferNode*>(node)->INPUTS.AUDIO_ID;
				break;
			case NType::DYNAMIC_MOD_BUFFER:
				nc_id = need_output ? dynamic_cast<DynamicModBufferNode*>(node)->OUTPUTS.AUDIO_ID
									: dynamic_cast<DynamicModBufferNode*>(node)->INPUTS.AUDIO_ID;
				break;
			case NType::RENDER:
				nc_id = need_output ? dynamic_cast<RenderNode*>(node)->OUTPUTS.AUDIO_ID
									: dynamic_cast<RenderNode*>(node)->INPUTS.MIDI_ID;
				break;
			case NType::SPEAKER:
				nc_id = need_output ? -1
									: dynamic_cast<SpeakerNode*>(node)->INPUTS.AUDIO_ID;
				break;
			case NType::MICROPHONE:
				nc_id = need_output ? dynamic_cast<MicrophoneNode*>(node)->OUTPUTS.AUDIO_ID
									: -1;
				break;
			case NType::INSTRUMENT:
				nc_id = need_output ? dynamic_cast<InstrumentNode*>(node)->OUTPUTS.INSTRUMENT_ID
									: -1;
				break;
			case NType::MIDI_DEVICE:
				nc_id = need_output ? dynamic_cast<MidiDeviceNode*>(node)->OUTPUTS.MIDI_ID
									: -1;
				break;
			default:
				nc_id = -1;
		}

		ngc_parent->finishConnect(nc_id);
	}
	
	NodeElement::onMouseUp(m_pos, b, direct);
}

void NodeControl::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && valid(b & MouseButton::MIDDLE))
	{
		NCID nc_id;

		switch(node->getType())
		{
		case NType::STATIC_AUDIO_BUFFER:
			nc_id = dynamic_cast<StaticAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::STATIC_MIDI_BUFFER:
			nc_id = dynamic_cast<StaticMidiBufferNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::DYNAMIC_AUDIO_BUFFER:
			nc_id = dynamic_cast<DynamicAudioBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::DYNAMIC_MIDI_BUFFER:
			nc_id = dynamic_cast<DynamicMidiBufferNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::STATIC_MOD_BUFFER:
			nc_id = dynamic_cast<StaticModBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::DYNAMIC_MOD_BUFFER:
			nc_id = dynamic_cast<DynamicModBufferNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::RENDER:
			nc_id = dynamic_cast<RenderNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::SPEAKER:
			nc_id = dynamic_cast<SpeakerNode*>(node)->INPUTS.AUDIO_ID;
			break;
		case NType::MICROPHONE:
			nc_id = dynamic_cast<MicrophoneNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::INSTRUMENT:
			nc_id = dynamic_cast<InstrumentNode*>(node)->OUTPUTS.INSTRUMENT_ID;
			break;
		case NType::MIDI_DEVICE:
			nc_id = dynamic_cast<MidiDeviceNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		default:
			nc_id = -1;
		}

		ngc_parent->startConnect(nc_id);
	}
	
	NodeElement::onMouseDown(m_pos, b, direct);
}


void NodeControl::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	NodeConnectionControl *movingConnection = ngc_parent->getMovingConnection();

	if(direct && movingConnection)
	{
		NodeConnector	*other_nc = movingConnection->getHangingNode(),
						*this_nc = getDefaultConnector();
		
		if(NodeConnector::validConnection(this_nc->getId(), other_nc->getId()))
		{
			//Snap to connector location
			movingConnection->setHangingPos(getConnectorPoint(this_nc->ioType));
		}
		else
		{
			//Snap to (???)
			//movingConnection->setHangingNode(this_nc);
		}
	}
	
	NodeElement::onDrag(m_pos, d_pos, direct);
}

void NodeControl::onPosChanged(AVec d_pos)
{
	NodeElement::onPosChanged(d_pos);
	node->graphPos = pos;
}


void NodeControl::setHighlight(IOType io_type, NodeDataType data_type)
{
	if(io_type == IOType::INVALID)
	{
		for(auto hc : connectorControls)
			if(hc)
				hc->setHighlight(HLevel::NORMAL);
	}
	else
	{
		for(auto hc : connectorControls)
		{
			if(hc)
			{
				if(hc->getIoType() == io_type && valid(hc->getDataType() & data_type))
					hc->setHighlight(HLevel::RIGHT);
				else
					hc->setHighlight(HLevel::WRONG);
			}
		}
	}
}




/////NODE CONNECTOR CONTROL/////
const GuiPropFlags NodeConnectorControl::PROP_FLAGS = PFlags::HARD_BACK;

NodeConnectorControl::NodeConnectorControl(NodeControl *parent_, GuiStateFlags s_flags, NCID nc_id)
	: GuiElement(parent_, APoint(), AVec(), GuiProps(s_flags, PROP_FLAGS)),
		Control(GuiProps(s_flags, PROP_FLAGS)),
		nc_parent(dynamic_cast<NodeControl*>(parent_)), ngc_parent(parent_->ngc_parent),
		id(nc_id), ioType(NodeConnector::getNC(nc_id)->ioType),
		dataType(NodeConnector::getNC(nc_id)->getDataType())
{
	setSize(getConnectorSize(ioType));

	APoint c_pos = nc_parent->getConnectorPoint(ioType) - nc_parent->getPos();
	AVec half_size = size.x*(1.0f/2.0f);
	
	//Set pos
	switch(ioType)
	{
	case IOType::DATA_INPUT:
		setPos(c_pos - AVec(0.0f, size.y*(1.0f/2.0f)));
		break;
	case IOType::DATA_OUTPUT:
		setPos(c_pos - AVec(size.x, size.y*(1.0f/2.0f)));
		break;

	case IOType::INFO_INPUT:
		setPos(c_pos - AVec(size.x*(1.0f/2.0f), 0.0f));
		break;
	case IOType::INFO_OUTPUT:
		setPos(c_pos - AVec(size.x*(1.0f/2.0f), size.y));
		break;

	default:
		setPos(c_pos - AVec(0.0f, size.y*(1.0f/2.0f)));
		break;
	}

	setHighlight(HLevel::NORMAL);
}

NodeConnectorControl::~NodeConnectorControl()
{

}

AVec NodeConnectorControl::getConnectorSize(IOType io_type)
{
	switch(io_type)
	{
	case IOType::DATA_INPUT:
		return AVec(15.0f, 40.0f);
	case IOType::DATA_OUTPUT:
		return AVec(15.0f, 40.0f);

	case IOType::INFO_INPUT:
		return AVec(50.0f, 15.0f);
	case IOType::INFO_OUTPUT:
		return AVec(50.0f, 15.0f);

	default:
		return AVec(30.0f, 30.0f);
	}
}

void NodeConnectorControl::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	//std::vector<NodeConnector*> connectors = nc_parent->getNode()->getConnectors();

	if(direct)
		ngc_parent->startConnect(this);
}

void NodeConnectorControl::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	NodeConnectionControl *movingConnection = ngc_parent->getMovingConnection();

	if(direct && movingConnection)
	{
		NodeConnector	*other_nc = movingConnection->getHangingNode(),
						*this_nc = NodeConnector::getNC(id);

		if(NodeConnector::validConnection(id, other_nc->getId()))
		{
			//Snap to connector location
			movingConnection->setHangingPos(nc_parent->getConnectorPoint(this_nc->ioType));
		}
		else
		{
			//Snap to (???)
			//movingConnection->setHangingNode(this_nc);
		}
	}
}

void NodeConnectorControl::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	if(direct)
		ngc_parent->finishConnect(this);
}

NCID NodeConnectorControl::getId() const
{
	return id;
}

IOType NodeConnectorControl::getIoType() const
{
	return ioType;
}

NodeDataType NodeConnectorControl::getDataType() const
{
	return dataType;
}


void NodeConnectorControl::setHighlight(HighlightLevel level)
{
	switch(level)
	{
	case HLevel::NORMAL:
		//BLUE-ISH
		setBgStateColor(Color(0.05, 0.1f, 0.5f, 1.0f), CS::NONE);
		setBgStateColor(Color(0.1, 0.2f, 0.65f, 1.0f), CS::HOVERING);
		setBgStateColor(Color(0.3, 0.3f, 0.8f, 1.0f), CS::CLICKING);
		setBgStateColor(Color(0.3, 0.3f, 0.8f, 1.0f), CS::DRAGGING);
		setBgStateColor(Color(0.3, 0.3f, 0.8f, 1.0f), CS::DROP_HOVERING);
		break;

	case HLevel::RIGHT:
		//GREEN-ISH
		setBgStateColor(Color(0.1f, 0.4f, 0.2f, 1.0f), CS::NONE);
		setBgStateColor(Color(0.2f, 0.55f, 0.3f, 1.0f), CS::HOVERING);
		setBgStateColor(Color(0.7f, 0.8f, 0.7f, 1.0f), CS::CLICKING);
		setBgStateColor(Color(0.7f, 0.8f, 0.7f, 1.0f), CS::DRAGGING);
		setBgStateColor(Color(0.7f, 0.8f, 0.7f, 1.0f), CS::DROP_HOVERING);
		break;

	case HLevel::WRONG:
		//RED-ISH
		setBgStateColor(Color(0.5f, 0.25f, 0.25f, 1.0f), CS::NONE);
		setBgStateColor(Color(0.75f, 0.5f, 0.5f, 1.0f), CS::HOVERING);
		setBgStateColor(Color(0.8f, 0.7f, 0.7f, 1.0f), CS::CLICKING);
		setBgStateColor(Color(0.8f, 0.7f, 0.7f, 1.0f), CS::DRAGGING);
		setBgStateColor(Color(0.8f, 0.7f, 0.7f, 1.0f), CS::DROP_HOVERING);
		break;
	}
}

void NodeConnectorControl::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);

		restoreViewport(gl);
	}
}