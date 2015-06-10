#include "WriteNode.h"

#include "NodePackets.h"
#include "NodeConnection.h"

#include "FileSystem.h"
#include "Path.h"

#include "AudioChunk.h"

#include <fstream>

/////WRITE NODE/////
const std::vector<NodeConnectorDesc>	WriteNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Audio data to write.", -1) };

WriteNode::WriteNode(NodeGraph *parent_graph)
	: Node(parent_graph, NType::WRITE, "Write Node", "Pulls data and writes it to a file."),
		cursor(DEFAULT_SAMPLE_RATE, AUDIO_CHUNK_SIZE, 1)
{
	initNode();
}

WriteNode::~WriteNode()
{

}

void WriteNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
}

Cursor* WriteNode::getCursor()
{
	return &cursor;
}

bool WriteNode::pullToFile(const Path &file_path)
{
	//Set up file
	std::ofstream out_file(file_path.getSystem(), std::ofstream::out);

	AudioVelChunk chunk(AUDIO_CHUNK_SIZE, 0);

	//Check input connections
	//TODO: Only pull from buffer
	AudioPullPacket packet(&cursor, &chunk, TransferMethod::COPY);
	
	return connectors[INPUTS.AUDIO_ID]->pullData(packet);
}

bool WriteNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;

	//Doesnt accept pushes

	return pushed;
}