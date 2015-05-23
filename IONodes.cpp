#include "IONodes.h"

#include "NodeConnection.h"
#include "ProjectTrackDisplay.h"

#include <functional>
#include "AUtility.h"
#include "windows.h"

#define TIME_SPEAKER false
//#define TIME_SPEAKER true


//TEST//
AudioChunk *lastActiveChunk = nullptr;


/////INSTRUMENT NODE/////
const std::vector<NodeConnectorDesc>	InstrumentNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::INFO, IOType::INFO_OUTPUT, "Instrument Output", "Instrument sampling data.", -1) };

WaveSynth InstrumentNode::defaultInstrument;

InstrumentNode::InstrumentNode()
	: InstrumentNode(&defaultInstrument, true)
{ }

/*
InstrumentNode::InstrumentNode(const InstrumentNDesc &in_desc)
	: Node(*dynamic_cast<const NDesc*>(&in_desc)),
		instrument(nullptr), needDelete(true)
{
	Node::init(in_desc.connectors);
	OUTPUTS.INSTRUMENT_ID = in_desc.connectors[0];

	switch(in_desc.instrument.type)
	{
	case InstrumentType::WAVESYNTH:
		instrument = new WaveSynth(*(WaveSynthDesc*)&in_desc.instrument);
		break;
	}
}
*/

InstrumentNode::InstrumentNode(Instrument *instrument_, bool cpy_instrument)
	: Node(NType::INSTRUMENT, "Instrument Node", "Holds a description of an instrument for use with MIDI data."),
		instrument(instrument_), needDelete(cpy_instrument && instrument_)
{
	initNode();

	if(cpy_instrument && instrument_)
	{
		switch(instrument_->getType())
		{
		case InstrumentType::WAVESYNTH:
			instrument = new WaveSynth();
			break;
		default:
			break;
		}
	}
	//else instrument is null --> keep null
}

InstrumentNode::~InstrumentNode()
{
	if(needDelete)
		AU::safeDelete(instrument);
}


void InstrumentNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);

	OUTPUTS.INSTRUMENT_ID = ids[0];
}

Instrument* InstrumentNode::getInstrument()
{
	return instrument;
}

//bool InstrumentNode::canPull()
//{
//	return static_cast<bool>(instrument);
//}
//
//bool InstrumentNode::canFlush()
//{
//	return false;
//}

bool InstrumentNode::pullData(PullPacket &output, NCID this_id)
{
	//Check output
	if(this_id == OUTPUTS.INSTRUMENT_ID)	
	{
		InstrumentPacket *inst_output = dynamic_cast<InstrumentPacket*>(&output);

		if(inst_output && instrument)
		{
			inst_output->sample = instrument->getSampleFunction();
			inst_output->sampleVel = instrument->getSampleVelFunction();
			return true;
		}
	}

	return false;
}

void InstrumentNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new InstrumentNDesc()));
	InstrumentNDesc *desc = dynamic_cast<InstrumentNDesc*>(objDesc);
	
	desc->instrument = *(InstrumentDesc*)instrument->getDesc();
}



/////MIDI DEVICE NODE/////
const std::vector<NodeConnectorDesc>	MidiDeviceNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "MIDI Output", "MIDI data recieved from the device.", -1) };

//TODO: Calculate this per MidiDeviceNode and according to attached speakers.
Time MidiDeviceNode::EVENT_STAY_TIME = 3.0*(double)AUDIO_CHUNK_SIZE/44100.0;

MidiDeviceNode::MidiDeviceNode(MidiPort port)
	: Node(NType::MIDI_DEVICE, "Midi Device Node", "Transmits MIDI data from a connected device.")
{
	initNode();

	device.init(port, std::bind(&MidiDeviceNode::midiCallback, this, std::placeholders::_1));
}

/*
MidiDeviceNode::MidiDeviceNode(const MidiDeviceNDesc &mdn_desc)
	: Node(*dynamic_cast<const NDesc*>(&mdn_desc)),
		device(mdn_desc.device)
{
	Node::init(mdn_desc.connectors);
	OUTPUTS.MIDI_ID = mdn_desc.connectors[0];
}
*/

MidiDeviceNode::~MidiDeviceNode()
{

}

void MidiDeviceNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	OUTPUTS.MIDI_ID = ids[0];
}

