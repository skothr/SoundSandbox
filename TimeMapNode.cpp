#include "TimeMapNode.h"

#include "TrackNodes.h"
#include "NodeConnection.h"
#include "NodePackets.h"


/////TIME MAP NODE/////
const std::vector<NodeConnectorDesc> TimeMapNode::nc_descs =
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Input audio to map.", -1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Mapped audio data.", -1) };

TimeMapNode::TimeMapNode(NodeGraph *parent_graph, int sample_rate)
	: Node(parent_graph, NType::TIME_MAP, "Time Map Node", "Maps tracks to global time."),
		sampleRate(sample_rate), cursor(sample_rate, AUDIO_CHUNK_SIZE, 1)
{
	initNode();
}

/*
TimeMapNode::TimeMapNode(const TimeMapNDesc &tmn_desc)
	: Node(*(NDesc*)&tmn_desc),
		sampleRate(tmn_desc.sampleRate), maxBufferLength(tmn_desc.maxBufferLength),
		cursor(tmn_desc.cursor)
{
	Node::init(tmn_desc.connectors);

	INPUTS.AUDIO_ID	= tmn_desc.connectors[0];
	OUTPUTS.AUDIO_ID = tmn_desc.connectors[1];
}
*/

void TimeMapNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID		= ids[0];
	OUTPUTS.AUDIO_ID	= ids[1];
}

Cursor* TimeMapNode::getCursor()
{
	return &cursor;
}

//bool TimeMapNode::hasCursor() const
//{
//	return true;
//}
//
//bool TimeMapNode::isPlaying()
//{
//	nodeLock.lockWait();
//	bool is_playing = playing;
//	nodeLock.unlock();
//
//	return is_playing;
//}

void TimeMapNode::setPlaying(bool play)
{
	nodeLock.lockWait();
	playing = play;

	//etc... (?)

	nodeLock.unlock();
}

void TimeMapNode::reset()
{
	nodeLock.lockWait();
	cursor.setTimeSamples(0);
	nodeLock.unlock();
}

std::vector<AudioMidiPair>* TimeMapNode::getAddedBuffers()
{
	return &addedBuffers;
}
std::vector<AudioMidiPair>* TimeMapNode::getRemovedBuffers()
{
	return &removedBuffers;
}
const std::unordered_map<NCID, AudioMidiPair>* TimeMapNode::getConnectedBuffers()
{
	return &connectedBuffers;
}

//TODO: Update connected buffers even if connections dont change
//		(if buffer is connected/disconnected from a different node in the connection chain)

void TimeMapNode::onConnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.AUDIO_ID)
	{
		nodeLock.lockWait();

		/*
		//See if there are any AudioTrackNodes down this connection chain
		int dist;
		Node *n = getClosestNode(NType::AUDIO_TRACK, NodeData::AUDIO, dist, true, -1);
		
		AudioMidiPair p;

		//Get pointer(s) to buffer(s) if any AudioTrackNodes were found
		if(n)
		{
			AudioTrackNode *ab_node = dynamic_cast<AudioTrackNode*>(n);
			
			//Update maxBuffLength is this buffer is longer than the current max
			c_time ab_length = ab_node->getLength();
			maxBufferLength += (ab_length - maxBufferLength)*(ab_length > maxBufferLength);

			//Make AudioMidiPair with the node and its corresponding MidiTrackNode (if there is one)
			p = AudioMidiPair(ab_node, ab_node->getMidiNode());
			addedBuffers.push_back(p);
		}
		
		connectedBuffers.emplace(other_nc, p);
		*/
		nodeLock.unlock();
	}
}

void TimeMapNode::onDisconnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.AUDIO_ID)
	{
		nodeLock.lockWait();

		AudioMidiPair p = connectedBuffers[other_nc];

		//Remove this connection's AudioMidiPair
		connectedBuffers.erase(other_nc);

		//Add to removedBuffers if there was a buffer
		if(p.audio)
			removedBuffers.push_back(p);

		//Update maxBufferLength if this buffer was the longest
		if(p.audio->getLength() == maxBufferLength)
		{
			maxBufferLength = 0;
			for(auto bp : connectedBuffers)
			{
				c_time ab_length = bp.second.audio->getLength();
				maxBufferLength += (ab_length - maxBufferLength)*(ab_length > maxBufferLength);
			}
		}
		
		nodeLock.unlock();
	}
}

c_time TimeMapNode::getMaxBufferLength() const
{
	return maxBufferLength;
}


