#include "RenderNode.h"

#include "AUtility.h"
#include "Waveform.h"

#include "NodeConnection.h"
#include "NodePackets.h"

#include "MidiData.h"

#define TIME_RENDER false
//#define TIME_RENDER true


/////MIDI TRACKER/////

MidiTracker::MidiTracker(int sample_rate)
{
	for(MidiIndex mi = 0; mi < NUM_MIDI_NOTES; mi++)
		info[mi] = SampleInfo(sample_rate, mi);

	activeStates.reserve(NUM_MIDI_NOTES);	//One bucket per midi index
	//events.reserve(32);
}

MidiTracker::~MidiTracker()
{
	activeStates.clear();
}



/////RENDER NODE/////
const std::vector<NodeConnectorDesc> RenderNode::nc_descs =
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_INPUT, "MIDI Input", "Input MIDI to render.", -1),
			  NodeConnectorDesc(NodeData::INFO, IOType::INFO_INPUT, "Instrument Input", "Input instrument to use in rendering.", 1),
			  NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_OUTPUT, "Audio Output", "Rendered audio data.", -1) };

std::unordered_set<RenderNode*> RenderNode::renderNodes;

RenderNode::RenderNode(NodeGraph *parent_graph, int sample_rate)
	: Node(parent_graph, NType::RENDER, "Render Node", "Renders MIDI data into actual audio data."),
		sampleRate(sample_rate), renderBuffer(AUDIO_CHUNK_SIZE, RENDER_BUFFER_SIZE, true)
{
	renderNodes.emplace(this);
	initNode();
	initBuffer();
}

/*
RenderNode::RenderNode(const RenderNDesc &rn_desc)
	: Node(*(NDesc*)&rn_desc),
		sampleRate(rn_desc.sampleRate), renderBuffer(AUDIO_CHUNK_SIZE, RENDER_BUFFER_SIZE, true)
{
	Node::init(rn_desc.connectors);
	INPUTS.MIDI_ID			= rn_desc.connectors[0];
	INPUTS.INSTRUMENT_ID	= rn_desc.connectors[1];
	OUTPUTS.AUDIO_ID		= rn_desc.connectors[2];

	initBuffer();
}
*/

RenderNode::~RenderNode()
{	
	//for(auto t : pullTrackers)
	//	AU::safeDelete(t.second);
	//pullTrackers.clear();
	
	for(auto t : pushTrackers)
		AU::safeDelete(t.second);
	pushTrackers.clear();

	renderNodes.erase(this);
}

void RenderNode::renderNodeFlush()
{
	for(auto rn : renderNodes)
	{
		bool has_events = false;

		for(auto t : rn->pushTrackers)
			has_events |= (t.second->events.size() > 0 || t.second->activeStates.size() > 0);

		if(rn->activeChunk)
		{
			if(!rn->pushedThisChunk && has_events)
				rn->flushEvents();
			else
				rn->activeChunk->loadZeros();

			rn->activeChunk = rn->renderBuffer.shiftBuffer();
		}
		
		rn->pushedThisChunk = false;
	}
}

void RenderNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	INPUTS.MIDI_ID			= ids[0];
	INPUTS.INSTRUMENT_ID	= ids[1];
	OUTPUTS.AUDIO_ID		= ids[2];
}

void RenderNode::initBuffer()
{
	//Initialize buffer
	//pushBuffer.reserve(RENDER_BUFFER_SIZE);
	//for(c_time c = 0; c < RENDER_BUFFER_SIZE; c++)
	//	pushBuffer.push_back(new AudioVelChunk(AUDIO_CHUNK_SIZE, 0));

	//Link chunks
	//for(c_time c = 0; c < pushBuffer.size(); c++)
	//{
	//	pushBuffer[c]->setPointers((c < pushBuffer.size() - 1 ? pushBuffer[c + 1] : nullptr),
	//							   (c > 0 ? pushBuffer[c - 1] : nullptr) );
	//}

	//pushBuffer[pushBuffer.size() - 1]->setSeed(0);

	//Set renderChunk
	//renderChunk = pushBuffer[0];
	//renderChunk->updateOutside();

	renderBuffer.loadZeros();
	activeChunk = renderBuffer.getActiveChunk();
	lastActiveChunk = activeChunk;
}

void RenderNode::setSampleRate(int sample_rate)
{
	sampleRate = sample_rate;
}

int RenderNode::getSampleRate()
{
	return sampleRate;
}


void RenderNode::addMidiEvents(MidiEventQueue &e_q, const MidiEventQueue &src)
{
	//(Assumes events is already sorted correctly)
	/*
	for(auto e : src)
	{
		bool found = false;
		//Event is more likely to fit near the end of the list, so go backwards
		for(int i = (int)events.size() - 1; i >= 0; i--)
		{
			if(events[i].time < e.fTime)
			{
				events.insert(events.begin() + i + 1, e);
				found = true;
				break;
			}
		}
		if(!found)
			events.insert(events.begin(), e);
	}
	*/

	//Push new events onto back of the queue
	e_q.insert(e_q.end(), src.begin(), src.end());
}

void RenderNode::pushEvent(MidiEvent e, NCID connector)
{
	/*
	MidiTracker &m_tracker = *pushTrackers[connector];
		
	m_tracker.events.push_back(e);
	*/

	
	MidiTracker &m_tracker = *pushTrackers[connector];

	m_tracker.events.push_back(e);

}