void MidiDeviceNode::getEvents(MidiEventQueue &e_q, Time min_time)
{
	e_q.clear();

	nodeLock.lockWait();

	MidiEvent e = events.front();
	//Find first midi event that matches
	while(e.time < min_time)
	{
		events.pop_front();
		events.push_back(e);
		e = events.front();
	}

	//Copy all events from min_time on into e_q
	while(e.time >= min_time)
	{
		e_q.push_back(e);
		events.push_back(e);
		events.pop_front();
		e = events.front();
	}

	nodeLock.unlock();
}

void MidiDeviceNode::onUpdate(Time dt)
{
	/*
	nodeLock.lockWait();
	bool has_notes = device.getState()->anyNotesOn();
	nodeLock.unlock();

	//Make output connections active if any notes are on (to be "sending data" (but not actually) when a note is held)
	if(has_notes)
	{
		for(auto nc : outputConnectors)
		{
			for(auto c : nc.second->getConnections())
				c.second->setActive(NCDir::FORWARD);
		}
	}
	*/
}

void MidiDeviceNode::getActiveNotes(MidiNoteSet &output)
{
	output = activeNotes;
}

MidiDevice* MidiDeviceNode::getDevice()
{
	return &device;
}

AStatus MidiDeviceNode::midiCallback(MidiEvent e)
{
	Time g_time = Clock::getGlobalTime();

	AStatus status;

	if(lastActiveChunk)
		lastActiveChunk->NOTE_ON = true;

	//std::cout << "NODE RECEIVED (" << e.midiIndex << ", " << e.type << " : " << e.time << ") at " << std::fixed << g_time << "\n";

	nodeLock.lockWait();

	//Add event
	events.push_back(e);
	//Add eventHandled for each connection
	//for(auto eh : eventsHandled)
	//	eh.second.push_back(false);

	nodeLock.unlock();

	return status;
}

void MidiDeviceNode::onConnect(NCID this_nc, NCID other_nc)
{
	/*
	NCID	this_id = this_nc->getId(),
			other_id = other_nc->getId();

	if(this_id == OUTPUTS.MIDI_ID)
	{
		nodeLock.lockWait();
		eventsHandled.emplace(other_id, std::vector<bool>(events.size(), false));
		nodeLock.unlock();
	}
	*/
}

void MidiDeviceNode::onDisconnect(NCID this_nc, NCID other_nc)
{
	/*
	NCID	this_id = this_nc->getId(),
			other_id = other_nc->getId();
	
	if(this_id == OUTPUTS.MIDI_ID)
	{
		nodeLock.lockWait();
		eventsHandled.erase(other_id);
		nodeLock.unlock();
	}
	*/
}

//bool MidiDeviceNode::canPull()
//{
//	return canFlush();//(activeNotes.size() > 0);	//<-- activeNotes for pulling?
//}
//bool MidiDeviceNode::canFlush()
//{
//	nodeLock.lockWait();
//	bool can_flush = (events.size() > 0);
//	nodeLock.unlock();
//
//	return can_flush;
//}