//bool TimeMapNode::canPull()
//{
//	if(isPlaying())
//	{
//		bool can_pull = false;
//		NodeConnector *nc = connectors[INPUTS.AUDIO_ID];
//	
//		nc->connectorLock.lockWait();
//		for(NodeConnector::CIndex i = 0; i < (NodeConnector::CIndex)nc->numConnections(); i++)
//		{
//			if(can_pull |= nc->canPull(i))
//				break;
//		}
//		nc->connectorLock.unlock();
//
//		return can_pull;
//	}
//	else
//		return false;
//}
//
//bool TimeMapNode::canPush()
//{
//	bool can_push = false;
//	NodeConnector *nc = connectors[OUTPUTS.AUDIO_ID];
//	
//	nc->connectorLock.lockWait();
//	for(NodeConnector::CIndex i = 0; i < (NodeConnector::CIndex)nc->numConnections(); i++)
//	{
//		if(can_push |= nc->canPush(i))
//			break;
//	}
//	nc->connectorLock.unlock();
//
//	return can_push;
//}
//
//bool TimeMapNode::canFlush()
//{
//	bool can_flush = false;
//	NodeConnector *nc = connectors[INPUTS.AUDIO_ID];
//	
//	nc->connectorLock.lockWait();
//	std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();
//	for(auto nconn : a_conn)
//	{
//		if(can_flush |= NodeConnector::getNodeConnector(nconn.first)->getNode()->canFlush())
//			break;
//	}
//	nc->connectorLock.unlock();
//
//	return can_flush;
//}

bool TimeMapNode::flushData(FlushPacket &info)
{
	bool flushed = false;
	
	//Length of chunk (in seconds)
	const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/44100.0;


	nodeLock.lockWait();

	//Calculate flush info based on cursor position (as num chunks/time offset from cursor time 0)
	c_time	start_c = cursor.getChunkRange().start - 1,
			num_chunks = info.flushChunkRange.length(),
			end_c = start_c + num_chunks;

	nodeLock.unlock();

	ChunkRange	cursor_f_cr(start_c, end_c),
				cursor_t_cr(end_c, end_c + num_chunks);

	TimeRange	cursor_f_tr(cursor_f_cr.start*chunk_time, cursor_f_cr.end*chunk_time),
				cursor_t_tr(cursor_f_tr.end, cursor_t_cr.end*chunk_time);
	
	//Info relative to the cursor
	FlushPacket cursor_info(info);
	
	cursor_info.flushTimeRange = cursor_f_tr;
	cursor_info.flushChunkRange = cursor_f_cr;
	cursor_info.targetTimeRange = cursor_t_tr;
	cursor_info.targetChunkRange = cursor_t_cr;
	
	//Propogate flush
	flushed = Node::flushData(cursor_info);

	return flushed;
}


bool TimeMapNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	return pulled;
	
	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		//if(isPlaying())
		//{
			bool keep_playing = false;
			AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

			if(audio_output)
			{
				c_time num_chunks = audio_output->range.length();
				NodeConnector *in_a_nc = connectors[INPUTS.AUDIO_ID];

				nodeLock.lockWait();

				//Set cursor
				//cursor.active = true;

				audio_output->method = (audio_output->method != TransferMethod::DIRECT)
										? audio_output->method
										: TransferMethod::COPY;	//Override (no pointer to non-transient data to give)

				//Calling for cursor to step, or getting data at a specific location?
				bool step_cursor = (TimeRange(audio_output->range) == audio_output->globalTimeRange);

				if(step_cursor)
				{
					const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/44100.0;

					//Calculate pull ranges based on cursor position (as num chunks/time offset from cursor time 0)
					c_time	start_c = cursor.getChunkRange().start;

					ChunkRange	cursor_cr(start_c, start_c + num_chunks);
					TimeRange	cursor_tr(cursor_cr.start*chunk_time, cursor_cr.end*chunk_time);

					//Request data at cursor
					audio_output->range = cursor_cr;
					audio_output->tRange = cursor_tr;
					//audio_output->range = cursor.getChunkRange();
					//audio_output->tRange = cursor.getTimeRange();
				}
				
				////Add all audio inputs together
				//for(NodeConnector::CIndex i = 0; i < (NodeConnector::CIndex)in_a_nc->numConnections(); i++)
				//{
				//	status = in_a_nc->pullData(*audio_output, i);

				//	if(statusSucceeded(status))
				//	{
				//		keep_playing = true;
				//		audio_output->method = TransferMethod::ADD;
				//	}
				//}
				
				audio_output->method = TransferMethod::ADD;
				pulled = in_a_nc->pullData(*audio_output);
				
				if(step_cursor && keep_playing)
				{
					//Step cursor
					cursor.setTimeSamples(cursor.getChunkRange().end*AUDIO_CHUNK_SIZE);
					//cursor.step();
				}

				nodeLock.unlock();
			}

			setPlaying(keep_playing);
		//}
		//else
		//{
			//nodeLock.lockWait();
			//cursor.active = false;
			//nodeLock.unlock();

		//	status.setWarning(AS::WType::NO_ACTION_TAKEN, "TimeMapNode is not playing -- data was not modified.");
		//}
	}

	return pulled;
}

bool TimeMapNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Midi Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);

		if(audio_input)
		{
			//Just propogate push
			pushed = connectors[OUTPUTS.AUDIO_ID]->pushData(*audio_input);
		}
	}

	return pushed;
}

/*
void TimeMapNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new TimeMapNDesc()));
	TimeMapNDesc *desc = dynamic_cast<TimeMapNDesc*>(objDesc);
	
	desc->sampleRate = sampleRate;
	desc->maxBufferLength = maxBufferLength;
	desc->cursor = *(CursorDesc*)cursor.getDesc();
}
*/