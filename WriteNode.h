#ifndef APOLLO_WRITE_NODE_H
#define APOLLO_WRITE_NODE_H

#include "NodeBaseTypes.h"
#include "Cursor.h"

class NodeGraph;
class Path;

class WriteNode : public OutputNode
{
protected:

	Cursor		cursor;

	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();



public:
	WriteNode(NodeGraph *parent_graph);
	virtual ~WriteNode();
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{ } OUTPUTS;

	virtual Cursor* getCursor() override;

	bool pullToFile(const Path &file_path);	//TODO: Format, etc

	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

};

#endif	//APOLLO_WRITE_NODE_H