bool MidiDeviceNode::flushData(FlushPacket &info)
{
	bool flushed = false;

	//List of events to flush
	MidiEventQueue flush_events;
	//flush_events.reserve(events.size());

	//std::cout << "LENGTH: " << info.globalTimeRange.length() << "\n";

	//std::cout.precision(15);
	//std::cout << "MIDI DEVICE FLUSH: " << std::fixed << info.globalTimeRange.start << " --> " << std::fixed << info.globalTimeRange.end << "\n";
	//std::cout << "\tNOW --> " << std::fixed << Clock::getGlobalTime() << "\n";

	if(!events.empty())
	{
		nodeLock.lockWait();

		MidiEvent e = events.front();
		while(e.time < info.globalTimeRange.end)
		{
			double rel_e_time = e.time - info.globalTimeRange.start + info.targetTimeRange.start;

			if(e.time >= info.globalTimeRange.start)
			{
				e.time = rel_e_time;
				e.fTime = rel_e_time;
				flush_events.push_back(e);
			}
			else if(e.time < info.globalTimeRange.start)
			{
				e.time = rel_e_time;
				e.fTime = info.targetTimeRange.start;
				flush_events.push_back(e);
			}
			else
			{

			}

			events.pop_front();
			if(!events.empty())
				e = events.front();
			else
				break;
		}

		nodeLock.unlock();
	}

	/*
	//Get all events within global range, and remove events that have expired
	//	(search backward so erasing works, plus requested events are more likely to be at the end)
	nodeLock.lockWait();
	for(int i = events.size() - 1; i >= 0; i--)
	{
		MidiEvent &e = events[i];

		double relative_e_time = info.targetTimeRange.start + (e.time - info.globalTimeRange.start);

		if(e.time >= info.globalTimeRange.start - 0.00002267 && e.time < info.globalTimeRange.end)//info.globalTimeRange.contains(e.time))//e.time < info.globalTimeRange.end)
		{
			//std::cout << "\tPUSHING EVENT: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			//std::cout << "\tEVENT PUSHED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			//std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
		
			flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));

			events.erase(events.begin() + i);
		}
		else if(e.time < info.globalTimeRange.start)// - EVENT_STAY_TIME)
		{
			std::cout.precision(15);
			std::cout << "\tEVENT F-OFFSET: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
			
			flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));
			flush_events[flush_events.size() - 1].fTime = info.targetTimeRange.start;

			events.erase(events.begin() + i);
		}
		else
		{
			std::cout.precision(15);
			std::cout << "\tEVENT SKIPPED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
		}
	}

	nodeLock.unlock();
	*/

	flushed = (!flush_events.empty());

	if(flushed)
	{
		//Events are in backwards order -- reverse vector
		//std::reverse(flush_events.begin(), flush_events.end());
		
		//Push events
		MidiPushPacket data(info.targetTimeRange, flush_events);
		flushed = connectors[OUTPUTS.MIDI_ID]->pushData(data);
	}
	else if(device.getState()->anyNotesOn())
	{
		for(auto c : connectors[OUTPUTS.MIDI_ID]->getConnections())
			c.second->setActive(NCDir::FORWARD);

		flushed = true;
	}
	
	return flushed;
}

bool MidiDeviceNode::pullData(PullPacket &output, NCID this_id)
{
	bool pulled = false;

	/*
	//status.setWarning(AS::WType::NO_ACTION_TAKEN, "Data pull for this node is undefined.");
	
	//List of events to flush
	MidiEventList flush_events;
	flush_events.reserve(events.size());

	//std::cout << "LENGTH: " << info.globalTimeRange.length() << "\n";

	//std::cout.precision(15);
	//std::cout << "MIDI DEVICE FLUSH: " << std::fixed << info.globalTimeRange.start << " --> " << std::fixed << info.globalTimeRange.end << "\n";
	//std::cout << "\tNOW --> " << std::fixed << Clock::getGlobalTime() << "\n";
	
	//Get all events within global range, and remove events that have expired
	//	(search backward so erasing works, plus requested events are more likely to be at the end)
	nodeLock.lockWait();
	for(int i = events.size() - 1; i >= 0; i--)
	{
		MidiEvent &e = events[i];

		double relative_e_time = output->targetTimeRange.start + (e.time - info.globalTimeRange.start);

		if(e.time >= info.globalTimeRange.start - 0.00002267 && e.time < info.globalTimeRange.end)//info.globalTimeRange.contains(e.time))//e.time < info.globalTimeRange.end)
		{
			//std::cout << "\tPUSHING EVENT: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			//std::cout << "\tEVENT PUSHED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			//std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
		
			flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));

			events.erase(events.begin() + i);
		}
		else if(e.time < info.globalTimeRange.start)// - EVENT_STAY_TIME)
		{
			std::cout.precision(15);
			std::cout << "\tEVENT F-OFFSET: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
			
			flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));
			flush_events[flush_events.size() - 1].fTime = info.targetTimeRange.start;

			events.erase(events.begin() + i);
		}
		else
		{
			std::cout.precision(15);
			std::cout << "\tEVENT SKIPPED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
			std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
		}
	}

	nodeLock.unlock();

	//Events are in backwards order -- reverse vector
	std::reverse(flush_events.begin(), flush_events.end());
		
	//Push events
	MidiPushPacket data(info.targetTimeRange, flush_events);
	status = connectors[OUTPUTS.MIDI_ID]->pushData(data);
	*/

	return pulled;
}

void MidiDeviceNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new MidiDeviceNDesc()));
	MidiDeviceNDesc *desc = dynamic_cast<MidiDeviceNDesc*>(objDesc);
	
	desc->device = *(MidiDevDesc*)device.getDesc();
}


/////SPEAKER NODE/////
const std::vector<NodeConnectorDesc>	SpeakerNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::AUDIO, IOType::DATA_INPUT, "Audio Input", "Audio data to play.", -1) };

SpeakerNode::SpeakerNode(unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks, ProjectTrackDisplay *track_display)
	: Node(NType::SPEAKER, "Speaker Node", "Sends audio to a speaker to be played"),
		sampleRate(sample_rate), buffer(AUDIO_CHUNK_SIZE, buffer_chunks, true), pushBuffer(AUDIO_CHUNK_SIZE, 1, false),
		trackDisp(track_display)
{
	//initialize(numChannels, sampleRate);
	device.init(device_id, sampleRate, AUDIO_CHUNK_SIZE, 1, std::bind(&SpeakerNode::deviceCallback, this, std::placeholders::_1, std::placeholders::_2));
	initNode();
	
	initBuffers();

	//Start playing
	device.play();
}

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

SpeakerNode::~SpeakerNode()
{
	//AU::safeDelete(renderChunk);
	//AU::safeDelete(trailingChunk);
	AU::safeDelete(activeVelChunk);

	//for(auto chunk : buffer)
	//	AU::safeDelete(chunk);
	//buffer.clear();

	//for(auto chunk : pushBuffer)
	//	AU::safeDelete(chunk);
	//pushBuffer.clear();
}

