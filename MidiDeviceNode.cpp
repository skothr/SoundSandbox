#include "MidiDeviceNode.h"

#include "NodePackets.h"
#include "NodeConnection.h"

#include "AudioChunk.h"
#include "SpeakerNode.h"

//TEST//
AudioChunk *lastActiveChunk = nullptr;

/////MIDI DEVICE NODE/////
const std::vector<NodeConnectorDesc>	MidiDeviceNode::nc_descs = 
			{ NodeConnectorDesc(NodeData::MIDI, IOType::DATA_OUTPUT, "MIDI Output", "MIDI data recieved from the device.", -1) };

//TODO: Calculate this per MidiDeviceNode and according to attached speakers.
Time MidiDeviceNode::EVENT_STAY_TIME = 3.0*(double)AUDIO_CHUNK_SIZE/44100.0;

std::unordered_set<MidiDeviceNode*> MidiDeviceNode::midiDeviceNodes;

MidiDeviceNode::MidiDeviceNode(NodeGraph *parent_graph, MidiPort port)
	: Node(parent_graph, NType::MIDI_DEVICE, "Midi Device Node", "Transmits MIDI data from a connected device.")
{
	midiDeviceNodes.emplace(this);
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
	midiDeviceNodes.erase(this);
}

void MidiDeviceNode::initNode()
{
	std::vector<NCID> ids = Node::init(nc_descs);
	OUTPUTS.MIDI_ID = ids[0];
}

void MidiDeviceNode::holdEvents()
{
	for(auto mdn : midiDeviceNodes)
	{
		mdn->nodeLock.lockWait();
		mdn->holdingEvents = true;
		mdn->nodeLock.unlock();
	}
}

void MidiDeviceNode::releaseEvents()
{
	for(auto mdn : midiDeviceNodes)
	{
		mdn->nodeLock.lockWait();
		mdn->holdingEvents = false;
		for(auto e : mdn->events)
		{
			MidiPushPacket packet(e);
			bool pushed = mdn->connectors[mdn->OUTPUTS.MIDI_ID]->pushData(packet);
		}
		mdn->events.clear();
		mdn->nodeLock.unlock();
	}
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

void MidiDeviceNode::onUpdate(const Time &dt)
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

	if(device.getState()->anyNotesOn())
	{
		for(auto c : connectors[OUTPUTS.MIDI_ID]->connections)
		{
			c.second->setActive(NCDir::FORWARD);
			c.second->setPushing(true);
		}
	}

}

void MidiDeviceNode::getActiveNotes(MidiNoteSet &output)
{
	output = activeNotes;
}

MidiDevice* MidiDeviceNode::getDevice()
{
	return &device;
}

