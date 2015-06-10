#include "AudioBufferNode.h"

#include "Node.h"
#include "NodePackets.h"
#include "NodeConnection.h"
#include "MidiBufferNode.h"
#include "NodeGraph.h"

#include "AudioData.h"

/////AUDIO_BUFFER_NODE/////
const std::vector<NodeConnectorDesc> AudioBufferNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Input audio to buffer.", 1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Buffered audio data.", -1) };

 
AudioBufferNode::AudioBufferNode(int sample_rate)
	: Node(nullptr, NType::INVALID, "Null Audio Buffer Node", "Unspecified"),
		BufferNode<c_time, AudioVelData>(1, AudioVelData(AUDIO_CHUNK_SIZE, 1, 0)),
		sampleRate(sample_rate)
{
	initNode();
}

 
AudioBufferNode::~AudioBufferNode()
{ }

 
void AudioBufferNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
	OUTPUTS.AUDIO_ID = ids[1];
}

 
void AudioBufferNode::onLengthChanged(c_time d_length)
{
	nodeLock.lockWait();

	c_time old_length = length - d_length;

	data.resize(length);

	nodeLock.unlock();
}


 /*
bool AudioBufferNode::update(ChunkRange cr, TimeRange gr)
{
	bool updated = true;

	//const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;

	nodeLock.lockWait();

	AudioSample seed = 0;

	if(cr.start > 0)
	{
		data(cr.start - 1).updateChunkStep();
		seed = data(cr.start - 1).getSeed() + data(cr.start - 1).getChunkStep();
	}

	for(c_time c = cr.start; c < cr.end; c++)
	{
		AudioPullPacket packet(&cursor, data[c], TransferMethod::COPY);
	
		//Update chunks
		updated = connectors[INPUTS.AUDIO_ID]->pullData(packet);
	
		//Clean chunks
		if(updated)
		{
			data[c]->setStatus(DataStatus::CLEAN);
			data[c]->setSeed(seed);
			data[c]->updateChunkStep();
		}

		seed = data[c]->getSeed() + data[c]->getChunkStep();
	}

	//if(display)
	//	display->getCursor()->setTimeChunks(display->getCursor()->getChunkRange().end);
	
	nodeLock.unlock();

	return updated;
}


 
bool AudioBufferNode::updateRange(ChunkRange cr, TimeRange gr)
{
	bool status = true;

	//Update each chain of dirty chunks together
	ChunkRange u_cr(-1, -1);
	for(c_time c = cr.start; c < cr.end; c++)
	{
		bool continue_chain = data[c]->getStatus() == DataStatus::DIRTY;
		if(continue_chain)
		{
			//Continue chain
			u_cr.start = (u_cr.start < 0 ? c : u_cr.start);
			u_cr.end = c + 1;
		}
		else

		if((u_cr.start >= 0 && u_cr.end >= 0) && (!continue_chain || (c + 1 == cr.end && continue_chain)))
		{
			//Update current chain
			status = update(u_cr, gr);
			if(!status)
				break;

			//Reset chain
			u_cr.start = -1;
			u_cr.end = -1;
		}
	}

	return status;
}
*/

bool AudioBufferNode::updateCurrent()
{
	bool status = true;

	if(cursor.getChunkRange().start <= data.getChunkSize())
	{
		AudioVelChunk *chunk = data[cursor.getChunkRange().start];
	
		if(chunk->getStatus() == DataStatus::DIRTY)
		{
			AudioPullPacket packet(&cursor, chunk, TransferMethod::COPY);
	
			status = connectors[INPUTS.AUDIO_ID]->pullData(packet);

			if(status)
				chunk->setStatus(DataStatus::CLEAN);
		}
	}
	else
		status = false;

	return status;
}

 
void AudioBufferNode::onConnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.AUDIO_ID)
	{
		//Update corresponding midi node
		int dist;
		//Node *n = getClosestNode(NType::MIDI_TRACK, NodeData::AUDIO | NodeData::MIDI, dist, true, -1);
		//midiNode = dynamic_cast<MidiTrackNode*>(n);

		//Match length
		//if(midiNode)
		//	setLength((c_time)ceil(midiNode->getLength()*(double)sampleRate/(double)AUDIO_CHUNK_SIZE));
	}
}

 
void AudioBufferNode::onDisconnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.AUDIO_ID)
	{
		//Update corresponding midi node
		int dist;
		//Node *n = getClosestNode(NType::MIDI_TRACK, NodeData::AUDIO | NodeData::MIDI, dist, true, -1);
		//midiNode = dynamic_cast<MidiTrackNode*>(n);

		//Match length
		//if(midiNode)
		//	setLength((c_time)ceil(midiNode->getLength()*(double)sampleRate/(double)AUDIO_CHUNK_SIZE));
	}
}

 
MidiBufferNode* AudioBufferNode::getMidiNode()
{
	return midiNode;
}

 
std::vector<ChildNodeTree> AudioBufferNode::getChildren()
{
	childNodes.clear();
	getChildNodes({ NType::STATIC_AUDIO_BUFFER, NType::STATIC_MIDI_BUFFER, NType::DYNAMIC_AUDIO_BUFFER, NType::DYNAMIC_MIDI_BUFFER }, childNodes);
	return childNodes;
}

// 
//void AudioBufferNode::clearRange(ChunkRange r, bool compress)
//{
//	//TODO
//}


 
void AudioBufferNode::onUpdate(const Time &dt)
{
	//Update length
	if(midiNode)
		setLength((c_time)ceil(midiNode->getLength()*(double)sampleRate/(double)AUDIO_CHUNK_SIZE));
}

 
//void AudioBufferNode::update(const Time &dt)
//{
//
//}



