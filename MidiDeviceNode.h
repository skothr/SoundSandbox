#ifndef APOLLO_IONODES_H
#define APOLLO_IONODES_H

#include "NodeBaseTypes.h"
#include "MidiDevice.h"

#include <array>
#include <unordered_set>

//class ProjectTrackDisplay;
//class Cursor;

class FlushPacket;
class PushPacket;
class PullPacket;


class MidiDeviceNode : public InputNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	static std::unordered_set<MidiDeviceNode*> midiDeviceNodes;
	bool holdingEvents = false;

protected:
	//Amount of time event will stay in the list of events 
	static Time		EVENT_STAY_TIME;

	MidiDevice		device;
	MidiEventQueue	events;

	//Whether each event has been handled for each connected speaker
	//TODO (?)
	//std::unordered_map<NCID, std::vector<bool>> eventsHandled;
	
	void getEvents(MidiEventQueue &e_q, Time min_time);

	//Which notes are currently active
	MidiNoteSet activeNotes;
	void getActiveNotes(MidiNoteSet &output);

	virtual void onUpdate(const Time &dt) override;

public:
	MidiDeviceNode(NodeGraph *parent_graph, MidiPort port);
	//MidiDeviceNode(const MidiDeviceNDesc &mdn_desc);
	virtual ~MidiDeviceNode();
	
	//Connector ids
	struct
	{ } INPUTS;
	struct
	{
		NCID MIDI_ID = 0;
	} OUTPUTS;

	static void holdEvents();		//Stop pushing events for a second
	static void releaseEvents();	//Flush any events that happened, and then resume pushing events as they come

	MidiDevice* getDevice();
	bool midiCallback(MidiEvent e);
	
	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;
	
	//Event times pushed are relative to given relative time range (e.g. relative to a track)
	//virtual bool flushData(FlushPacket &info) override;
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	
protected:
	//virtual void updateDesc() override;
};

/////OUTPUT/////



#endif	//APOLLO_IONODES_H