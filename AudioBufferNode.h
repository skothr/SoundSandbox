#ifndef APOLLO_DYNAMIC_AUDIO_BUFFER_NODE_H
#define APOLLO_DYNAMIC_AUDIO_BUFFER_NODE_H

#include "BufferNode.h"
#include "StaticBufferNode.h"
#include "DynamicBufferNode.h"

#include "Audio.h"
#include "Timing.h"
#include "AudioChunk.h"

class MidiBufferNode;
class ProjectTrackDisplay;
class NodeGraph;

//Audio buffer base class
class AudioBufferNode : public virtual BufferNode<c_time, AudioVelData>
{
private:
	//typedef std::conditional<dynamic, DynamicBufferNode<c_time, AudioVelData>, StaticBufferNode<c_time, AudioVelData>>::type BaseClass;

	void initNode();

protected:
	static const std::vector<NodeConnectorDesc> nc_descs;

	std::vector<ChildNodeTree> childNodes;

	MidiBufferNode *midiNode = nullptr;

	int sampleRate = 0;
	
	virtual void onLengthChanged(c_time d_length) override;

	//Updates everything within the range, regardless of update status.
	//bool update(ChunkRange cr, TimeRange gr);

	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;

	ProjectTrackDisplay *display = nullptr;


public:
	AudioBufferNode(int sample_rate);
	virtual ~AudioBufferNode();

	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{
		NCID AUDIO_ID = 0;
	} OUTPUTS;
	
	MidiBufferNode* getMidiNode();
	
	std::vector<ChildNodeTree> getChildren();

	//bool updateRange(ChunkRange cr, TimeRange gr);

	//Updates the current chunk defined by cursor
	bool updateCurrent();	//Returns true unless something went wrong with updating a dirty chunk.

	
	//virtual void clearRange(ChunkRange r, bool compress) override;

	virtual void onUpdate(const Time &dt) override;

};


class StaticAudioBufferNode : public AudioBufferNode, public StaticBufferNode<c_time, AudioVelData>
{
protected:

public:
	StaticAudioBufferNode(NodeGraph *parent_graph, int sample_rate, c_time initial_length = 0);
	~StaticAudioBufferNode();

	
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

	virtual void update(const Time &dt) override;
};


class DynamicAudioBufferNode : public AudioBufferNode, public DynamicBufferNode<c_time, AudioVelData>
{
protected:

public:
	DynamicAudioBufferNode(NodeGraph *parent_graph, int sample_rate, c_time initial_length = 0);
	~DynamicAudioBufferNode();

	
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

	virtual void update(const Time &dt) override;
};


#endif	//APOLLO_DYNAMIC_AUDIO_BUFFER_NODE_H