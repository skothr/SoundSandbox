#include "MidiBufferNode.h"

#include "Node.h"
#include "NodePackets.h"
#include "NodeConnection.h"


/////MIDI BUFFER NODE/////

 
const std::vector<NodeConnectorDesc> MidiBufferNode::nc_descs =
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_INPUT, "MIDI Input", "Input MIDI to buffer.", -1),
			  NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "MIDI Output", "Buffered MIDI data.", -1) };


 
MidiBufferNode::MidiBufferNode()
	: Node(nullptr, NType::INVALID, "Null Midi Buffer Node", "Unspecified"),
		BufferNode(0, MidiData())
{
	initNode();
}

 
MidiBufferNode::~MidiBufferNode()
{

}


 
void MidiBufferNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.MIDI_ID = ids[0];
	OUTPUTS.MIDI_ID = ids[1];
}

/*
 
void MidiBufferNode::onUpdate(const Time &dt)
{

}
*/


 
void MidiBufferNode::setBuffer(const MidiData &new_data)
{
	nodeLock.lockWait();

	data = new_data;
	
	//TODO: Make forward nodes dirty
	//connectors[OUTPUTS.MIDI_ID]->setDirty(ChunkRange(0, maxLength));//data.getSpan());

	nodeLock.unlock();
}

 
void MidiBufferNode::onLengthChanged(Time d_length)
{

}


void MidiBufferNode::applyEvent(MidiEvent e)
{
	//Event times are relative to the given range, no need to offset.
	
	nodeLock.lockWait();

	//Convert event time from global to local cursor
	e.time -= globalRange.start;
	e.time += cursor.getTimeRange().start;

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
					data.removeNote(n);
				unfinishedNotes.erase(n_iter);
					
				break;
			}
		}

		//Add new (unfinished) note
		unfinishedNotes.push_back(data.addNote(MidiNote(e.midiIndex, e.velocity, e.time, cursor.getTimeRange().end)));
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
					data.removeNote(n);
				unfinishedNotes.erase(n_iter);

				break;
			}
		}

		break;
	}

	nodeLock.unlock();
}

 /*
void MidiBufferNode::applyEvents(const MidiEventQueue &events, const TimeRange &range, ChunkRange &c_range)
{
	//Event times are relative to the given range, no need to offset.
	
	nodeLock.lockWait();

	//MidiEventQueue e_q(events);
	//MidiEvent e = e_q.front();

	std::vector<MidiNote*> new_notes;

	for(auto e : events)
	{
		if(e.time >= range.start && e.time < range.end)
		{
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
							data.removeNote(n);
						unfinishedNotes.erase(n_iter);
					
						//std::cout << "FINISHED NOTE (EARLY) --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
					
						break;
					}
				}

				//Add new (unfinished) note
				unfinishedNotes.push_back(data.addNote(MidiNote(e.midiIndex, e.velocity, e.time, -1.0)));
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
							data.removeNote(n);
						unfinishedNotes.erase(n_iter);
					
						//std::cout << "FINISHED NOTE --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
					
						break;
					}
				}

				break;
			}
		}
	}


	
	//while(e.time < range.end)
	//{
	//	//std::cout << "MIDI TRACK APPLY --> " << std::fixed << e.time << ", (" << std::fixed << range.start << ", " << std::fixed << range.end << ")\n";

	//	//
	//	
	//	//if(e.time >= range.end)
	//	//{
	//	//	c_range.end++;
	//	//	//std::cout << "ADJUSTED CHUNK RANGE END: (" << e.midiIndex << ", " << e.type << " : " << e.time << " --> " << e.fTime << ")\n";
	//	//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
	//	//	e.time = e.fTime;
	//	//}
	//	//else if(e.time < range.start)
	//	//{
	//	//	//TODO: Figure out what to do when note starts before start of range
	//	//	//	FOR NOW: just use fTime (starts at beginning of range)

	//	//	//std::cout << "ADJUSTED NOTE START: " << e.time << " --> " << e.fTime << "\n";
	//	//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
	//	//	e.time = e.fTime;

	//	//	//c_range.start--;
	//	//	//std::cout << "ADJUSTED CHUNK RANGE START --> (" << e.midiIndex << ", " << e.type << " : " << e.time << "\n";
	//	//	//std::cout << "\tRANGE: (" << range.start << ", " << range.end << ")\n";
	//	//}
	//	

	//	switch(e.type)
	//	{
	//	case MidiEventType::NOTE_ON:
	//		//If note with this index is already on, stop it prematurely
	//		for(auto n_iter = unfinishedNotes.begin(); n_iter != unfinishedNotes.end(); n_iter++)
	//		{
	//			MidiNote *n = *n_iter;
	//			if(n->index == e.midiIndex)
	//			{
	//				//Finish note and remove from unfinishedNotes
	//				n->setFinished(e.time);
	//				if(n->range.length() <= 0.0)
	//					data.removeNote(n);
	//				unfinishedNotes.erase(n_iter);
	//				
	//				//std::cout << "FINISHED NOTE (EARLY) --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
	//				
	//				break;
	//			}
	//		}

	//		//Add new (unfinished) note
	//		unfinishedNotes.push_back(data.addNote(MidiNote(e.midiIndex, e.velocity, e.time)));
	//		//std::cout << "STARTED NOTE --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";

	//		break;

	//	case MidiEventType::NOTE_OFF:
	//		//If note with this index is on, stop it
	//		for(auto n_iter = unfinishedNotes.begin(); n_iter != unfinishedNotes.end(); n_iter++)
	//		{
	//			MidiNote *n = *n_iter;
	//			if(n->index == e.midiIndex)
	//			{
	//				//Finish note and remove from unfinishedNotes
	//				n->setFinished(e.time);
	//				if(n->range.length() <= 0.0)
	//					data.removeNote(n);
	//				unfinishedNotes.erase(n_iter);
	//				
	//				//std::cout << "FINISHED NOTE --> " << e.time << " --> (" << range.start << ", " << range.end << ")\n";
	//				
	//				break;
	//			}
	//		}

	//		break;
	//	}

	//	e_q.pop_front();

	//	if(!e_q.empty())
	//		e = e_q.front();
	//	else
	//		break;
	//}
	//
	nodeLock.unlock();
	
	//connectors[OUTPUTS.MIDI_ID]->setDirty(TimeRange(start_time, last_event + (double)AUDIO_CHUNK_SIZE/sampleRate));
}
*/

