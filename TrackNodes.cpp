#include "TrackNodes.h"
#include "Samplers.h"

#include <unordered_set>

#include "NodeConnection.h"
#include "NodePackets.h"
#include "ProjectTrackDisplay.h"
#include "Cursor.h"

/*


//TODO: getData functions may not be perfect



/////AUDIO BUFFER NODE/////
const std::vector<NodeConnectorDesc> AudioTrackNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Input audio to buffer.", 1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Buffered audio data.", -1) };

AudioTrackNode::AudioTrackNode(NodeGraph *parent_graph, int sample_rate, c_time track_length)
	: Node(parent_graph, NType::AUDIO_TRACK, "Audio Track", "Stores a buffer of audio data."),
		TrackNode<c_time>(track_length),
		sampleRate(sample_rate), audioData(AUDIO_CHUNK_SIZE, track_length, 0)
{
	initNode();
	
	//nodeLock.lockWait();
	//Allocate inital audio data
	//audioData.reserve(length);

	//AudioVelChunk *last_c = nullptr;
	//for(c_time c = 0; c < length; c++)
	//{
	//	audioData.push_back(new AudioVelChunk(AUDIO_CHUNK_SIZE, 0));
	//	//Link chunks together
	//	audioData[c]->prevChunk = last_c;
	//	(last_c ? last_c->nextChunk = audioData[c] : 0);
	//	last_c = audioData[c];
	//}
	//nodeLock.unlock();
}


//AudioTrackNode::AudioTrackNode(const AudioTrackNDesc &atn_desc)
//	: Node(*(NDesc*)&atn_desc),
//		TrackNode<c_time>(atn_desc.maxLength),
//		sampleRate(atn_desc.sampleRate), audioData(atn_desc.data)
//{
//	Node::init(atn_desc.connectors);
//	INPUTS.AUDIO_ID = atn_desc.connectors[0];
//	OUTPUTS.AUDIO_ID = atn_desc.connectors[1];
//
//	//audioData.reserve(atn_desc.data.size());
//	//audioData.insert(audioData.end(), atn_desc.data.begin(), atn_desc.data.end());
//}


AudioTrackNode::~AudioTrackNode()
{ }

void AudioTrackNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
	OUTPUTS.AUDIO_ID = ids[1];
}

void AudioTrackNode::onLengthChanged(c_time max_length_changed)
{
	nodeLock.lockWait();
	//if(max_length_changed > 0)
	//{
	//	c_time old_length = maxLength - max_length_changed;

	//	//Allocate space for new audio data
	//	//audioData.reserve(length);
	//	AudioVelChunk *last_c = (old_length > 0 ? audioData[old_length - 1] : nullptr);
	//	for(c_time c = old_length; c < length; c++)
	//	{
	//		audioData.push_back(new AudioVelChunk(AUDIO_CHUNK_SIZE, (c > 0 ? audioData[c - 1]->getLastSample() : 0)));
	//		audioData[c]->prevChunk = last_c;
	//		(last_c ? last_c->nextChunk = audioData[c] : 0);
	//		last_c = audioData[c];
	//	}

	//	//audioData[old_length]->prevValue = (old_length > 0 ? audioData[old_length - 1]->data[AUDIO_CHUNK_SIZE - 1] : 0);
	//	
	//	//Call for update for added chunks
	//	//if(updateStatus.size() == 0 || updateStatus[updateStatus.size() - 1].status == DataStatus::CLEAN)
	//	//	updateStatus.push_back(DataStatusDivider(old_length, DataStatus::DIRTY));
	//	
	//	//std::cout << old_length << ", " << maxLength << "\n";
	//}
	//else if(max_length_changed < 0)
	//{
	//	//Erase anything after maxLength
	//	audioData.erase(audioData.begin() + maxLength, audioData.end());
	//	audioData[maxLength - 1]->nextChunk = nullptr;
	//}
	////else keep chunks past the edge, just pretend theyre not there

	audioData.resize(maxLength);

	nodeLock.unlock();
}

//
//Range<int> AudioTrackNode::getIntersectingStatusRange(ChunkRange r)
//{
//	Range<int> ind(-1, -1);
//
//	if(updateStatus.size() == 1)
//	{
//		ind.start = 0;
//		ind.end = 0;
//	}
//	else if(updateStatus.size() > 1)
//	{
//		for(unsigned int i = 0; i < updateStatus.size(); i++)
//		{
//			if(ind.start < 0 && updateStatus[i].start >= r.start)
//				ind.start = i - 1;
//
//			if(i + 1 == updateStatus.size() || updateStatus[i + 1].start >= r.end)
//			{
//				ind.start = (ind.start < 0 ? i : ind.start);
//				ind.end = i;
//				break;
//			}
//		}
//	}
//
//	return ind;
//}


AStatus AudioTrackNode::update(ChunkRange cr)
{
	bool updated = false;

	const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;

	nodeLock.lockWait();

	AudioPullPacket packet(&(*audioData.getData())[cr.start], cr, TransferMethod::COPY, true);
	//packet.globalRange = (?)
	packet.tRange = TimeRange(cr.start*chunk_time, cr.end*chunk_time);
	
	//Update chunks
	updated = connectors[INPUTS.AUDIO_ID]->pullData(packet);
	
	//Clean chunks
	for(c_time c = packet.range.start; c < packet.range.end; c++)
	{
		AudioVelChunk *chunk = audioData.getChunk(c);
		chunk->setStatus(DataStatus::CLEAN);
		if(c > 0) audioData.getChunk(c - 1)->updateChunkStep();
		chunk->setSeed(c > 0 ? audioData.getChunk(c - 1)->getSeed() + audioData.getChunk(c - 1)->getChunkStep() : 0);
	}

	//if(display)
	//	display->getCursor()->setTimeChunks(display->getCursor()->getChunkRange().end);
	
	nodeLock.unlock();

	return AS::A_SUCCESS;
}

AStatus AudioTrackNode::updateRange(ChunkRange cr)
{
	AStatus status;

	//Update each chain of dirty chunks together
	ChunkRange u_cr(-1, -1);
	for(c_time c = cr.start; c < cr.end; c++)
	{
		bool continue_chain = audioData.getChunk(c)->getStatus() == DataStatus::DIRTY;
		if(continue_chain)
		{
			//Continue chain
			u_cr.start = (u_cr.start < 0 ? c : u_cr.start);
			u_cr.end = c + 1;
		}

		if((u_cr.start >= 0 && u_cr.end >= 0) && (!continue_chain || (c + 1 == cr.end && continue_chain)))
		{
			//Update current chain
			status = update(u_cr);
			if(!statusGood(status))
				break;

			//Reset chain
			u_cr.start = -1;
			u_cr.end = -1;
		}
	}

	
	////Find range of status sections that intersect the range
	//Range<int> status_r = getIntersectingStatusRange(r);

	//if(status_r.start >= 0 && status_r.end >= 0)
	//{
	//	//Update non-clean status sections
	//	for(unsigned int i = status_r.start; i <= status_r.end; i++)
	//	{
	//		if(updateStatus[i].status != DataStatus::CLEAN)
	//		{
	//			ChunkRange u_r;
	//			u_r.start = (i == status_r.start) ? r.start : updateStatus[i].start;
	//			u_r.end = (i == status_r.end) ? r.end : updateStatus[i + 1].start;
	//			
	//			status = update(u_r);
	//			if(!statusGood(status))
	//				return status;
	//		}
	//	}

	//	if(status_r.length() == 0)	//start == end
	//	{
	//		DataStatus s = updateStatus[status_r.start].status;
	//		if(s != DataStatus::CLEAN)
	//		{
	//			//New clean section
	//			if(updateStatus[status_r.start].start == r.start)
	//			{
	//				updateStatus[status_r.start].status = DataStatus::CLEAN;
	//			}
	//			else
	//			{
	//				updateStatus.insert(updateStatus.begin() + status_r.start + 1, DataStatusDivider(r.start, DataStatus::CLEAN));
	//				status_r.start++;
	//			}
	//			//Trailing old section
	//			updateStatus.insert(updateStatus.begin() + status_r.start + 1, DataStatusDivider(r.end, s));
	//		}
	//	}
	//	else
	//	{
	//		//Adjust last intersecting status divider
	//		updateStatus[status_r.end].start = r.end;
	//
	//		if(status_r.length() >= 2)
	//		{
	//			//Delete status dividers completely contained within cleared range
	//			//	Also delete last_section (combine with new section) if its clean
	//			updateStatus.erase( updateStatus.begin() + status_r.start + 1,
	//								updateStatus.begin() + status_r.end - 1*(updateStatus[status_r.end].status != DataStatus::CLEAN) );
	//		}

	//		//Add divider for new clean section, unless first_section is also clean
	//		if(updateStatus[status_r.start].status != DataStatus::CLEAN)
	//			updateStatus.insert(updateStatus.begin() + status_r.start, DataStatusDivider(r.start, DataStatus::CLEAN));
	//	}
	//}
	//else
	//	status.setError(AS::ErrorType::INDEX_OUT_OF_BOUNDS, "UpdateStatus indices came back negative -- AUDIO TRACK UPDATE.");
		
	return status;
}

void AudioTrackNode::onConnect(NCID this_nc, NCID other_nc)
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

void AudioTrackNode::onDisconnect(NCID this_nc, NCID other_nc)
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

void AudioTrackNode::rangeTest()
{
	//TEST FUNCTION
}

MidiTrackNode* AudioTrackNode::getMidiNode()
{
	return midiNode;
}


//const StatusList* AudioTrackNode::getStatus()
//{
//	return &updateStatus;
//}


const AudioVelData* AudioTrackNode::getData() const
{
	return &audioData;
}

std::vector<ChildNodeTree> AudioTrackNode::getChildren()
{
	childNodes.clear();
	getChildNodes({ NodeType::AUDIO_TRACK, NodeType::MIDI_TRACK }, childNodes);
	return childNodes;
}


//TODO: Double check this function (shifting status dividers)
void AudioTrackNode::clearRange(ChunkRange r, bool compress)
{
	//if(r.start >= maxLength) r.start = maxLength - 1;
	//if(r.end >= maxLength) r.end = maxLength - 1;

	//c_time num_chunks_cleared = r.end - r.start;

	//if(num_chunks_cleared > 0)
	//{
	//	if(compress)
	//	{
	//		//Delete chunks
	//		for(c_time c = r.start; c < r.end; c++)
	//			delete audioData[c];

	//		//Compress
	//		audioData.erase(audioData.begin() + r.start, audioData.begin() + r.end);

	//		//Find range of status sections that intersect the cleared range
	//		int		first_section = -1,	//First and last sections that the range intersects.
	//				last_section = -1;
	//		if(updateStatus.size() == 1)
	//		{
	//			first_section = 0;
	//			last_section = 0;
	//		}
	//		else
	//		{
	//			for(unsigned int i = 0; i < updateStatus.size(); i++)
	//			{
	//				if(first_section < 0 && i != 0 && updateStatus[i].start >= r.start)
	//					first_section = i - 1;
	//				if(i == updateStatus.size() - 1 || updateStatus[i + 1].start > r.end)
	//				{
	//					last_section = i;
	//					if(first_section < 0) first_section = i;
	//					break;
	//				}
	//			}
	//		}
	//		
	//		//Adjust last intersecting status divider
	//		if(first_section != last_section)
	//			updateStatus[last_section].start = r.start;

	//		//Adjust non-intersecting status dividers after the cleared range
	//		for(unsigned int i = last_section + 1; i < updateStatus.size(); i++)
	//			updateStatus[i].start -= num_chunks_cleared;

	//		//Delete status dividers completely contained within cleared range
	//		updateStatus.erase(updateStatus.begin() + first_section + 1, updateStatus.begin() + last_section - 1);

	//		//Adjust length
	//		length -= (r.end - r.start);
	//		maxLength -= (r.end - r.start);
	//	}
	//	else
	//	{
	//		//Set chunks to 0
	//		for(c_time c = r.start; c < r.end; c++)
	//			audioData[c]->setData(0, audioData[c]->size);

	//		//TODO: Reflect this somehow in needUpdate (does this range now need update? not need update?)

	//	}
	//}
	
}


void AudioTrackNode::onUpdate(const Time &dt)
{
	//Update length
	if(midiNode)
		setLength((c_time)ceil(midiNode->getLength()*(double)sampleRate/(double)AUDIO_CHUNK_SIZE));
}


//void AudioTrackNode::setDisplay(ProjectTrackDisplay *new_display)
//{
//	if(display != new_display)
//	{
//		if(display)
//			display->removeTrack(this);
//
//		if(new_display)
//			new_display->addTrack(this);
//
//		display = new_display;
//	}
//}



//bool AudioTrackNode::setDirty(ChunkRange r)
//{
//	AStatus status;
//
//	//Convert to chunk range
//	//const double mult = (double)sampleRate/(double)AUDIO_CHUNK_SIZE;
//	//ChunkRange c_r((c_time)ceil(r.start*mult), (c_time)ceil(r.end*mult));
//
//	r.start = max(r.start, 0);
//	r.end = min(r.end, maxLength);
//
//	if(r.length() > 0)
//	{
//		nodeLock.lockWait();
//		//Make all chunks in the range dirty
//		for(c_time c = r.start; c < r.end; c++)
//		{
//			audioData[c]->updateAll();
//			audioData[c]->setStatus(DataStatus::DIRTY);
//		}
//		nodeLock.unlock();
//
//		//Propogate dirty set
//		Node::setDirty(r);
//	}
//	else
//	{
//		status.setError(AS::ErrorType::INDEX_OUT_OF_BOUNDS, "AUDIO TRACK SET DIRTY -- range out of bounds.");
//		std::cout << r.start << ", " << r.end << "\n";
//	}
//
//
//	return status;
//}



//bool AudioTrackNode::canPull()
//{
//	return true;
//}
//bool AudioTrackNode::canPush()
//{
//	
//	//bool can_push = false;
//	//NodeConnector *nc = connectors[OUTPUTS.AUDIO_ID];
//	//
//	//nc->connectorLock.lockWait();
//	//for(NodeConnector::CIndex i = 0; i < (NodeConnector::CIndex)nc->numConnections(); i++)
//	//{
//	//	if(can_push |= nc->canPush(i))
//	//		break;
//	//}
//	//nc->connectorLock.unlock();
//
//	//return can_push;
//	
//
//	return static_cast<bool>(pRecordCursor);
//}
//
//
//bool AudioTrackNode::canFlush()
//{
//	bool can_flush = false;
//	NodeConnector *nc = connectors[INPUTS.AUDIO_ID];
//	
//	nc->connectorLock.lockWait();
//	std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();
//	for(auto nconn : a_conn)
//	{
//		if(can_flush = NodeConnector::getNodeConnector(nconn.first)->getNode()->canFlush())
//			break;
//	}
//	nc->connectorLock.unlock();
//
//	return can_flush;
//}


bool AudioTrackNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;

	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

		if(audio_output)
		{
			c_time	old_num_chunks = audio_output->range.length();
			const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;
			
			//Clamp range to be within data range
			audio_output->range = ChunkRange(max(audio_output->range.start, (c_time)0),
											 min(audio_output->range.end, maxLength));
	
			c_time num_chunks = audio_output->range.length();

			//Check range
			if(num_chunks > 0)
			{
				if(num_chunks != old_num_chunks)
				{
					//status.setWarning(AS::WType::PARAMETERS_CHANGED, "Requested range is not possible. Range has been adjusted.");
					audio_output->tRange = TimeRange(audio_output->range.start*chunk_time, audio_output->range.end*chunk_time);
				}
				//Update range if requested
				if(audio_output->update && !updateRange(audio_output->range))
					return false;
				
				c_time start_chunk = audio_output->range.start;

				nodeLock.lockWait();

				//Pass data
				switch(audio_output->method)
				{
				case TransferMethod::COPY:
					for(c_time c = 0; c < num_chunks; c++)
						*audio_output->data[c] = *audioData.getChunk(start_chunk + c);
					break;

				case TransferMethod::ADD:
					//Add data to output pointer
					//status = AudioChunk::sampleChunks((const AudioChunk**)(&audioData[start_chunk]), audio_output->data,
					//								  (start_chunk > 0 ? audioData[start_chunk - 1] : nullptr), num_chunks,
					//								  audio_output->sampleMethod, (audio_output->method == TransferMethod::ADD));
					for(c_time c = 0; c < num_chunks; c++)
						*audio_output->data[c] += *audioData.getChunk(start_chunk + c);
					break;

				case TransferMethod::DIRECT:
					//Return pointer to buffer data
					//	WARNING: READ ONLY, DO NOT OVERWRITE!! BUFFER WILL BE OVERWRITTEN.
					audio_output->data = audioData.getData()->data() + start_chunk;
					break;
				}

				pulled = true;

				nodeLock.unlock();
			}
		}
	}

	return pulled;
}

bool AudioTrackNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);
		if(audio_input)
		{
			if(pRecordCursor)	//If recording
			{
				ChunkRange cr = audio_input->range;//pRecordCursor->getChunkRange();//(start_chunk, start_chunk + num_chunks);
				c_time num_chunks = cr.length();

				//Clamp range to be within data range
				cr.start = max(cr.start,  (c_time)0);
				cr.end = min(cr.end, maxLength);
	
				//Check range
				if(cr.length() <= 0)
					return false;//AStatus(AS::A_ERROR, "Requested range is outside of data");
				
				//Adjust track length
				//TODO: Make option for stopping recording if it goes out of range
				//if(cr.end > maxLength)
				//	setLength(cr.end);

				//Update range if requested
				//if(!statusGood(status = updateRange(r)))
				//	return status;
				
				nodeLock.lockWait();
				//Copy data
				for(c_time c = 0; c < num_chunks; c++)
				{
					*audioData.getChunk(cr.start + c) = *audio_input->data[c];
				}
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

void AudioTrackNode::update(const Time &dt)
{

}


//void AudioTrackNode::updateDesc()
//{
//	Node::updateDesc();
//	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new RenderNDesc()));
//	AudioTrackNDesc *desc = dynamic_cast<AudioTrackNDesc*>(objDesc);
//	
//	desc->sampleRate = sampleRate;
//	desc->data.resize(audioData.getNumChunks());
//
//	for(unsigned int i = 0; i < audioData.getNumChunks(); i++)
//		desc->data.getData()[i] = audioData.getData()[i];
//}



/////MIDI BUFFER NODE
const std::vector<NodeConnectorDesc> MidiTrackNode::nc_descs =
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_INPUT, "MIDI Input", "Input MIDI to buffer.", -1),
			  NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "MIDI Output", "Buffered MIDI data.", -1) };

MidiTrackNode::MidiTrackNode(NodeGraph *parent_graph, c_time track_length)
	: Node(parent_graph, NType::MIDI_TRACK, "MIDI Track", "Stores a buffer of MIDI notes."), TrackNode(track_length), midiData(), initial_mods()
{
	initNode();
}


//MidiTrackNode::MidiTrackNode(const MidiTrackNDesc &mtn_desc)
//	: Node(*(NDesc*)&mtn_desc), TrackNode<Time>(mtn_desc.maxLength),
//		sampleRate(mtn_desc.sampleRate), midiData(mtn_desc.data)
//{
//	Node::init(mtn_desc.connectors);
//	INPUTS.MIDI_ID = mtn_desc.connectors[0];
//	OUTPUTS.MIDI_ID = mtn_desc.connectors[1];
//}


MidiTrackNode::~MidiTrackNode()
{ }

void MidiTrackNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.MIDI_ID = ids[0];
	OUTPUTS.MIDI_ID = ids[1];
}

void MidiTrackNode::onUpdate(const Time &dt)
{
	
	//if(pRecordCursor)
	//{
	//	double cursor_time = (double)(pRecordCursor->getTime() + AUDIO_CHUNK_SIZE)/sampleRate;
	//	for(auto n : unfinishedNotes)
	//		n->end = cursor_time;
	//}
	
}

void MidiTrackNode::setBuffer(const MidiData &data)
{
	nodeLock.lockWait();

	midiData = data;
	//Update length
	setLength(data.getLength());

	//Make forward nodes dirty
	//connectors[OUTPUTS.MIDI_ID]->setDirty(ChunkRange(0, maxLength));//data.getSpan());

	nodeLock.unlock();
}

void MidiTrackNode::onLengthChanged(double max_length_changed)
{

}

void MidiTrackNode::onRecordChanged(Cursor *p_old_cursor, bool recording)
{
	if(!recording && p_old_cursor)
	{
		//Stopped recording//

		nodeLock.lockWait();

		//double cursor_time = p_old_cursor->getTimeRange().end;//(double)(p_old_cursor->getTime())/sampleRate;

		//double	g_time = Clock::getGlobalTime(),	
		//		rel_time = p_old_cursor->getTimeRange().start;//TODO: Figure this out --> // + (curr_time - AUDIO_CLOCK.t);		//Current time relative to start of the track

		//Set ends of notes to end of chunk that recording stopped in
		//	(for now)
		double note_end = p_old_cursor->getTimeRange().start;

		for(auto n : unfinishedNotes)
		{
			if(note_end > n->range.start)
				n->setFinished(note_end);
			else if(!statusSucceeded(midiData.removeNote(n)))
				std::cout << "WEIRD ERROR --> MidiTrackNode onRecordChanged : note not in midiData!\n";
		}
		unfinishedNotes.clear();
		
		nodeLock.unlock();
	}
}

void MidiTrackNode::applyEvents(const MidiEventQueue &events, const TimeRange &range, ChunkRange &c_range)
{
	//Event times are relative to the given range, no need to offset.
	
	nodeLock.lockWait();

	MidiEventQueue e_q(events);

	MidiEvent e = e_q.front();

	while(e.time < range.end)
	{
		//std::cout << "MIDI TRACK APPLY --> " << std::fixed << e.time << ", (" << std::fixed << range.start << ", " << std::fixed << range.end << ")\n";

		//
		
		//if(e.time >= range.end)
		//{
		//	c_range.end++;
		//	//std::cout << "ADJUSTED CHUNK RANGE END: (" << e.midiIndex << ", " << e.type << " : " << e.time << " --> " << e.fTime << ")\n";
		//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
		//	e.time = e.fTime;
		//}
		//else if(e.time < range.start)
		//{
		//	//TODO: Figure out what to do when note starts before start of range
		//	//	FOR NOW: just use fTime (starts at beginning of range)

		//	//std::cout << "ADJUSTED NOTE START: " << e.time << " --> " << e.fTime << "\n";
		//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
		//	e.time = e.fTime;

		//	//c_range.start--;
		//	//std::cout << "ADJUSTED CHUNK RANGE START --> (" << e.midiIndex << ", " << e.type << " : " << e.time << "\n";
		//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
		//}
		

		switch(e.type)
		{
		case MidiEventType::NOTE_ON:
			//If note with this index is already on, stop it prematurely
			for(auto n_iter = unfinishedNotes.begin(); n_iter != unfinishedNotes.end(); n_iter++)
			{
				MidiNote *n = *n_iter;
				if(n->index == e.midiIndex)
				{
					//Finish note and remove from unfinishedNotes
					n->setFinished(e.time);
					if(n->range.length() <= 0.0)
						midiData.removeNote(n);
					unfinishedNotes.erase(n_iter);
					
					//std::cout << "FINISHED NOTE (EARLY) --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
					
					break;
				}
			}

			//Add new (unfinished) note
			unfinishedNotes.push_back(midiData.addNote(MidiNote(e.midiIndex, e.velocity, e.time)));
			//std::cout << "STARTED NOTE --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";

			break;

		case MidiEventType::NOTE_OFF:
			//If note with this index is on, stop it
			for(auto n_iter = unfinishedNotes.begin(); n_iter != unfinishedNotes.end(); n_iter++)
			{
				MidiNote *n = *n_iter;
				if(n->index == e.midiIndex)
				{
					//Finish note and remove from unfinishedNotes
					n->setFinished(e.time);
					if(n->range.length() <= 0.0)
						midiData.removeNote(n);
					unfinishedNotes.erase(n_iter);
					
					//std::cout << "FINISHED NOTE --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
					
					break;
				}
			}

			break;
		}

		e_q.pop_front();

		if(!e_q.empty())
			e = e_q.front();
		else
			break;
	}
	
	nodeLock.unlock();
	
	//connectors[OUTPUTS.MIDI_ID]->setDirty(TimeRange(start_time, last_event + (double)AUDIO_CHUNK_SIZE/sampleRate));
}

void MidiTrackNode::clearRange(TimeRange r, bool compress)
{

}

const MidiData* MidiTrackNode::getData() const
{
	return &midiData;
}


//bool MidiTrackNode::canPull()
//{
//	return true;
//}
//bool MidiTrackNode::canPush()
//{
//	return static_cast<bool>(pRecordCursor);
//}
//
//bool MidiTrackNode::canFlush()
//{
//	bool can_flush = false;
//
//	//if(pRecordCursor)
//	//{
//		can_flush = unfinishedNotes.size() > 0;
//		NodeConnector *nc = connectors[INPUTS.MIDI_ID];
//	
//		nc->connectorLock.lockWait();
//		std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();
//		for(auto nconn : a_conn)
//			can_flush = NodeConnector::getNodeConnector(nconn.first)->getNode()->canFlush();
//		nc->connectorLock.unlock();
//	//}
//
//	return can_flush;
//}

bool MidiTrackNode::flushData(FlushPacket &info)
{
	bool flushed = false;

	//info is relative to pRecordCursor already. (?)

	//Propogate flush
	flushed = Node::flushData(info);

	if(flushed && pRecordCursor && unfinishedNotes.size() > 0)
	{
		nodeLock.lockWait();

		//Set end of notes to the end of this flush's range (they're still being pressed)
		for(auto n : unfinishedNotes)
			n->range.end = pRecordCursor->getTimeRange().end;//info.targetTimeRange.end;
		
		nodeLock.unlock();
		
		//Make forward nodes dirty
		//flushed = connectors[OUTPUTS.MIDI_ID]->setDirty(info.targetChunkRange);
	}
	
	return flushed;
}

bool MidiTrackNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	
	//Check output
	if(this_id == OUTPUTS.MIDI_ID)
	{
		MidiPullPacket *midi_output = dynamic_cast<MidiPullPacket*>(&output);
		
		if(midi_output)
		{
			nodeLock.lockWait();
			
			midi_output->mods = initial_mods;
			midiData.getNotes(midi_output->range, midi_output->notes);
			pulled = true;//(midi_output->notes.numNotes() > 0);

			nodeLock.unlock();
		}
	}

	return pulled;
}

bool MidiTrackNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.MIDI_ID)	//Midi Packet
	{
		if(pRecordCursor)	//If recording
		{
			MidiPushPacket *midi_input = dynamic_cast<MidiPushPacket*>(&input);

			if(midi_input)
			{
				const double	chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate,
								chunk_time_inv = 1.0/chunk_time;

				//Get range of pushed data
				TimeRange tr = midi_input->range;//midi_input->range.end, midi_input->range.end + midi_input->range.length());
				ChunkRange cr((c_time)std::round(tr.start*chunk_time_inv), (c_time)std::round(tr.end*chunk_time_inv));

				//Adjust track length
				//TODO: Make option for stopping recording if it goes out of range
				//if(cr.end > maxLength)
				//	setLength(cr.end);
				
				//Record data
				applyEvents(midi_input->events, tr, cr);
				//Make forward nodes dirty
				//pushed = connectors[OUTPUTS.MIDI_ID]->setDirty(cr);

				//Propogate push
				//connectors[OUTPUTS.MIDI_ID]->pushData(*midi_input);
				pushed = true;
			}
		}
		//Only accept pushed audio data if this track is recording
	}

	return pushed;
}


//void MidiTrackNode::updateDesc()
//{
//	Node::updateDesc();
//	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new RenderNDesc()));
//	MidiTrackNDesc *desc = dynamic_cast<MidiTrackNDesc*>(objDesc);
//	
//	desc->sampleRate = sampleRate;
//
//	ConstMidiData md(midiData.getConstNotes());
//	desc->data.resize(md.size());
//
//	for(unsigned int i = 0; i < md.size(); i++)
//	{
//		desc->data[i] = *md[i];
//		desc->data[i].finished = true;
//	}
//}


/////MOD TRACK NODE

ModTrackNode::ModTrackNode(c_time track_length)
	: Node(nullptr, NType::INVALID, "Null Mod Track Node", "Unspecified"), TrackNode(track_length), ModNode(), points()
{
	reset();
}

void ModTrackNode::reset()
{
	points.clear();
	points.push_back(TrackPoint(0, 0.0, Interp::LINEAR));
	points.push_back(TrackPoint(1000000, 0.0, Interp::LINEAR));
}


/////AUDIO MOD TRACK NODE
const std::vector<NodeConnectorDesc> AudioModTrackNode::nc_descs =
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Input audio to modify.", 1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Modified audio data.", -1) };

AudioModTrackNode::AudioModTrackNode(NodeGraph *parent_graph, AudioModFunction mod_function, c_time track_length)
	: Node(parent_graph, NType::AUDIO_MOD_TRACK, "Audio-Mod Track", "Modifies audio data over time."), ModTrackNode(track_length), modify(mod_function)
{
	initNode();
}


//AudioModTrackNode::AudioModTrackNode(const AudioModTrackNDesc &amtn_desc)
//	: Node(*(NDesc*)&amtn_desc), ModTrackNode(),
//		modify(amtn_desc.modifyFunc)
//{
//	Node::init(amtn_desc.connectors);
//	INPUTS.AUDIO_ID = amtn_desc.connectors[0];
//	OUTPUTS.AUDIO_ID = amtn_desc.connectors[1];
//
//	points.reserve(amtn_desc.points.size());
//	points.insert(points.end(), amtn_desc.points.begin(), amtn_desc.points.end());
//}


void AudioModTrackNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.AUDIO_ID = ids[0];
	OUTPUTS.AUDIO_ID = ids[1];
}

void AudioModTrackNode::onLengthChanged(c_time max_length_changed)
{

}

void AudioModTrackNode::clearRange(ChunkRange r, bool compress)
{

}

//bool AudioModTrackNode::canPull()
//{
//	return true;
//}
//bool AudioModTrackNode::canPush()
//{
//	return false;
//}
//bool AudioModTrackNode::canFlush()
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

bool AudioModTrackNode::flushData(FlushPacket &info)
{
	bool flushed;

	//Propogate flush
	flushed = Node::flushData(info);

	//No unique data to push

	return flushed;
}

//TODO: Check this
bool AudioModTrackNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled;
	
	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		//Check connections
		if(connectors[INPUTS.AUDIO_ID]->numConnections() > 0)
		{
			AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

			if(audio_output)
			{
				audio_output->method = TransferMethod::COPY;			//Most likely receiving from a buffer; shouldnt overwrite	

				//Pull data
				//AudioPullPacket audio_input = *audio_output;
				pulled = connectors[INPUTS.AUDIO_ID]->pullData(*audio_output);
				if(!pulled)
					return pulled;

				//Modify data
				if(modify)
				{
					std::cout << "MODIFYING AUDIO... (PULL)\n";
					modify((const AudioVelChunk**)audio_output->data, audio_output->data, audio_output->globalRange.start);
				}
			}
		}
	}

	return pulled;
}

bool AudioModTrackNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed;

	if(this_id == INPUTS.AUDIO_ID)
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);

		if(audio_input)
		{
			//Modify data
			if(modify)
			{
				std::cout << "MODIFYING AUDIO... (PUSH)\n";
				modify((const AudioVelChunk**)audio_input->data, audio_input->data, Clock::getGlobalTime());
			}


		}
	}

	return pushed;
}

//
//void AudioModTrackNode::updateDesc()
//{
//	Node::updateDesc();
//	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new RenderNDesc()));
//	AudioModTrackNDesc *desc = dynamic_cast<AudioModTrackNDesc*>(objDesc);
//	
//	desc->modifyFunc = modify;
//
//	desc->points.reserve(points.size());
//
//	for(unsigned int i = 0; i < points.size(); i++)
//		desc->points.push_back(points[i]);
//}



/////MIDI MOD TRACK NODE
const std::vector<NodeConnectorDesc> MidiModTrackNode::nc_descs =
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_INPUT, "MIDI Input", "Input MIDI to modify.", 1),
			  NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "MIDI Output", "Modified MIDI data.", -1) };

MidiModTrackNode::MidiModTrackNode(NodeGraph *parent_graph, SampleModFunction mod_function, c_time track_length)
	: Node(parent_graph, NType::MIDI_MOD_TRACK, "MIDI-Mod Track", "Modifies MIDI data over time."), ModTrackNode(track_length), modify(mod_function)
{
	initNode();
}

//
//MidiModTrackNode::MidiModTrackNode(NodeGraph *parent_graph, const MidiModTrackNDesc &mmtn_desc)
//	: Node(parent_graph, *(NDesc*)&mmtn_desc), ModTrackNode(),
//		modify(mmtn_desc.modifyFunc)
//{
//	Node::init(mmtn_desc.connectors);
//	INPUTS.MIDI_ID = mmtn_desc.connectors[0];
//	OUTPUTS.MIDI_ID = mmtn_desc.connectors[1];
//
//	points.reserve(mmtn_desc.points.size());
//	points.insert(points.end(), mmtn_desc.points.begin(), mmtn_desc.points.end());
//}


void MidiModTrackNode::initNode()
{	
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.MIDI_ID = ids[0];
	OUTPUTS.MIDI_ID = ids[1];
}

void MidiModTrackNode::onLengthChanged(c_time max_length_changed)
{

}

void MidiModTrackNode::clearRange(ChunkRange r, bool compress)
{

}

//bool MidiModTrackNode::canPull()
//{
//	return true;
//}
//bool MidiModTrackNode::canPush()
//{
//	return false;
//}
//bool MidiModTrackNode::canFlush()
//{
//	bool can_flush = false;
//	NodeConnector *nc = connectors[INPUTS.MIDI_ID];
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

bool MidiModTrackNode::flushData(FlushPacket &info)
{
	bool flushed;

	//Propogate flush
	flushed = Node::flushData(info);

	//No unique data to push

	return flushed;
}

bool MidiModTrackNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled;

	if(this_id == OUTPUTS.MIDI_ID)	//Midi Packet
	{
		//Check connections
		if(connectors[INPUTS.MIDI_ID]->numConnections() > 0)
		{
			MidiPullPacket *midi_output = dynamic_cast<MidiPullPacket*>(&output);

			if(midi_output)
			{
				//Pull data
				pulled = connectors[INPUTS.MIDI_ID]->pullData(*midi_output);
				if(!pulled)
					return pulled;

				//Put modify at the back of the mods list (happens after previous mods)
				if(modify)
					midi_output->mods.push_back(modify);
			}
		}
	}

	return pulled;
}

bool MidiModTrackNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed;

	if(this_id == INPUTS.MIDI_ID)
	{
		MidiPushPacket *midi_input = dynamic_cast<MidiPushPacket*>(&input);

		if(midi_input)
		{
			//Put modify at the back of mods list
			if(modify)
				midi_input->mods.push_back(modify);

			//Push data
			connectors[OUTPUTS.MIDI_ID]->pushData(*midi_input);
		}
	}

	return pushed;
}


//void MidiModTrackNode::updateDesc()
//{
//	Node::updateDesc();
//	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new RenderNDesc()));
//	MidiModTrackNDesc *desc = dynamic_cast<MidiModTrackNDesc*>(objDesc);
//	
//	desc->modifyFunc = modify;
//
//	desc->points.reserve(points.size());
//
//	for(unsigned int i = 0; i < points.size(); i++)
//		desc->points.push_back(points[i]);
//}

*/