void RenderNode::addPulledEvents(const MidiSet &notes, TimeRange t_range, NCID connector)
{
	MidiTracker &m_tracker = *pullTrackers[connector];
	
	MidiEventQueue new_events;

	//TimeRange cursor_range = globalCursor.getGlobalTimeRange();
	//TimeRange	g_range = globalRange;
	
	//Add note events
	for(const auto n : notes)
	{
		//TimeRange g_range = globalCursor.convertToGlobal(n->range);

		//Add note start if note starts within rendering range (convert to global time)
		if(t_range.contains(n->range.start))
		{
			new_events.push_back(MidiEvent(n->index, MidiEventType::NOTE_ON, n->velocity, n->range.start));
			//std::cout << "SUCCESS --> " << n->range.start << ", " << n->range.end << ":\n";
			//std::cout << "\t G_NOTE --> " << g_range.start << ", " << g_range.end << "\n";
			//std::cout << "\t G_CURSOR --> " << gc_range.start << ", " << gc_range.end << "\n\n";
		}

		//TODO: Handle notes that started before renderNode started rendering

		//else if(n.end >= start && !states.keyStates[n.index].noteOn)
		//	events.push_back(MidiEvent(n.index, MidiEventType::NOTE_ON, n.velocity, n.start));

		//Add note end if note ends within rendering range (convert to global time)
		else if(n->isFinished() && t_range.contains(n->range.end))//(range.contains(n->range.end) || n->range.end == range.end))
		{
			new_events.push_back(MidiEvent(n->index, MidiEventType::NOTE_OFF, n->velocity, n->range.end));
		}
		else if(m_tracker.activeStates.find(n->index) == m_tracker.activeStates.end())
		{
			std::cout << "FAILURE --> " << n->range.start << ", " << n->range.end << ":\n";
			std::cout << "\t RANGE --> " << t_range.start << ", " << t_range.end << "\n";
		}
	}

	//Add new events (ordered by time)
	addMidiEvents(m_tracker.events, new_events);


	//std::cout << "\n-------------------\n\n";

	//m_tracker.events.insert(m_tracker.events.end(), note_queue.begin(), note_queue.end());
}

/*
void RenderNode::updateEvents(const MidiData &notes, TimeRange range, NCID connector, NCDir dir)
{
	MidiTracker &m_tracker = *(dir == NCDir::BACKWARD ? pullTrackers : pushTrackers)[connector];
	//MidiDeviceState &states = m_tracker.noteStates;
	
	MidiEventQueue new_events;
	//new_events.reserve(notes.numNotes()*2);
	
	//Add note events
	for(const auto n : notes.getOrderedConstNotes())
	{
		//std::cout << "RENDER EVENT --> (" << n->range.start << ", " << n->range.end << ") : (" << range.start << ", " << range.end << ")\n";

		//std::cout << "EVENT -->  " << n->start << ", " << range.start << "  :  " << n->end << ", " << range.end << "  -->  " << range.length() << "\n";//(n->start >= range.start - range.length()) << ", " << (n->end < range.end - range.length()) << ", " << n->isFinished() << "\n";
		//Add note start if note starts within rendering range (convert to global time)
		if(range.contains(n->range.start))
		{
			new_events.push_back(MidiEvent(n->index, MidiEventType::NOTE_ON, n->velocity, n->range.start));
			//std::cout << "NOTE ON --> ";
		}

		//TODO: Handle notes that started before renderNode started rendering

		//else if(n.end >= start && !states.keyStates[n.index].noteOn)
		//	events.push_back(MidiEvent(n.index, MidiEventType::NOTE_ON, n.velocity, n.start));

		//Add note end if note ends within rendering range (convert to global time)
		if(n->isFinished() && range.contains(n->range.end))//(range.contains(n->range.end) || n->range.end == range.end))
		{
			new_events.push_back(MidiEvent(n->index, MidiEventType::NOTE_OFF, n->velocity, n->range.end));
			//std::cout << "NOTE OFF --> ";
		}
		//std::cout << n->index << ", (" << n->range.start << ", " << n->range.end << ")\n";
		//std::cout << "\tRANGE:   (" << range.start << ", " << range.end << ")\n";
	}

	//Add new events (ordered by time)
	addMidiEvents(m_tracker.events, new_events);
}

void RenderNode::updateEvents(const MidiEventQueue &e_q, NCID connector, NCDir dir)
{
	MidiTracker &m_tracker = *(dir == NCDir::BACKWARD ? pullTrackers : pushTrackers)[connector];

	//Add note events (ordered by time)
	addMidiEvents(m_tracker.events, e_q);
}

void RenderNode::shiftBuffer()
{
	activeChunk = renderBuffer.shiftBuffer();
	//renderBuffer.getActiveChunk();

	//AudioSample new_seed = pushBuffer[pushBuffer.size() - 1]->getLastSample();
	////Rotate buffer so last chunk is first, and the rest are shifted down
	//std::rotate(pushBuffer.begin(), pushBuffer.end() - 1, pushBuffer.end());
	//
	////First element of buffer is the active one
	//renderChunk = pushBuffer[0];

	////Update pointers
	//pushBuffer[pushBuffer.size() - 1]->setPointers(nullptr, pushBuffer[pushBuffer.size() - 2]);

	//pushBuffer[0]->setPointers(pushBuffer[1], nullptr);
	//pushBuffer[1]->setNextChunk(pushBuffer[0]);
	//pushBuffer[pushBuffer.size() - 2]->setPrevChunk(pushBuffer[pushBuffer.size() - 1]);

	//pushBuffer[pushBuffer.size() - 1]->setSeed(new_seed);
}
*/


