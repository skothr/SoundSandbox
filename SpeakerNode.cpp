#include "SpeakerNode.h"

#include "ProjectTrackDisplay.h"
#include "AUtility.h"

#include "Cursor.h"
#include "NodePackets.h"
#include "NodeConnection.h"
#include "MidiDeviceNode.h"
#include "MicrophoneNode.h"

#include "RenderNode.h"
#include "NodeGraph.h"

/////SPEAKER NODE/////

const std::vector<NodeConnectorDesc>	SpeakerNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Audio data to play.", -1) };

SpeakerNode::SpeakerNode(NodeGraph *parent_graph, unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks, ProjectTrackDisplay *track_display)
	: Node(parent_graph, NType::SPEAKER, "Speaker Node", "Sends audio to a speaker to be played"),
		sampleRate(sample_rate), buffer(AUDIO_CHUNK_SIZE, buffer_chunks, true), pushChunk(AUDIO_CHUNK_SIZE, 0),
		cursor(sample_rate, AUDIO_CHUNK_SIZE, 1)
{
	//initialize(numChannels, sampleRate);
	device.init(device_id, sampleRate, AUDIO_CHUNK_SIZE, 1, std::bind(&SpeakerNode::deviceCallback, this, std::placeholders::_1, std::placeholders::_2));
	initNode();
	
	initBuffers();

	cursor.setActive(true);
	//Start playing
	threadStarted();
	device.start();
}

/*
SpeakerNode::SpeakerNode(const SpeakerNDesc &sn_desc)
	: Node(*(NDesc*)&sn_desc),
		device(sn_desc.device), sampleRate(sn_desc.sampleRate),
		numChannels(sn_desc.numChannels), buffer(AUDIO_CHUNK_SIZE, sn_desc.bufferSize, true), pushBuffer(AUDIO_CHUNK_SIZE, 2, false)
{
	Node::init(sn_desc.connectors);
	INPUTS.AUDIO_ID = sn_desc.connectors[0];

	initBuffers();
	
	//Start playing
	device.play();
}
*/

SpeakerNode::~SpeakerNode()
{
	shutDownThread();
	AU::safeDelete(activeVelChunk);
}

void SpeakerNode::initBuffers()
{
	activeChunk = buffer.getActiveChunk();
	trailingChunk = buffer[0];
	activeVelChunk = new AudioVelChunk(AUDIO_CHUNK_SIZE, 0);


	//Initialize buffer
	//buffer.resize(bufferSize);
	//for(auto &c : buffer)
	//	c = new AudioChunk(AUDIO_CHUNK_SIZE);

	//renderChunk = new AudioChunk(AUDIO_CHUNK_SIZE);
	//trailingChunk = new AudioChunk(AUDIO_CHUNK_SIZE);

	//Connect buffer in a doubly-linked list (split between renderChunk and trailingChunk)
	//renderChunk->setPointers(buffer[0], nullptr);
	//trailingChunk->setPointers(nullptr, buffer[bufferSize - 1]);
	//for(unsigned int i = 0; i < bufferSize; i++)
	//{
	//	buffer[i]->setPointers(		(i < bufferSize - 1 ? buffer[i + 1] : trailingChunk),
	//								(i > 0 ? buffer[i - 1] : renderChunk));
	//}
	
	//AudioVelChunk to render to
	//activeVelChunk = new AudioVelChunk(AUDIO_CHUNK_SIZE);
	//Connect to sample place in the loop as renderChunk
	//activeVelChunk->setPointers(buffer[0], nullptr);

	//Initialize pushBuffer with 2 chunks
	//pushBuffer.resize(2);
	//pushBuffer[0] = new AudioVelChunk(AUDIO_CHUNK_SIZE, 0);
	//pushBuffer[1] = new AudioVelChunk(AUDIO_CHUNK_SIZE, 0);

	//pushBuffer[0]->setPointers(nullptr, pushBuffer[1]);
	//pushBuffer[1]->setPointers(pushBuffer[0], nullptr);

	//lastActiveChunk = buffer[0];
}

void SpeakerNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
}

void SpeakerNode::onUpdate(const Time &dt)
{
	/*
	//Make sure it keeps playing
	if(!device.isActive())
	{
		std::cout << "SPEAKER STOPPED PLAYING\n";
		device.start();
	}
	*/
}

