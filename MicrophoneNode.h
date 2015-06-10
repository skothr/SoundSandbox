#ifndef APOLLO_MICROPHONE_NODE_H
#define APOLLO_MICROPHONE_NODE_H

#include "Node.h"
#include "NodeBaseTypes.h"
#include "AudioDevice.h"
#include "AudioDataBuffer.h"
#include "ThreadedObject.h"

#include <unordered_set>

class NodeGraph;

class MicrophoneNode : public InputNode, public ThreadedObject
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

protected:
	static std::unordered_set<MicrophoneNode*> micNodes;
	static void renderMicFlush();

	AudioInDevice			device;
	bool deviceCallback(BufferDesc &data, double elapsed_time);

	AudioAmpDataBuffer		buffer;
	AudioAmpChunk			*activeChunk = nullptr;
	AudioVelChunk			*activeVelChunk = nullptr;

	AudioSample				last_sample = 0;

	bool					waitingForPull = false,
							stopThread = false;

	void shiftBuffer();

public:
	MicrophoneNode(NodeGraph *parent_graph, unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks);
	virtual ~MicrophoneNode();

	//Connector indices
	struct
	{ } INPUTS;
	struct
	{
		NCID AUDIO_ID = 0;
	} OUTPUTS;


	const AudioAmpData* getBuffer();
	//const AudioChunk* getRenderChunk();

	AudioInDevice* getDevice();

	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	//virtual bool flushData(FlushPacket &info) override;

	virtual void shutDownThread() override;
	
	void flushChunk();

	friend class SpeakerNode;
};

#endif	//APOLLO_MICROPHONE_NODE_H