bool RenderNode::render(NoteSampleFunction &sample, AudioVelChunk *chunk, TimeRange g_range, TransferMethod method, bool flush)
{
	AudioVelChunk *output_data = chunk;

	const double	sample_rate_inv = 1.0/(double)sampleRate,
					chunk_size_inv = 1.0/(double)chunk->getSize(),
					amp_mult = ((double)AUDIO_MAX_AMPLITUDE*(1.0/127.0)),	//Scales a midi velocity to an actual amplitude

					time_length = (double)chunk->getSize()*sample_rate_inv;
	
	//const s_time	s_end = (s_time)ceil(time_length*(double)sampleRate);
	
	//Overwrite previous data
	//TODO: See if this can be avoided?
	if(method == TransferMethod::COPY)
		output_data->loadZeros();

	std::unordered_map<NCID, MidiTracker*> tracker_list = flush ? pushTrackers : pullTrackers;
	
	//Loop through each connection
	for(auto mt_iter : tracker_list)
	{
		NCID nc_id = mt_iter.first;
		MidiTracker *m_tracker = mt_iter.second;
		
		MidiEventQueue &events = m_tracker->events;
		MidiDeviceState &note_states = m_tracker->noteStates;
		std::unordered_multimap<MidiIndex, SampleState> &active_states = m_tracker->activeStates;
		std::vector<SampleModFunction> &mods = m_tracker->mods;

		//Add any new active states from events
		if(!events.empty())
		{
			MidiEvent e = events.front();

			while(e.time < g_range.end)
			{
				note_states.applyEvent(e);

				switch(e.type)
				{
				case MidiEventType::NOTE_ON:
					active_states.emplace(	e.midiIndex,
											SampleState(getFrequency(e.midiIndex), 0.0, 0.0,
														(double)e.velocity*amp_mult, TimeRange((Time)e.time, -1.0),
														NoteState::ATTACKING) );
					break;
					
				case MidiEventType::NOTE_OFF:
					//Get all notes with this midi index (iterator range)
					auto range = active_states.equal_range(e.midiIndex);
					
					//Turn each note off
					for(auto iter = range.first; iter != range.second; iter++)
						iter->second.noteOff(e.time);

					break;
				}
			
				events.pop_front();

				if(!events.empty())
					e = events.front();
				else
					break;
				
				//rel_t = e.fTime - HRes_Clock::getGlobalTime() + t_range.start;
			}
		}
		//Notes that were inactivated on this sample (smooth tracker completed)
		std::vector<StateMultiMap::iterator> inactivatedNotes;
		inactivatedNotes.reserve(active_states.size());

		bool add = (method == TransferMethod::ADD);

		//Loop through each active (non-zero amplitude) note
		for(auto an_iter = active_states.begin(); an_iter != active_states.end(); an_iter++)
		{
			const MidiIndex m_index = an_iter->first;

			//Structures for this note
			SampleState &s_state = an_iter->second;
			SampleInfo &s_info = m_tracker->info[m_index];

			//When the note ends (positive if known)
			const Time note_end = s_state.range.end;
			//Time value at the current sample (seconds)
			Time t = ((s_state.started || s_state.range.start < g_range.start) ? g_range.start : s_state.range.start);
			
			//Loop through each sample
			s_time	s = (s_time)floor((t - g_range.start)*(Time)sampleRate),
					c_offset = s % AUDIO_CHUNK_SIZE;
			c_time	c = s*chunk_size_inv;

			AudioVelChunk *chunk = output_data;
			std::vector<AudioVelSample> *chunk_data = chunk->getDataRef();

			chunk->seed = (add ? chunk->seed : 0) + s_state.lastSample;

			for(; s < chunk->getSize(); s++)
			{
				//Switch to releasing if note ends on this sample
				if(note_end > 0.0 && t >= note_end && s_state.state < NoteState::RELEASING)
					s_state.noteReleased();

				//Apply mods (m) to info (in order)
				for(auto m : mods)
					m(s_state, s_info, t);

				//Sample value --> returns whether the note is finished
				if(sample(s_state, s_info, chunk_data->at(c_offset), add))
				{
					//Deactivate note and break
					inactivatedNotes.push_back(an_iter);
					break;
				}

				//Increment other stuff
				bool chunk_wrap = (++c_offset >= AUDIO_CHUNK_SIZE);
				if(chunk_wrap)
				{
					//chunk->updateChunkStep();
					c_offset = 0;
					c++;
				}
				t += sample_rate_inv;
			}
			add = true;
		}

		//Remove any inactivated notes
		for(auto iter : inactivatedNotes)
			active_states.erase(iter);
	}
	
	output_data->updateChunkStep();

	return true;
}