void SpeakerNode::shiftBuffer()
{
	buffer.shiftBuffer();
	
	activeChunk = buffer.getActiveChunk();
	trailingChunk = buffer[0];//buffer.getNumChunks() - 1);


	////Rotate buffer so last chunk is first, and the rest are shifted down
	//std::rotate(buffer.begin(), buffer.end() - 1, buffer.end());

	////AudioChunk *temp = renderChunk;
	//
	////trailingChunk is the new renderChunk
	//std::swap(renderChunk, trailingChunk);
	////Chunk that was last in buffer is the new trailingChunk,
	////	and new first element of buffer is the previous activeChunk
	//std::swap(trailingChunk, buffer[0]);

	////Adjust prev/next pointers
	//renderChunk->setPointers(buffer[0], nullptr);
	//buffer[0]->setPointers(buffer[1], renderChunk);
	//trailingChunk->setPointers(nullptr, buffer[bufferSize - 1]);
	//buffer[buffer.size() - 1]->setPointers(trailingChunk, buffer[buffer.size() - 2]);

	//*activeVelChunk = renderChunk->getVelChunk();
	//activeVelChunk->setPointers(buffer[0], nullptr);

	////activeVelChunk->updateAll();

	//lastActiveChunk = buffer[0];
	//lastActiveChunk->NOTE_ON = false;
}

/*
void SpeakerNode::shiftPushBuffer()
{
	pushBuffer[pushBuffer.getNumChunks() - 1]->setStatus(DataStatus::DIRTY);
	numPushedChunks--;

	pushBuffer.shiftBuffer();

	////First chunk is now invalid, so set to overwrite data
	//pushBuffer[0]->setStatus(DataStatus::DIRTY);
	////One less pushed chunk
	//numPushedChunks--;

	////Rotate buffer so last chunk is first, and the rest are shifted down
	////std::rotate(pushBuffer.begin(), pushBuffer.end() - 1, pushBuffer.end());
}
*/

