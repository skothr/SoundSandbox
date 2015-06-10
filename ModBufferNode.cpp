#include "ModBufferNode.h"

#include "NodePackets.h"

/////MOD BUFFER NODE/////
const std::vector<NodeConnectorDesc> ModBufferNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Input audio to modify.", 1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Modified audio data.", 1),
			  NodeConnectorDesc(NodeData::MIDI, IOType::DATA_INPUT, "Midi Input", "Input midi to modify.", 1),
			  NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "Midi Output", "Modified midi data.", -1) };


ModBufferNode::ModBufferNode()
	: Node(nullptr, NType::INVALID, "Null Mod Buffer Node", "Unspecified"),
		BufferNode(0, ModData())
{
	initNode();
}

ModBufferNode::~ModBufferNode()
{

}

void ModBufferNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
	OUTPUTS.AUDIO_ID = ids[1];
	INPUTS.MIDI_ID = ids[2];
	OUTPUTS.MIDI_ID = ids[3];
}



/////STATIC MOD BUFFER NODE/////

StaticModBufferNode::StaticModBufferNode(NodeGraph *parent_graph, Time initial_length)
	: Node(parent_graph, NType::STATIC_MOD_BUFFER, "Static Mod Buffer Node", "Modulates an input signal based on the amplitude of the buffer."),
		BufferNode<Time, ModData>(initial_length, ModData())
{

}

StaticModBufferNode::~StaticModBufferNode()
{

}
	
bool StaticModBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	return false;
}

bool StaticModBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	return false;
}

/////DYNAMIC MOD BUFFER NODE/////

DynamicModBufferNode::DynamicModBufferNode(NodeGraph *parent_graph, Time initial_length)
	: Node(parent_graph, NType::DYNAMIC_MOD_BUFFER, "Dynamic Mod Buffer Node", "Modulates an input signal based on the amplitude of the buffer."),
		BufferNode<Time, ModData>(initial_length, ModData())
{

}
DynamicModBufferNode::~DynamicModBufferNode()
{

}
	
bool DynamicModBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	return false;
}

bool DynamicModBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	return false;
}