/*
bool RenderNode::render(NoteSampleFunction &sample, AudioVelChunk **chunk, const TimeRange &t_range, const TimeRange &g_range, const ChunkRange &c_range, TransferMethod method)
{
	//AStatus status;
	AudioVelChunk **output_data = chunk;//&activeChunk;//&pRenderChunk;
	
	//renderChunk->setSeed(pushBuffer[1]->getLastSample());

	const double	sample_rate_inv = 1.0/(double)sampleRate,
					chunk_size_inv = 1.0/(double)AUDIO_CHUNK_SIZE,
					chunk_time = (double)AUDIO_CHUNK_SIZE*sample_rate_inv,	//Length of a chunk (seconds)
					amp_mult = ((double)AUDIO_MAX_AMPLITUDE*(1.0/127.0)),	//Scales a midi velocity to an actual amplitude

					time_length = t_range.length();

	const s_time	s_end = (s_time)ceil(time_length*(double)sampleRate);
	const c_time	num_chunks = c_range.length();

	std::unordered_map<NCID, MidiTracker*> &trackers = (dir == NCDir::BACKWARD ? pullTrackers : pushTrackers);
	
	if(method == TransferMethod::COPY)
	{
		for(c_time c = 0; c < num_chunks; c++)
			output_data[c]->loadZeros();
	}

	//Loop through each connection
	for(auto mt_iter : trackers)
	{
		NCID nc_id = mt_iter.first;
		MidiTracker *m_tracker = mt_iter.second;
		
		MidiEventQueue &events = m_tracker->events;
		MidiDeviceState &note_states = m_tracker->noteStates;
		std::unordered_multimap<MidiIndex, SampleState> &active_states = m_tracker->activeStates;
		std::vector<SampleModFunction> &mods = m_tracker->mods;
		
		//static int	num_on = 0;

		//Add any new active states from events
		unsigned int j = 0;

		if(!events.empty())
		{

			MidiEvent e = events.front();
			//Time rel_t = e.fTime - g_range.start + t_range.start;
			while(e.fTime < g_range.end)
			{
				note_states.applyEvent(e);

				switch(e.type)
				{
				case MidiEventType::NOTE_ON:
					active_states.emplace(	e.midiIndex,
											SampleState(getFrequency(e.midiIndex), 0.0, 0.0,
														(double)e.velocity*amp_mult, TimeRange(e.fTime, -1.0),
														NoteState::ATTACKING) );
					
					//num_on++;
					//std::cout << "NOTE ON (" << num_on << ")\t-->\t" << e.midiIndex << ", " << e_t << ", " << t_range.start << "\n";
					
					break;
					
				case MidiEventType::NOTE_OFF:
					//Get all notes with this midi index (iterator range)
					auto range = active_states.equal_range(e.midiIndex);
					
					//num_on--;
					//std::cout << "NOTE OFF (" << num_on << ")\t-->\t" << e.midiIndex << ", " << e_t << ", " << t_range.start << "\n";
					
					//Turn each note off
					for(auto iter = range.first; iter != range.second; iter++)
						iter->second.noteOff(e.fTime);

					break;
				}
			
				events.pop_front();

				if(!events.empty())
					e = events.front();
				else
					break;
				
				//rel_t = e.fTime - HRes_Clock::getGlobalTime() + t_range.start;
			}
		}
		//Notes that were inactivated on this sample (smooth tracker completed)
		std::vector<StateMultiMap::iterator> inactivatedNotes;
		inactivatedNotes.reserve(active_states.size());

		bool add = (method == TransferMethod::ADD);

		//Loop through each active (non-zero amplitude) note
		for(auto an_iter = active_states.begin(); an_iter != active_states.end(); an_iter++)
		{
			const MidiIndex m_index = an_iter->first;

			//Structures for this note
			SampleState &s_state = an_iter->second;
			SampleInfo &s_info = m_tracker->info[m_index];

			//When the note ends (positive if known)
			const Time note_end = s_state.range.end;
			//Time value at the current sample (seconds)
			Time t = ((s_state.started || s_state.range.start < g_range.start) ? g_range.start : s_state.range.start);
			
			//Loop through each sample
			s_time	s = (s_time)floor((t - g_range.start)*(Time)sampleRate),
					c_offset = s % AUDIO_CHUNK_SIZE;
			c_time	c = s*chunk_size_inv;

			AudioVelChunk *chunk = output_data[c];
			std::vector<AudioVelSample> *chunk_data = chunk->getDataRef();

			chunk->seed = (add ? chunk->seed : 0) + s_state.lastSample;

			for(; s < s_end && c < num_chunks; s++)
			{
				//Switch to releasing if note ends on this sample
				if(note_end > 0.0 && t >= note_end && s_state.state < NoteState::RELEASING)
					s_state.noteReleased();

				//Apply mods (m) to info (in order)
				for(auto m : mods)
					m(s_state, s_info, t);

				//Sample value --> returns whether the note is finished
				if(sample(s_state, s_info, chunk_data->at(c_offset), add))
				{
					//Deactivate note and break
					inactivatedNotes.push_back(an_iter);
					break;
				}

				//Increment other stuff
				bool chunk_wrap = (++c_offset >= AUDIO_CHUNK_SIZE);
				if(chunk_wrap)
				{
					//chunk->updateChunkStep();
					c_offset = 0;
					c++;
				}
				t += sample_rate_inv;
			}
			add = true;
		}

		//Remove any inactivated notes
		for(auto iter : inactivatedNotes)
			active_states.erase(iter);
	}
	
	for(c_time c = 0; c < num_chunks; c++)
		output_data[c]->updateChunkStep();
	
	//Update data
	//for(unsigned int c = 0; c < num_chunks; c++)
	//	output_data[c]->updateAll();
	
	//for(unsigned int c = 0; c < num_chunks; c++)
	//{
	//	for(unsigned int s = 0; s < AUDIO_CHUNK_SIZE; s++)
	//		output_data[c]->setSample(s, ceil(output_data[c]->getSample(s))); 
	//}




	
	//bool add_outer = (method == TransferMethod::ADD);
	//
	////For each connection
	//for(auto mt_iter : trackers)
	//{
	//	NCID nc_id = mt_iter.first;
	//	MidiTracker *m_tracker = mt_iter.second;
	//	
	//	MidiEventList &events = m_tracker->events;
	//	MidiDeviceState &note_states = m_tracker->noteStates;
	//	std::unordered_multimap<MidiIndex, SampleState> &active_states = m_tracker->activeStates;
	//	std::vector<SampleModFunction> &mods = m_tracker->mods;

	//	//Add any new active states from events
	//	unsigned int j = 0;
	//	for(auto e : events)
	//	{
	//		if(e.time < end_time)
	//		{
	//			note_states.applyEvent(e);
	//			MidiIndex m_index = e.midiIndex;

	//			switch(e.type)
	//			{
	//			case MidiEventType::NOTE_ON:
	//				//Add SampleState of new note to activeStates
	//				active_states.emplace(	m_index,
	//										SampleState(getFrequency(m_index), 0.0, 0.0,
	//													(double)e.velocity*amp_mult, TimeRange(e.time, -1.0),
	//													NoteState::ATTACKING) );
	//				break;
	//				
	//			case MidiEventType::NOTE_OFF:
	//				//Get all notes with this midi index (iterator range)
	//				auto range = active_states.equal_range(m_index);

	//				//Set each note's off time
	//				for(auto iter = range.first; iter != range.second; iter++)
	//					iter->second.noteOff(e.time);

	//				break;
	//			}

	//			j++;
	//		}
	//		else
	//			break;
	//	}
	//	//Remove events that have been handled
	//	events.erase(events.begin(), events.begin() + j);
	//	
	//	//Notes that were inactivated on this sample (smooth tracker completed)
	//	std::vector<StateMultiMap::iterator> inactivated_notes;
	//	inactivated_notes.reserve(active_states.size());

	//	double	t = start_time;
	//	c_time	chunk = 0;
	//	s_time	c_offset = 0;

	//	//For each sample (s)
	//	for(register s_time s = 0; s < num_samples; s++)
	//	{
	//		AudioSample &p_out_s = output_data[chunk]->data[c_offset];
	//		double out_s = add_outer*p_out_s;

	//		bool add_inner = add_outer;
	//		
	//		for(auto ss_iter = active_states.begin(); ss_iter != active_states.end(); ss_iter++)
	//		{
	//			SampleState &s_state = ss_iter->second;

	//			if(s_state.state != NoteState::DONE)
	//			{
	//				SampleInfo &s_info = m_tracker->info[ss_iter->first];

	//				//Switch to releasing if note ends on this sample
	//				s_state.state = (	(t < s_state.range.end || s_state.range.end <= 0.0) ?
	//									s_state.state : NoteState::RELEASING );

	//				//Apply mods (m) to info (in order)
	//				for(auto m : mods)
	//					m(s_state, s_info, t);

	//				//Sample value
	//				if(sample(s_state, s_info, out_s, add_inner))
	//					//Note completely finished
	//					inactivated_notes.push_back(ss_iter);

	//				add_inner = true;
	//			}
	//		}

	//		//Set output data
	//		p_out_s = (AudioSample)((out_s < AUDIO_MAX_AMPLITUDE) ? out_s : AUDIO_MAX_AMPLITUDE);

	//		//Increment other stuff
	//		bool chunk_wrap = (++c_offset >= AUDIO_CHUNK_SIZE);
	//		c_offset *= !chunk_wrap;
	//		chunk += chunk_wrap;
	//		
	//		t += sample_rate_inv;
	//	}
	//	
	//	//Remove notes that are finished
	//	for(auto iter : inactivated_notes)
	//		active_states.erase(iter);

	//	add_outer = true;
	//}
	










	//
	////(Definitely slowest one)//
	////For each connection
	//for(auto mt_iter : trackers)
	//{
	//	NCID nc_id = mt_iter.first;
	//	MidiTracker *m_tracker = mt_iter.second;
	//	
	//	MidiEventList &events = m_tracker->events;
	//	MidiDeviceState &note_states = m_tracker->noteStates;
	//	std::unordered_multimap<MidiIndex, SampleState> &activeStates = m_tracker->activeStates;
	//	std::vector<SampleModFunction> &mods = m_tracker->mods;
	//	
	//	c_time chunk = 0;
	//	s_time c_offset = 0;
	//	double	t = start_time;
	//	//For each sample (s)
	//	for(s_time s = 0; s < num_samples; s++)
	//	{
	//		bool add = (method == TransferMethod::ADD);
	//		AudioSample &p_out_s = output_data[chunk]->data[c_offset];
	//		double out_s = 0.0;
	//		
	//		//Apply each event that occurred since the last sample to note state
	//		unsigned int j = 0;
	//		for(auto e : events)
	//		{
	//			if(e.time > t)
	//				break;
	//			else
	//			{
	//				note_states.applyEvent(e);

	//				switch(e.type)
	//				{
	//				case MidiEventType::NOTE_ON:
	//					//Add SampleState of new note to activeStates
	//					activeStates.emplace(e.midiIndex, SampleState(	getFrequency(e.midiIndex), 0.0, 0.0,
	//																	(double)e.velocity*amp_mult,
	//																	NoteState::ATTACKING) );
	//					
	//					//std::cout << "VELOCITY --> " << (double)e.velocity*amp_mult << "\n";

	//					break;
	//					
	//				case MidiEventType::NOTE_OFF:
	//					//Get all notes with this midi index (iterator range)
	//					auto range = activeStates.equal_range(e.midiIndex);

	//					//Make sure each one is releasing (since note is now off)
	//					for(auto iter = range.first; iter != range.second; iter++)
	//						iter->second.noteOff();

	//					break;
	//				}

	//				j++;
	//			}
	//		}
	//		//Remove events that have been handled
	//		events.erase(events.begin(), events.begin() + j);
	//	
	//		//Notes that were inactivated on this sample (smooth tracker completed)
	//		std::vector<StateMultiMap::iterator> inactivatedNotes;

	//		//For each active note
	//		for(auto ss_iter = activeStates.begin(); ss_iter != activeStates.end(); ss_iter++)
	//		{
	//			MidiIndex m_index = ss_iter->first;

	//			//Continue rendering note
	//			//MidiNoteState *key_state = &note_states.keyStates[m_index];
	//			SampleInfo &si = m_tracker->info[m_index];
	//			SampleState &ss = ss_iter->second;

	//			//Apply mods (m) to info (in order)
	//			for(auto m : mods)
	//				m(ss, si, t);

	//			//Sample value
	//			if(sample(ss, si, out_s, true))
	//				//Note completely finished
	//				inactivatedNotes.push_back(ss_iter);
	//		}

	//		for(auto iter : inactivatedNotes)
	//			activeStates.erase(iter);
	//	
	//		p_out_s = p_out_s*((AudioSample)add) + (AudioSample)out_s;
	//		add = true;
	//		
	//		//Update 
	//		t += sample_rate_inv;

	//		bool chunk_wrap = (++c_offset >= AUDIO_CHUNK_SIZE);
	//		c_offset *= !chunk_wrap;
	//		chunk += chunk_wrap;
	//	}
	//}
	//
	
	return true;
}
*/