bool SpeakerNode::getData(BufferDesc &data)
{
	nodeLock.lockWait();

	NodeConnector *nc = connectors[INPUTS.AUDIO_ID].get();
	
	//const Time	chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;

	//Range to flush
	//ChunkRange	flush_cr(chunkNumber - 1, chunkNumber),
	//			target_cr(flush_cr.end, flush_cr.end + 1);

	//TimeRange	flush_tr(flush_cr.start*chunk_time, flush_cr.end*chunk_time),
	//			target_tr(target_cr.start*chunk_time, target_cr.end*chunk_time),

				//flush_global_tr(clock.globalOffset + clock.last_t, clock.globalOffset + clock.t);
	//			flush_global_tr(g_time - clock.dt, g_time);
	
	//FlushPacket flush_info(flush_tr, target_tr, flush_cr, target_cr, flush_global_tr);

	//Only flush after first chunk
	//if(chunkNumber > 0)
	//{
		//Flush data that has accumulated since last render
	//	bool flushed = nc->flushData(flush_info);
	//}
	
	bool add = false;	//First copy, then add after (since activeVelChunk contains data from last trailingChunk)

	//Add pulled data (only if trackDisp is playing)
	//if(trackDisp && trackDisp->isPlaying())
	//{
		//target_cr = trackDisp->getCursor()->getChunkRange();
		//target_tr = trackDisp->getCursor()->getTimeRange();

		//AudioPullPacket audio_data(&activeVelChunk, target_cr, (add ? TransferMethod::ADD : TransferMethod::COPY), requestUpdate);
		//AudioPullPacket audio_data(&activeVelChunk, target_cr, flush_global_tr, (add ? TransferMethod::ADD : TransferMethod::COPY), requestUpdate);
		//audio_data.tRange = target_tr;

	AudioPullPacket audio_packet(nullptr, activeVelChunk, TransferMethod::COPY);

	add = nc->pullData(audio_packet);

	RenderNode::renderNodeFlush();
	MicrophoneNode::renderMicFlush();

		//trackDisp->getCursor()->step();
		//trackDisp->updateCursorPos();
	//}

	//activeVelChunk->updateChunkStep();

	//std::unordered_map<NCID, NCOwnedPtr> conn = nc->getConnections();
	//for(auto c : conn)
	//{
	//	AudioPullPacket audio_data(&activeVelChunk, target_cr, (add ? TransferMethod::ADD : TransferMethod::COPY), requestUpdate);
	//	audio_data.globalRange = target_cr;
	//	audio_data.tRange = target_tr;
	//	bool pulled = nc->pullData(audio_data, c.first);

	//	if(pulled)
	//	{
	//		/*
	//		switch(status.getWarningCode())
	//		{
	//		case AS::WType::NO_ACTION_TAKEN:
	//			//std::cout << "SPEAKER " << status << "\n";
	//			//Don't set add to true
	//			break;

	//		case AS::WType::PARAMETERS_CHANGED:
	//			std::cout << "SPEAKER " << status << "\n";
	//			//Still set add to true

	//		default:
	//			add = true;
	//		}
	//		*/

	//		//add |= (status.getWarningCode() != AS::WType::NO_ACTION_TAKEN);
	//	}
	//	else
	//	{
	//		std::cout << "SPEAKER PULL " << pulled << "\n";
	//		return AStatus(AS::A_WARNING, "No Pull.");
	//	}
	//}
	
	//Copy pushed data (if there is any)
	if(pushChunk.getStatus() == DataStatus::CLEAN)
	{
		//AudioVelChunk *pb_chunk = pushBuffer[pushBuffer.getNumChunks() - 1];

		add ? (*activeVelChunk += pushChunk) : (*activeVelChunk = pushChunk);
		add = true;
		pushChunk.setStatus(DataStatus::DIRTY);

	//	shiftPushBuffer();
	}

	//Clear active chunk (to zeros) if nothing copied over it
	//if(audio_data.method == TransferMethod::COPY)
	
	activeChunk->loadZeros();
	
	if(add)
	{
		//activeVelChunk->updateAll();
		//activeVelChunk->setPointers(renderChunk->prevChunk, renderChunk->nextChunk);
		//sAmplitude = activeVelChunk->get;
		//activeVelChunk->setSeed(buffer[1]->getLastSample());
		//activeVelChunk->updateInside();

		//Calculate data from velocities, and clamp to +/- max amplitude
		//for(unsigned int i = 0; i < renderChunk->getSize(); i++)
		//{
		//	sAmplitude += activeVelChunk->getSample(i);
		//
		//	sAmplitude = max((AudioSample)-AUDIO_MAX_AMPLITUDE, min(sAmplitude, (AudioSample)AUDIO_MAX_AMPLITUDE));//std::max((double)-AUDIO_MAX_AMPLITUDE, std::min(sAmplitude, (double)AUDIO_MAX_AMPLITUDE));
		//
		//	renderChunk->setSample(i, sAmplitude);
		//}

		//activeVelChunk->updateChunkStep();

		*activeChunk += *activeVelChunk;
		activeChunk->offset(activeVelChunk->getSeed());

		activeChunk->clamp((AudioSample)-AUDIO_MAX_AMPLITUDE, (AudioSample)AUDIO_MAX_AMPLITUDE);
	}
	//else
	//{
		//activeChunk->loadZeros();
		//sAmplitude = 0;
	//}

	//static AudioSample *buf = new AudioSample[AUDIO_CHUNK_SIZE];

	//for(int i = 0; i < AUDIO_CHUNK_SIZE; i++)
	//	buf[i] = (AudioSample)ceil(renderChunk->data[i]);

	data.numChunks = 1;
	data.data[0] = *activeChunk;
	
	shiftBuffer();
	
	nodeLock.unlock();
	return true;
}

