#ifndef APOLLO_MOD_BUFFER_NODE_H
#define APOLLO_MOD_BUFFER_NODE_H

#include "BufferNode.h"
#include "StaticBufferNode.h"
#include "DynamicBufferNode.h"
#include "Timing.h"
#include "ModData.h"

class PullPacket;
class PushPacket;
class FlushPacket;
class NodeGraph;

class ModBufferNode : public virtual BufferNode<Time, ModData>
{
protected:
	void initNode();

public:
	static const std::vector<NodeConnectorDesc> nc_descs;

	ModBufferNode();
	virtual ~ModBufferNode();
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
		NCID MIDI_ID = 0;
	} INPUTS;
	struct
	{
		NCID AUDIO_ID = 0;
		NCID MIDI_ID = 0;
	} OUTPUTS;
};


class StaticModBufferNode : public ModBufferNode, public StaticBufferNode<Time, ModData>
{
protected:

public:
	StaticModBufferNode(NodeGraph *parent_graph, Time initial_length = 0);
	virtual ~StaticModBufferNode();
	
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;
};


class DynamicModBufferNode : public ModBufferNode, public DynamicBufferNode<Time, ModData>
{
protected:

public:
	DynamicModBufferNode(NodeGraph *parent_graph, Time initial_length = 0);
	virtual ~DynamicModBufferNode();
	
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;
};

#endif	//APOLLO_MOD_BUFFER_NODE_H