bool RenderNode::renderUpTo(NoteSampleFunction &sample, Time g_time)
{
	if(g_time <= renderTime)
		return false;

	AudioVelChunk	*chunk = activeChunk;

	const double	sample_rate_inv = 1.0/(double)sampleRate,
					chunk_size_inv = 1.0/(double)chunk->getSize(),
					amp_mult = ((double)AUDIO_MAX_AMPLITUDE*(1.0/127.0));	//Scales a midi velocity to an actual amplitude

	const Time	chunk_length = (Time)chunk->getSize()*sample_rate_inv;

	//Clamp range within current active chunk
	TimeRange	t_range(renderTime,  min(g_time, chunkStart + chunk_length)),
				rel_t_range(t_range.start - chunkStart, t_range.end - chunkStart);
	
	SampleRange	s_range((s_time)floor(rel_t_range.start*(Time)sampleRate),	//renderSample
						(s_time)floor(rel_t_range.end*(Time)sampleRate));

	bool chunk_overlap = (g_time >= chunkStart + chunk_length);

	//Zero the render range
	//for(s_time s = s_range.start; s < s_range.end; s++)
	//	(*chunk)[s] = 0;
	
	bool add = true;
	
	//Loop through each pushed connection
	for(auto mt_iter : pushTrackers)
	{
		NCID nc_id = mt_iter.first;
		MidiTracker *m_tracker = mt_iter.second;
		
		MidiEventQueue &events = m_tracker->events;
		MidiDeviceState &note_states = m_tracker->noteStates;
		std::unordered_multimap<MidiIndex, SampleState> &active_states = m_tracker->activeStates;
		std::vector<SampleModFunction> &mods = m_tracker->mods;

		//Time t = t_range.start;

		//Add any new active states from events
		if(!events.empty())
		{
			MidiEvent e = events.front();

			while(e.time < g_time)
			{
				note_states.applyEvent(e);

				switch(e.type)
				{
				case MidiEventType::NOTE_ON:
					active_states.emplace(	e.midiIndex,
											SampleState(getFrequency(e.midiIndex), 0.0, 0.0,
														(double)e.velocity*amp_mult, TimeRange((Time)e.time, -1.0),
														NoteState::ATTACKING) );
					break;
					
				case MidiEventType::NOTE_OFF:
					//Get all notes with this midi index (iterator range)
					auto range = active_states.equal_range(e.midiIndex);
					
					//Turn each note off
					for(auto iter = range.first; iter != range.second; iter++)
						iter->second.noteOff(e.time);

					break;
				}
			
				events.pop_front();

				if(!events.empty())
					e = events.front();
				else
					break;
				
				//rel_t = e.fTime - HRes_Clock::getGlobalTime() + t_range.start;
			}
		}

		//Notes that were inactivated on this sample (smooth tracker completed)
		std::vector<StateMultiMap::iterator> inactivatedNotes;
		inactivatedNotes.reserve(active_states.size());

		//Loop through each active (non-zero amplitude) note
		for(auto an_iter = active_states.begin(); an_iter != active_states.end(); an_iter++)
		{
			zeroChunk = false;
			const MidiIndex m_index = an_iter->first;

			//Structures for this note
			SampleState &s_state = an_iter->second;
			SampleInfo &s_info = m_tracker->info[m_index];

			//When the note ends (positive if known)
			const Time note_end = s_state.range.end;
			//Time value at the current sample (seconds)
			Time t = max(s_state.range.start, t_range.start);
			s_range.start = (s_time)floor((t - chunkStart)*(Time)sampleRate);

			for(s_time s = s_range.start; s < s_range.end; s++)
			{
				//Switch to releasing if note ends on this sample
				if(note_end > 0.0 && t >= note_end && s_state.state < NoteState::RELEASING)
					s_state.noteReleased();

				//Apply mods (m) to info (in order)
				for(auto m : mods)
					m(s_state, s_info, t);

				//Sample value --> returns whether the note is finished
				if(sample(s_state, s_info, (*chunk)[s], add))
				{
					//Deactivate note and break
					inactivatedNotes.push_back(an_iter);
					break;
				}

				t += sample_rate_inv;
			}
			add = true;
		}

		//Remove any inactivated notes
		for(auto iter : inactivatedNotes)
			active_states.erase(iter);
	}
	
	renderTime = t_range.end;
	
	if(chunk_overlap)
	{
		//Rotate chunks
		chunk->updateChunkStep();
		chunkStart = renderTime;

		lastActiveChunk = activeChunk;
		activeChunk = renderBuffer.shiftBuffer();

		//Render next chunk
		return renderUpTo(sample, g_time);
	}
	else
		return true;
}