/////STATIC AUDIO BUFFER NODE/////

StaticAudioBufferNode::StaticAudioBufferNode(NodeGraph *parent_graph, int sample_rate, c_time initial_length)
	: Node(parent_graph, NType::STATIC_AUDIO_BUFFER, "Static Audio Buffer Node", "A buffer of audio that can be recorded on to."),
		BufferNode<c_time, AudioVelData>(initial_length, AudioVelData(AUDIO_CHUNK_SIZE, initial_length, 0)),
		AudioBufferNode(sample_rate)
{ }

StaticAudioBufferNode::~StaticAudioBufferNode()
{ }

bool StaticAudioBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;

	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

		if(audio_output)
		{
			Cursor *pull_cursor = audio_output->pCursor;

			//Update this cursor to match parent
			if(pull_cursor)
				cursor.setTimeSamples(pull_cursor->getSampleRange().start);

			//Only update range if recording
			if(recording && !updateCurrent())//updateRange(globalCursor.getChunkRange(), globalCursor.getTimeRange()))
				return false;

			c_time c = cursor.getChunkRange().start;

			nodeLock.lockWait();

			//Pass data
			switch(audio_output->method)
			{
			case TransferMethod::COPY:
				*audio_output->data = data(c);
				break;

			case TransferMethod::ADD:
				//Add data to output pointer
				*audio_output->data += data(c);
				break;

			case TransferMethod::DIRECT:
				//Return pointer to buffer data
				//	WARNING: READ ONLY, DO NOT OVERWRITE!! BUFFER WILL BE OVERWRITTEN.
				audio_output->data = data[c];
				break;
			}

			pulled = true;
			
			cursor.step();

			nodeLock.unlock();
		}
	}

	return pulled;
}

 
bool StaticAudioBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);

		if(audio_input)
		{
			if(recording)
			{
				ChunkRange cr = cursor.getChunkRange();//pRecordCursor->getChunkRange();//(start_chunk, start_chunk + num_chunks);

				//Clamp range to be within data range
				//cr.start = std::max(cr.start,  (c_time)0);
				//cr.end = std::min(cr.end, length);
	
				//Check range
				//if(cr.length() <= 0)
				//	return false;//AStatus(AS::A_ERROR, "Requested range is outside of data");
				
				//Adjust track length
				//TODO: Make option for stopping recording if it goes out of range
				//if(cr.end > maxLength)
				//	setLength(cr.end);

				//Update range if requested
				//if(!statusGood(status = updateRange(r)))
				//	return status;
				
				nodeLock.lockWait();

				//Copy data
				data(cr.start) = *audio_input->data;

				nodeLock.unlock();

				//Set data clean
				//clean(cr);
			}
			//else
				//Only accept pushed audio data if this track is recording
				//status.setWarning(AS::WType::NO_ACTION_TAKEN, "This AUDIO TRACK NODE is not recording, and can't accept a data push.");
		
			//Propogate push
			//connectors[OUTPUTS.AUDIO_ID]->pushData(*audio_input);
		
		}
	}

	return pushed;
}

void StaticAudioBufferNode::update(const Time &dt)
{

}

/////DYNAMIC AUDIO BUFFER NODE/////

DynamicAudioBufferNode::DynamicAudioBufferNode(NodeGraph *parent_graph, int sample_rate, c_time initial_length)
	: Node(parent_graph, NType::DYNAMIC_AUDIO_BUFFER, "Dynamic Audio Buffer Node", "A buffer of audio that automatically updates."),
		BufferNode<c_time, AudioVelData>(initial_length, AudioVelData(AUDIO_CHUNK_SIZE, initial_length, 0)),
		AudioBufferNode(sample_rate)
{ }

DynamicAudioBufferNode::~DynamicAudioBufferNode()
{ }

	
bool DynamicAudioBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;

	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

		if(audio_output)
		{
			Cursor *pull_cursor = audio_output->pCursor;
			if(pull_cursor)
				cursor.setTimeSamples(pull_cursor->getSampleRange().start);

			if(cursor.isActive())
			{
				//Update range if requested
				if(!updateCurrent())//updateRange(globalCursor.getChunkRange(), globalCursor.getTimeRange()))
					return false;
				
				c_time c = cursor.getChunkRange().start;

				//Pass data
				nodeLock.lockWait();
				switch(audio_output->method)
				{
				case TransferMethod::COPY:
					*audio_output->data = data(c);
					break;

				case TransferMethod::ADD:
					//Add data to output pointer
					*audio_output->data += data(c);
					break;

				case TransferMethod::DIRECT:
					//Return pointer to buffer data
					//	WARNING: READ ONLY, DO NOT OVERWRITE!! BUFFER WILL BE OVERWRITTEN.
					audio_output->data = data[c];
					break;
				}
				nodeLock.unlock();

				pulled = true;
				cursor.step();
			}
		}
	}

	return pulled;
}

 
bool DynamicAudioBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);

		if(audio_input && cursor.isActive())
		{
			//ChunkRange cr = globalCursor.getChunkRange();
			c_time c = cursor.getChunkRange().start;
				
			//Adjust track length
			//TODO: Make option for stopping recording if it goes out of range
			//if(cr.end > maxLength)
			//	setLength(cr.end);

			//Update range if requested
			//if(!statusGood(status = updateRange(r)))
			//	return status;
				
			nodeLock.lockWait();

			//Copy data
			data(c) = *audio_input->data;

			nodeLock.unlock();

			//Set data clean
			//clean(cr);

			//Propogate push
			//connectors[OUTPUTS.AUDIO_ID]->pushData(*audio_input);
		
		}
	}

	return pushed;
}


void DynamicAudioBufferNode::update(const Time &dt)
{

}