bool MidiDeviceNode::midiCallback(MidiEvent e)
{
	bool status = true;

	/*
	Time g_time = Clock::getGlobalTime();

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
	*/

	//Push event
	//Time g_time = Clock::getGlobalTime();

	//std::cout << "EVENT:  " << e.time << ",   GLOBAL:  " << g_time << "\n";
	//e.time = SpeakerNode::clock.t;
	
	nodeLock.lockWait();
	if(holdingEvents)
		events.push_back(e);
	nodeLock.unlock();

	if(!holdingEvents)
	{
		if(lastActiveChunk)
			lastActiveChunk->NOTE_ON = true;

		MidiPushPacket packet(e);
	
		bool pushed = connectors[OUTPUTS.MIDI_ID]->pushData(packet);
	}

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

/*
bool MidiDeviceNode::flushData(FlushPacket &info)
{
	bool flushed = false;

	
	////List of events to flush
	//MidiEventQueue flush_events;
	////flush_events.reserve(events.size());

	////std::cout << "LENGTH: " << info.globalTimeRange.length() << "\n";

	////std::cout.precision(15);
	////std::cout << "MIDI DEVICE FLUSH: " << std::fixed << info.globalTimeRange.start << " --> " << std::fixed << info.globalTimeRange.end << "\n";
	////std::cout << "\tNOW --> " << std::fixed << Clock::getGlobalTime() << "\n";

	//if(!events.empty())
	//{
	//	nodeLock.lockWait();

	//	MidiEvent e = events.front();
	//	while(e.time < info.globalTimeRange.end)
	//	{
	//		double rel_e_time = e.time - info.globalTimeRange.start + info.targetTimeRange.start;

	//		if(e.time >= info.globalTimeRange.start)
	//		{
	//			e.time = rel_e_time;
	//			e.fTime = rel_e_time;
	//			flush_events.push_back(e);
	//		}
	//		else if(e.time < info.globalTimeRange.start)
	//		{
	//			e.time = rel_e_time;
	//			e.fTime = info.targetTimeRange.start;
	//			flush_events.push_back(e);
	//		}
	//		else
	//		{

	//		}

	//		events.pop_front();
	//		if(!events.empty())
	//			e = events.front();
	//		else
	//			break;
	//	}

	//	nodeLock.unlock();
	//}

	////
	//////Get all events within global range, and remove events that have expired
	//////	(search backward so erasing works, plus requested events are more likely to be at the end)
	////nodeLock.lockWait();
	////for(int i = events.size() - 1; i >= 0; i--)
	////{
	////	MidiEvent &e = events[i];

	////	double relative_e_time = info.targetTimeRange.start + (e.time - info.globalTimeRange.start);

	////	if(e.time >= info.globalTimeRange.start - 0.00002267 && e.time < info.globalTimeRange.end)//info.globalTimeRange.contains(e.time))//e.time < info.globalTimeRange.end)
	////	{
	////		//std::cout << "\tPUSHING EVENT: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
	////		//std::cout << "\tEVENT PUSHED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
	////		//std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
	////	
	////		flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));

	////		events.erase(events.begin() + i);
	////	}
	////	else if(e.time < info.globalTimeRange.start)// - EVENT_STAY_TIME)
	////	{
	////		std::cout.precision(15);
	////		std::cout << "\tEVENT F-OFFSET: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
	////		std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
	////		
	////		flush_events.push_back(MidiEvent(e.midiIndex, e.type, e.velocity, relative_e_time));
	////		flush_events[flush_events.size() - 1].fTime = info.targetTimeRange.start;

	////		events.erase(events.begin() + i);
	////	}
	////	else
	////	{
	////		std::cout.precision(15);
	////		std::cout << "\tEVENT SKIPPED: (" << e.midiIndex << ", " << e.type << " : " << std::fixed << e.time << " --> " << std::fixed << relative_e_time << ")\n";
	////		std::cout << "\t\t RANGE:  (" << std::fixed << info.globalTimeRange.start << ", " << std::fixed << info.globalTimeRange.end << ")\n";
	////	}
	////}

	//nodeLock.unlock();
	//

	//flushed = (!flush_events.empty());

	//if(flushed)
	//{
	//	//Events are in backwards order -- reverse vector
	//	//std::reverse(flush_events.begin(), flush_events.end());
	//	
	//	//Push events
	//	MidiPushPacket data(info.targetTimeRange, flush_events);
	//	flushed = connectors[OUTPUTS.MIDI_ID]->pushData(data);
	//}
	//else if(device.getState()->anyNotesOn())
	//{
	//	for(auto c : connectors[OUTPUTS.MIDI_ID]->getConnections())
	//		c.second->setActive(NCDir::FORWARD);

	//	flushed = true;
	//}
	
	return flushed;
}
*/
bool MidiDeviceNode::pullData(PullPacket &output, NCID this_id, NCID other_id)
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

/*
void MidiDeviceNode::updateDesc()
{
	Node::updateDesc();
	//objDesc = (objDesc ? objDesc : (ObjDesc*)(new MidiDeviceNDesc()));
	MidiDeviceNDesc *desc = dynamic_cast<MidiDeviceNDesc*>(objDesc);
	
	desc->device = *(MidiDevDesc*)device.getDesc();
}
*/