void RenderNode::onConnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.MIDI_ID)
	{
		nodeLock.lockWait();

		//Add trackers for this connection
		pullTrackers.emplace(other_nc, new MidiTracker(sampleRate));
		pushTrackers.emplace(other_nc, new MidiTracker(sampleRate));

		nodeLock.unlock();
	}
}

void RenderNode::onDisconnect(NCID this_nc, NCID other_nc)
{
	if(this_nc == INPUTS.MIDI_ID)
	{
		nodeLock.lockWait();
		
		//Remove trackers for this connection
		AU::safeDelete(pullTrackers[other_nc]);
		pullTrackers.erase(other_nc);
		
		AU::safeDelete(pushTrackers[other_nc]);
		pushTrackers.erase(other_nc);

		nodeLock.unlock();
	}
}

//
//bool RenderNode::canPull()
//{
//	bool can_pull = getConnector(INPUTS.INSTRUMENT_ID)->canPull(0);
//	if(can_pull)
//	{
//		can_pull = false;
//
//		//Check if there are any events or active notes to push
//		nodeLock.lockWait();
//		for(auto t_iter = pullTrackers.begin(); !can_pull && t_iter != pullTrackers.end(); t_iter++)
//		{
//			can_pull = (t_iter->second->events.size() > 0 || t_iter->second->activeStates.size() > 0);
//		}
//		nodeLock.unlock();
//
//		NodeConnector *nc = connectors[INPUTS.MIDI_ID];
//
//		//Check if any input nodes have data to pull
//		nc->connectorLock.lockWait();
//		std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();
//		for(auto nconn : a_conn)//NodeConnector::CIndex i = 0; !can_pull && i < (NodeConnector::CIndex)nc->numConnections(); i++)
//		{
//			if(can_pull = NodeConnector::getNodeConnector(nconn.first)->getNode()->canPull())
//				break;
//		}
//		nc->connectorLock.unlock();
//	}
//
//	return can_pull;
//}
//bool RenderNode::canPush()
//{
//	bool can_push = false;
//	NodeConnector *nc = connectors[INPUTS.INSTRUMENT_ID];
//	
//	nc->connectorLock.lockWait();
//	for(auto ncc : nc->getConnections())
//	{
//		if(can_push |= nc->canPull(ncc.first))
//			break;
//	}
//	nc->connectorLock.unlock();
//
//	return can_push;
//}
//
//bool RenderNode::canFlush()
//{
//	bool can_flush = false;
//	NodeConnector *nc = connectors[INPUTS.INSTRUMENT_ID];
//	
//	nc->connectorLock.lockWait();
//	for(auto ncc : nc->getConnections())
//	{
//		if(can_flush |= nc->canPull(ncc.first))
//			break;
//	}
//	nc->connectorLock.unlock();
//
//	if(can_flush)
//	{
//		can_flush = false;
//
//		//Check if there are any events or active notes to push
//		nodeLock.lockWait();
//		for(auto t_iter = pushTrackers.begin(); !can_flush && t_iter != pushTrackers.end(); t_iter++)
//		{
//			can_flush = (t_iter->second->events.size() > 0 || t_iter->second->activeStates.size() > 0);
//		}
//		nodeLock.unlock();
//		
//		nc = connectors[INPUTS.MIDI_ID];
//	
//		//Check if any input nodes can flush
//		nc->connectorLock.lockWait();
//		std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();
//		for(auto nconn : a_conn)//NodeConnector::CIndex i = 0; !can_flush && i < (NodeConnector::CIndex)nc->numConnections(); i++)
//		{
//			if(can_flush |= nconn.second->fromNode->canFlush())//NodeConnector::getNodeConnector(nconn.first)->getNode()->canFlush())
//				break;
//		}
//		nc->connectorLock.unlock();
//	}
//	
//	if(!can_flush)
//	{
//		//for(auto pb : pushBuffer)
//		//	pb->loadZeros();
//	}
//
//	return can_flush;
//}