//Called when speaker needs more audio data
bool SpeakerNode::deviceCallback(BufferDesc &data, double elapsed_time)
{
	if(stopThread)
		return true;

	bool status = true;
	/*
	const double chunk_time = (double)pushChunk.getSize()/(double)sampleRate;
	
	if(!clock.initialized)
	{
		//Initialize clock -- starts from 0 right now
		clock.reset(true);

		Node::globalRange = TimeRange(clock.last_t, clock.t);

		//lambda = 2.0*chunk_time;

		//Load zeros at first -- no data yet
		for(int c = 0; c < numChannels; c++)
			data.data[c].loadZeros();

		//Wait for first-chunk events to happen (1 chunk length) before doing anything
		//clock.waitUntil(chunk_time);

		//Now current clock.t is the length of a chunk.
		//clock.nextTimeFrame();
		
		//NodeConnection::resetAllConnectionStates();
		//MidiDeviceNode::holdEvents();
		return status;
	}
	else
	{
		//Length of chunk to be rendered (seconds)
		const double	chunk_time = (double)pushChunk.getSize()/(double)sampleRate;

		//clock.last_t is start of last chunk's events, and clock.t is the end of the last chunk's events.
		//	

		//Need to wait until event input is done for the previous chunk (exactly 1 chunk length)
		//Time		next_t = clock.t + chunk_time;

		//Wait
		//clock.waitUntil(next_t);
		//Update to next time --> now last_t is beginning of last chunk's events and t is beginning of this chunks events
		//clock.nextTimeFrame();
		
	}
		*/
	//MidiDeviceNode::holdEvents();

	Node::updateGlobalRange((Time)cursor.chunkSize/(Time)sampleRate);
	parentGraph->resetConnectionStates();

	static bool cursor_step = true;
	if(cursor_step)
	{
		cursor.step();
		cursor_step = false;
	}

	if(TIME_SPEAKER)
	{
		static TimeTest timer(TimeUnits::MICROSECONDS, "SPEAKER CHUNK TIMING");
		static Time result = 0.0;
		status = timer.timeFunction<bool>(std::bind(&SpeakerNode::getData, this, data), 256, result);
	}
	else
		status = getData(data);

	//MidiDeviceNode::releaseEvents();

	return status;
}

Cursor* SpeakerNode::getCursor()
{
	return &cursor;
}

const AudioAmpData* SpeakerNode::getBuffer()
{
	//nodeLock.waitForUnlocked();
	return &buffer;
}

const AudioChunk* SpeakerNode::getRenderChunk()
{
	//nodeLock.waitForUnlocked();
	return activeChunk;
}

const AudioChunk* SpeakerNode::getTrailingChunk()
{
	//nodeLock.waitForUnlocked();
	return trailingChunk;
}

AudioOutDevice* SpeakerNode::getDevice()
{
	return &device;
}

/*
void SpeakerNode::setTrackDisplay(ProjectTrackDisplay *track_disp)
{
	trackDisp = track_disp;
}
*/

/*
void SpeakerNode::onConnect(NodeConnector *nc)
{
	//NodeConnector *nc_in = getConnector(INPUTS.AUDIO_ID);
	//if(nc->hasTimeline())
	//	pullConnections.push_back(nc_in->getIndex(nc));
}

void SpeakerNode::onDisconnect(NodeConnector *nc)
{
	//pullConnections.clear();

	//NodeConnector *nc_in = getConnector(INPUTS.AUDIO_ID);
	//std::vector<NodeConnector*> connections = nc_in->getConnections();

	//for(auto nc : connections)
	//	pullConnections.push_back(nc_in->getIndex(nc));
}
*/

//bool SpeakerNode::canPush()
//{
//	return true;
//}

bool SpeakerNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;

	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);
		
		if(audio_input)
		{
			nodeLock.lockWait();

			//Copy chunks directly
			if(pushChunk.getStatus() == DataStatus::DIRTY)
			{
				//AudioSample seed = pushChunk.getSeed() + pushChunk.getChunkStep();
				pushChunk = *audio_input->data;
				//pushChunk.seed -= seed;
				//pushChunk.seed = audio_input->data->seed;
				pushChunk.setStatus(DataStatus::CLEAN);
			}
			else
			{
				pushChunk += *audio_input->data;
				//pushChunk.seed += audio_input->data->seed;
			}
			pushChunk.updateChunkStep();

			nodeLock.unlock();

			pushed = true;
		}
	}

	return pushed;
}

void SpeakerNode::shutDownThread()
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
void SpeakerNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new MidiDeviceNDesc()));
	SpeakerNDesc *desc = dynamic_cast<SpeakerNDesc*>(objDesc);
	
	desc->device = *(AudioDevDesc*)device.getDesc();

	desc->bufferSize = buffer.getNumChunks();
	desc->numChannels = numChannels;
	desc->sampleRate = sampleRate;

	desc->requestUpdate = requestUpdate;
}
*/