void SpeakerNode::initBuffers()
{
	activeChunk = buffer.getActiveChunk();
	trailingChunk = buffer.getChunk(0);
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

void SpeakerNode::onUpdate(double dt)
{
	//Make sure it keeps playing
	if(!device.isPlaying())
	{
		std::cout << "SPEAKER STOPPED PLAYING\n";
		device.play();
	}
}

void SpeakerNode::shiftBuffer()
{
	buffer.shiftBuffer();
	
	activeChunk = buffer.getActiveChunk();
	trailingChunk = buffer.getChunk(0);//buffer.getNumChunks() - 1);


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

void SpeakerNode::shiftPushBuffer()
{
	pushBuffer.getChunk(pushBuffer.getNumChunks() - 1)->setStatus(DataStatus::DIRTY);
	numPushedChunks--;

	pushBuffer.shiftBuffer();

	////First chunk is now invalid, so set to overwrite data
	//pushBuffer[0]->setStatus(DataStatus::DIRTY);
	////One less pushed chunk
	//numPushedChunks--;

	////Rotate buffer so last chunk is first, and the rest are shifted down
	////std::rotate(pushBuffer.begin(), pushBuffer.end() - 1, pushBuffer.end());
}

AStatus SpeakerNode::getData(BufferDesc &data)
{
	//if(Keyboard::keyDown(Keys::K_1))
	//{
	//	std::cout	<< "------------------------------------\n"
	//				<< "------------------------------------\n";
	//}

	NodeConnector *nc = connectors[INPUTS.AUDIO_ID];
	
	const Time	chunk_time = (Time)AUDIO_CHUNK_SIZE/(Time)sampleRate;

	//if(Keyboard::keyDown(Keys::K_1))
	//	std::cout << clock.last_t << " --> " << clock.t << " : " << clock.dt << " (" << chunk_time << ", " << chunkNumber << ")\n";
	
	//Range to flush
	ChunkRange	flush_cr(chunkNumber - 1, chunkNumber),
				target_cr(flush_cr.end, flush_cr.end + 1);

	TimeRange	flush_tr(flush_cr.start*chunk_time, flush_cr.end*chunk_time),
				target_tr(target_cr.start*chunk_time, target_cr.end*chunk_time),

				flush_global_tr(clock.globalOffset + clock.last_t, clock.globalOffset + clock.t);
	
	FlushPacket flush_info(flush_tr, target_tr, flush_cr, target_cr, flush_global_tr);

	//Only flush after first chunk
	if(chunkNumber > 0)
	{
		//Flush data that has accumulated since last render
		bool flushed = nc->flushData(flush_info);
	}
	
	bool add = false;	//First copy, then add after (since activeVelChunk contains data from last trailingChunk)

	//Add pulled data (only if trackDisp is playing)
	if(trackDisp && trackDisp->isPlaying())
	{
		target_cr = trackDisp->getCursor()->getChunkRange();
		target_tr = trackDisp->getCursor()->getTimeRange();

		//AudioPullPacket audio_data(&activeVelChunk, target_cr, (add ? TransferMethod::ADD : TransferMethod::COPY), requestUpdate);
		AudioPullPacket audio_data(&activeVelChunk, target_cr, (add ? TransferMethod::ADD : TransferMethod::COPY), requestUpdate);
		audio_data.globalRange = target_cr;
		audio_data.tRange = target_tr;
		
		//trackDisp->updateCursorPos();

		add = nc->pullData(audio_data);

		trackDisp->getCursor()->step();
		//trackDisp->updateCursorPos();
	}

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
	if(numPushedChunks > 0)
	{
		AudioVelChunk *pb_chunk = pushBuffer.getChunk(pushBuffer.getNumChunks() - 1);

		add ? (*activeVelChunk += *pb_chunk) : (*activeVelChunk = *pb_chunk);
		add = true;
		shiftPushBuffer();
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
	data.data[0] = activeChunk->getData()->data();
	
	shiftBuffer();
	
	return AS::A_SUCCESS;
}

//Called when speaker needs more audio data
AStatus SpeakerNode::deviceCallback(BufferDesc &data, double elapsed_time)
{
	AStatus status;

	nodeLock.lockWait();

	if(!clock.initialized)
	{
		//Initialize clock -- not on global time (t = 0 is start time of first chunk)
		clock.reset(false);
		chunkNumber = 0;
	}
	else
	{
		//Length of chunk to be rendered (seconds)
		const double	chunk_time = (double)AUDIO_CHUNK_SIZE/(double)sampleRate;
		//Time when rendering the next chunk can begin (1/2 chunk length from last chunk to this chunk)
		const Time		next_chunk_t = ((Time)chunkNumber)*chunk_time;

		////Wait until next rendering time
		clock.waitUntil(next_chunk_t);


		//UPDATE TIME --> clock.t is the beginning of the chunk about to be rendered
		clock.nextTimeFrame();
		chunkNumber++;

		//if(Keyboard::keyDown(Keys::K_2))
		//	std::cout << "\n\nEntering range (" << clock.globalOffset + clock.last_t << ", " << clock.globalOffset + clock.t << ")\n";
	}

	static TimeTest timer(TimeUnits::MICROSECONDS, "SPEAKER CHUNK TIMING");
	static double result = 0.0;

	if(TIME_SPEAKER)
		status = timer.timeFunction<AStatus>(std::bind(&SpeakerNode::getData, this, data), 256, result);
	else
		status = getData(data);

	//Sleep(100);

	nodeLock.unlock();

	return status;
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

AudioDevice* SpeakerNode::getDevice()
{
	return &device;
}

void SpeakerNode::setTrackDisplay(ProjectTrackDisplay *track_disp)
{
	trackDisp = track_disp;
}

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

bool SpeakerNode::pushData(PushPacket &input, NCID this_id)
{
	bool pushed = false;

	//Check connector
	if(this_id == INPUTS.AUDIO_ID)	//Audio Packet
	{
		AudioPushPacket *audio_input = dynamic_cast<AudioPushPacket*>(&input);
		
		if(audio_input)
		{
			nodeLock.lockWait();

			c_time num_chunks = audio_input->range.length();
			
			if(num_chunks > pushBuffer.getNumChunks())
			{
				pushBuffer.resize(num_chunks);
			}

			numPushedChunks = max(num_chunks, numPushedChunks);

			//Copy/add each pushed chunk to pushBuffer
			for(c_time c = 0; c < num_chunks; c++)
			{
				AudioVelChunk *chunk = pushBuffer.getChunk(c);
				if(chunk->getStatus() == DataStatus::DIRTY)
				{
					*chunk = *audio_input->data[c];
					chunk->seed = audio_input->data[c]->seed;
					chunk->setStatus(DataStatus::CLEAN);
				}
				else
				{
					*chunk += *audio_input->data[c];
					chunk->seed += audio_input->data[c]->seed;
				}

				chunk->updateChunkStep();
			}

			nodeLock.unlock();

			pushed = true;
		}
	}

	return pushed;
}

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