const AudioVelDataBuffer* RenderNode::getBuffer()
{
	return &renderBuffer;
}

/*
bool RenderNode::flushData(FlushPacket &info)
{
	bool flushed = false;
	
	NodeConnector	*midi_nc = connectors[INPUTS.MIDI_ID],
					*instrument_nc = connectors[INPUTS.INSTRUMENT_ID];
	
	
	//for(auto c : midi_nc->get())
	//{
	//	flushed |= (c.second->
	//	//flushed |= (NodeConnector::getNC(c.first)->getNode()->flushData(info)
	//	//			|| pushTrackers[c.first]->activeStates.size() > 0);
	//}
	
	//flushed = Node::flushData(info);

	flushed = midi_nc->flushData(info);

	for(auto t_iter : pushTrackers)
		flushed |= (t_iter.second->activeStates.size() > 0) || (t_iter.second->events.size() > 0);

	if(flushed)	//Propogate flush
	{
		//Length of time in a chunk (seconds)
		//const double chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate;
		c_time num_chunks = info.flushChunkRange.length();

		ChunkRange	cr(info.targetChunkRange);
		TimeRange	tr(info.targetTimeRange),
					gr(info.globalTimeRange);
	
		//Get waveform
		InstrumentPacket instrument;
		if(instrument_nc->pullData(instrument))	//Make sure instrument data can be pulled
		{
			//Time render function (and print results)
			static TimeTest timer(TimeUnits::MILLISECONDS, "RENDER TIMING");
			static double result = 0.0;
	
			nodeLock.lockWait();
			
			for(auto t_iter : pushTrackers)
				t_iter.second->mods.clear();

			////Render audio////
			if(TIME_RENDER)
			{
				flushed = timer.timeFunction<bool>(	std::bind(
							&RenderNode::render, this, instrument.sampleVel, &activeChunk, tr, gr, cr, TransferMethod::COPY, NCDir::FORWARD),
														32, result);
			}
			else
			{
				flushed = render(instrument.sampleVel, &activeChunk, tr, gr, cr, TransferMethod::COPY, NCDir::FORWARD);
			}

			//Clear mods
			if(flushed)
			{
				//Push rendered data
				AudioPushPacket audio_output(&activeChunk, cr, TransferMethod::COPY);
				flushed = connectors[OUTPUTS.AUDIO_ID]->pushData(audio_output);

				//std::cout << "FLUSH:  " << info.flushTimeRange.start << ", " << info.flushTimeRange.end
				//			<< "   GLOBAL:  " << info.globalTimeRange.start << ", " << info.globalTimeRange.end << "\n";
			}
	
			nodeLock.unlock();
		}
	}
	else
	{
		activeChunk->loadZeros();
	}
	
	shiftBuffer();

	return flushed;
}
*/

