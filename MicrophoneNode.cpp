#include "MicrophoneNode.h"

#include "NodePackets.h"
#include "NodeConnection.h"

/////MICROPHONE NODE/////
const std::vector<NodeConnectorDesc>	MicrophoneNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "The input audio data.", -1) };

std::unordered_set<MicrophoneNode*> MicrophoneNode::micNodes;

MicrophoneNode::MicrophoneNode(NodeGraph *parent_graph, unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks)
	: Node(parent_graph, NType::MICROPHONE, "Microphone Node", "Provides input audio from a specified audio device."),
		buffer(AUDIO_CHUNK_SIZE, buffer_chunks, true)
{
	micNodes.emplace(this);

	activeChunk = buffer.getActiveChunk();
	activeVelChunk = new AudioVelChunk(AUDIO_CHUNK_SIZE, 0);
	initNode();

	//initialize(numChannels, sampleRate);
	device.init(device_id, sample_rate, AUDIO_CHUNK_SIZE, 1, std::bind(&MicrophoneNode::deviceCallback, this, std::placeholders::_1, std::placeholders::_2));

	//Start playing
	threadStarted();
	device.start();
}

MicrophoneNode::~MicrophoneNode()
{
	shutDownThread();

	if(activeVelChunk)
		delete activeVelChunk;
	activeVelChunk = nullptr;

	micNodes.erase(this);
}

void MicrophoneNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	OUTPUTS.AUDIO_ID = ids[0];
}

void MicrophoneNode::renderMicFlush()
{
	for(auto mn : micNodes)
		mn->flushChunk();
}

void MicrophoneNode::shiftBuffer()
{
	activeChunk = buffer.shiftBuffer();
}

const AudioAmpData* MicrophoneNode::getBuffer()
{
	return &buffer;
}

AudioInDevice* MicrophoneNode::getDevice()
{
	return &device;
}

bool MicrophoneNode::deviceCallback(BufferDesc &data, double elapsed_time)
{
	bool status = true;

	while(waitingForPull && !stopThread);

	if(stopThread)
		return true;

	nodeLock.lockWait();
	shiftBuffer();

	//Copy the received data
	*activeChunk = data.data[0];

	std::vector<AudioVelSample> vel_data;
	vel_data.reserve(activeChunk->getSize());
	
	//AudioAmpChunk *lastChunk = buffer.getLastChunk();

	activeChunk->getVelData(vel_data, last_sample);
	activeVelChunk->setData(vel_data);
	activeVelChunk->setSeed(last_sample);
	
	//New last sample
	last_sample = (*activeChunk)[activeChunk->getSize() - 1];

	//Push the received data
	AudioPushPacket audio_data(activeVelChunk, TransferMethod::COPY);
	connectors[OUTPUTS.AUDIO_ID]->pushData(audio_data);

	waitingForPull = true;

	nodeLock.unlock();

	return status;
}


bool MicrophoneNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	/*
	if(this_id == OUTPUTS.AUDIO_ID)
	{
		AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

		if(audio_output)
		{
			nodeLock.lockWait();

			//Only copy active chunk
			switch(audio_output->method)
			{
			case TransferMethod::COPY:
				*audio_output->data = *activeVelChunk;
				break;
			case TransferMethod::ADD:
				*audio_output->data  += *activeVelChunk;
				break;
			case TransferMethod::DIRECT:
				audio_output->data = activeVelChunk;
				break;
			}
			nodeLock.unlock();

			pulled = true;
		}
	}
	*/
	return pulled;
}

void MicrophoneNode::flushChunk()
{
	nodeLock.lockWait();

	if(waitingForPull)
	{
		AudioPushPacket audio_output(activeVelChunk);//, lastActiveChunk, chunkStart);
		connectors[OUTPUTS.AUDIO_ID]->pushData(audio_output);

		waitingForPull = false;
	}

	nodeLock.unlock();
}

void MicrophoneNode::shutDownThread()
{
	if(threadRunning())
	{
		stopThread = true;

		//nodeLock.lockWait();
		device.stop();
		//nodeLock.unlock();
	}
}

/*
bool MicrophoneNode::flushData(FlushPacket &info)
{
	bool flushed = false;
	
	//nodeLock.lockWait();

	////Push active chunk
	//AudioPushPacket audio_output(&activeVelChunk, info.targetChunkRange, TransferMethod::COPY);
	//flushed = connectors[OUTPUTS.AUDIO_ID]->pushData(audio_output);

	//nodeLock.unlock();
	
	return flushed;
}
*/