/////STATIC MIDI BUFFER NODE/////

StaticMidiBufferNode::StaticMidiBufferNode(NodeGraph *parent_graph, Time initial_length)
	: Node(parent_graph, NType::STATIC_MIDI_BUFFER, "Static Midi Buffer Node", "Stores buffered midi that has been recorded onto it."),
		BufferNode<Time, MidiData>(initial_length, MidiData()),
		MidiBufferNode()
{ }

StaticMidiBufferNode::~StaticMidiBufferNode()
{

}
//
//bool StaticMidiBufferNode::flushData(FlushPacket &info)
//{
//	bool flushed = false;
//
//	//info is relative to pRecordCursor already. (?)
//
//	//Propogate flush
//	flushed = Node::flushData(info);
//
//	if(pCursor && unfinishedNotes.size() > 0)
//	{
//		nodeLock.lockWait();
//
//		//Set end of notes to the end of this flush's range (they're still being pressed)
//		for(auto n : unfinishedNotes)
//			n->range.end = pCursor->getTimeRange().end;//info.targetTimeRange.end;
//		
//		nodeLock.unlock();
//		
//		//Make forward nodes dirty
//		//flushed = connectors[OUTPUTS.MIDI_ID]->setDirty(info.targetChunkRange);
//	}
//	
//	return flushed;
//}

 
bool StaticMidiBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	
	//Check output
	if(this_id == OUTPUTS.MIDI_ID)
	{
		MidiPullPacket *midi_output = dynamic_cast<MidiPullPacket*>(&output);
		
		//Can always pull from a static node, no need to be recording.
		if(midi_output)
		{
			Cursor *pull_cursor = midi_output->pCursor;

			if(pull_cursor)
				cursor.setTimeSamples(pull_cursor->getSampleRange().start);
			
			//const Time chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;
			
			midi_output->mods = initial_mods;
			//MidiSet set = data.getNotesInRange(midi_output->range);
			//std::vector<MidiNote*> vec(set.begin(), set.end());

			//std::vector<MidiNote*> vec();

			MidiSet notes = data.getNotesInRange(cursor.getTimeRange());
			midi_output->notes.insert(notes.begin(), notes.end());
			
			for(auto n : unfinishedNotes)
				n->range.end = cursor.getTimeRange().end;
			
			pulled = true;

			cursor.step();
		}
	}

	return pulled;
}

 
bool StaticMidiBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.MIDI_ID)	//Midi Packet
	{
		if(recording)	//Only if recording
		{
			MidiPushPacket *midi_input = dynamic_cast<MidiPushPacket*>(&input);
			//MidiEventPushPacket *midi_event = dynamic_cast<MidiEventPushPacket*>(&input);

			if(midi_input)
			{
				//const double	chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate,
				//				chunk_time_inv = 1.0/chunk_time;

				//Get range of pushed data
				//TimeRange tr = midi_input->range;//midi_input->range.end, midi_input->range.end + midi_input->range.length());
				//ChunkRange cr((c_time)std::round(tr.start*chunk_time_inv), (c_time)std::round(tr.end*chunk_time_inv));

				//Adjust track length
				//TODO: Make option for stopping recording if it goes out of range
				//if(cr.end > length)
				//	setLength(cr.end);

				MidiEvent e = midi_input->event;

				//Convert time from global to local time
				//e.time = globalCursor.convertToLocal(e.time);
				
				//Record data
				applyEvent(midi_input->event);
				//Make forward nodes dirty
				//pushed = connectors[OUTPUTS.MIDI_ID]->setDirty(cr);
				
				pushed = true;//(unfinishedNotes.size() > 0);

				//Propogate push
				//connectors[OUTPUTS.MIDI_ID]->pushData(*midi_input);
			}
			/*
			else if(midi_event)
			{
				const double	chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate,
								chunk_time_inv = 1.0/chunk_time;

				TimeRange range = pCursor->getTimeRange();

				//Get range of pushed data
				TimeRange tr(midi_event->e.time - 1.0, midi_event->e.time + 1.0);//midi_input->range.end, midi_input->range.end + midi_input->range.length());
				ChunkRange cr((c_time)std::round(tr.start*chunk_time_inv), (c_time)std::round(tr.end*chunk_time_inv));

				applyEvents({midi_event->e}, tr, cr);
				
				pushed = true;
				
				//Propogate push
				connectors[OUTPUTS.MIDI_ID]->pushData(*midi_event);
			}
			*/
		}
		//Only accept pushed audio data if this track is recording
	}

	return pushed;
}
	