bool RenderNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
{
	bool pulled = false;
	
	if(this_id == OUTPUTS.AUDIO_ID)	//Audio Packet
	{
		NodeConnector	*instrument_nc	= connectors[INPUTS.INSTRUMENT_ID].get(),
						*midi_nc		= connectors[INPUTS.MIDI_ID].get();

		//Check connections
		if(midi_nc->numConnections() > 0)
		{
			AudioPullPacket *audio_output = dynamic_cast<AudioPullPacket*>(&output);

			if(audio_output)
			{
				Cursor *pull_cursor = audio_output->pCursor;

				if(pull_cursor)
				{

					audio_output->method = (audio_output->method != TransferMethod::DIRECT)
											? audio_output->method
											: TransferMethod::COPY;	//Override (no pointer to non-transient data to give)
				
					bool pulled_midi = pull_cursor->isActive();
					
					nodeLock.lockWait();
					//Propogate pull
					for(auto mt_iter : pullTrackers)
					{
						NCID nc_id = mt_iter.first;
						MidiTracker *m_tracker = mt_iter.second;

						//Pull midi data from this connection
						MidiPullPacket midi_data(pull_cursor);

						if(midi_nc->pullData(midi_data))
						{
							//Update mods for this connection
							if(m_tracker->mods.size() == 0)
								m_tracker->mods.insert(m_tracker->mods.end(), midi_data.mods.begin(), midi_data.mods.end());

							addPulledEvents(midi_data.notes, pull_cursor->getTimeRange(), nc_id);

							//updateEvents(midi_data.notes, audio_output->tRange, nc_id, NCDir::BACKWARD);
							pulled_midi |= midi_data.notes.size() > 0;
						}
					}
					nodeLock.unlock();

					//Get waveform
					InstrumentPacket instrument;

					if(pulled_midi && instrument_nc->pullData(instrument))
					{
						const double chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate;

						//Render audio to output
						//pulled = render(instrument.sampleVel, audio_output->data, audio_output->tRange, audio_output->globalTimeRange, audio_output->range, TransferMethod::COPY, NCDir::BACKWARD);

						pulled = render(instrument.sampleVel, audio_output->data, audio_output->pCursor->getTimeRange(), TransferMethod::COPY, false);

						pushedThisChunk = pulled;
					
						if(pulled)
							activeChunk = renderBuffer.shiftBuffer();

						//**audio_output->data += renderChunk;
					}
					else
					{
						pulled = false;
					}
				}
			}
		}
	}
	
	return pulled;
}

bool RenderNode::pushData(PushPacket &input, NCID this_id, NCID other_id)
{
	bool pushed = false;
	
	//Check connector
	if(this_id == INPUTS.MIDI_ID)	//Midi Packet
	{
		MidiPushPacket *midi_input = dynamic_cast<MidiPushPacket*>(&input);

		if(midi_input)
		{
			nodeLock.lockWait();

			MidiTracker *m_tracker = pushTrackers[other_id];
			//Update mods for this connection
			if(m_tracker->mods.size() == 0)
				m_tracker->mods.insert(m_tracker->mods.end(), midi_input->mods.begin(), midi_input->mods.end());
			//else already updated

			//Apply pushed events
			pushEvent(midi_input->event, other_id);

			nodeLock.unlock();
			pushed = true;
		}
	}

	return pushed;
}

void RenderNode::flushEvents()
{

	//Get waveform
	InstrumentPacket instrument;

	if(connectors[INPUTS.INSTRUMENT_ID]->pullData(instrument))
	{
		
		nodeLock.lockWait();
		//renderUpTo(instrument.sampleVel, push_range.end);
		render(instrument.sampleVel, activeChunk, globalRange, TransferMethod::COPY, true);
		
		nodeLock.unlock();

		AudioPushPacket audio_output(activeChunk);//, lastActiveChunk, chunkStart);

		connectors[OUTPUTS.AUDIO_ID]->pushData(audio_output);

		//activeChunk = renderBuffer.shiftBuffer();

		//nextPushChunk = activeChunk;
	}

}

/*
void RenderNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new RenderNDesc()));
	RenderNDesc *desc = dynamic_cast<RenderNDesc*>(objDesc);
	
	desc->sampleRate = sampleRate;
}
*/