/////DYNAMIC MIDI BUFFER NODE/////

DynamicMidiBufferNode::DynamicMidiBufferNode(NodeGraph *parent_graph, Time initial_length)
	: Node(parent_graph, NType::DYNAMIC_MIDI_BUFFER, "Dynamic Midi Buffer Node", "Stores buffered midi, and automatically updates."),
		BufferNode<Time, MidiData>(initial_length, MidiData()),
		MidiBufferNode()
{ }

DynamicMidiBufferNode::~DynamicMidiBufferNode()
{

}
	
//
//bool DynamicMidiBufferNode::flushData(FlushPacket &info)
//{
//	bool flushed = false;
//
//	//info is relative to pRecordCursor already. (?)
//	/*
//	//Propogate flush
//	flushed = Node::flushData(info);
//
//	if(flushed && unfinishedNotes.size() > 0)
//	{
//		nodeLock.lockWait();
//
//		//Set end of notes to the end of this flush's range (they're still being pressed)
//		for(auto n : unfinishedNotes)
//			n->range.end = info.targetTimeRange.end;//info.targetTimeRange.end;
//		
//		nodeLock.unlock();
//		
//		//Make forward nodes dirty
//		//flushed = connectors[OUTPUTS.MIDI_ID]->setDirty(info.targetChunkRange);
//	}
//	*/
//	return flushed;
//}

 
bool DynamicMidiBufferNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	
	//Check output
	if(this_id == OUTPUTS.MIDI_ID)
	{
		MidiPullPacket *midi_output = dynamic_cast<MidiPullPacket*>(&output);
		
		if(midi_output)
		{
			Cursor *pull_cursor = midi_output->pCursor;

			if(pull_cursor)
				cursor.setTimeSamples(pull_cursor->getSampleRange().start);

			connectors[INPUTS.MIDI_ID]->pullData(*midi_output);
			
			midi_output->mods = initial_mods;
			//midi_output->notes.addNotes(data.getOrderedConstNotesInRange(midi_output->range));
			MidiSet notes = data.getNotesInRange(cursor.getTimeRange());
			midi_output->notes.insert(notes.begin(), notes.end());
			
			for(auto n : unfinishedNotes)
				n->range.end = cursor.getTimeRange().end;

			pulled = true;
			cursor.step();
		}
	}

	return pulled;
}

 
bool DynamicMidiBufferNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	/*
	//Check connector
	if(this_id == INPUTS.MIDI_ID)	//Midi Packet
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
	*/
	return pushed;
}