#ifndef APOLLO_DYNAMIC_MIDI_BUFFER_NODE_H
#define APOLLO_DYNAMIC_MIDI_BUFFER_NODE_H

#include "BufferNode.h"
#include "StaticBufferNode.h"
#include "DynamicBufferNode.h"

#include "Timing.h"
#include "MIDI.h"
#include "MidiData.h"

class FlushPacket;
class PushPacket;
class PullPacket;

class MidiBufferNode : public virtual BufferNode<Time, MidiData>
{
private:
	void initNode();

protected:
	//typedef std::conditional<dynamic, DynamicBufferNode<Time, MidiData>, StaticBufferNode<Time, MidiData>>::type BaseClass;
	static const std::vector<NodeConnectorDesc> nc_descs;

	int sampleRate = 44100;		//TODO: fix this ugly shit
	
	std::vector<SampleModFunction>	initial_mods;

	virtual void onLengthChanged(Time max_time_changed) override;
	//virtual void onRecordChanged(Cursor *p_old_cursor, bool recording) override;

	//c_range is adjusted to cover all chunks that events applied to
	//void applyEvents(const MidiEventQueue &events, const TimeRange &range, ChunkRange &c_range);
	void applyEvent(MidiEvent e);
	
	std::vector<MidiNote*>			unfinishedNotes;		//Currently active(unfinished) notes (only if recording)
															// --> points to notes in data


public:
	MidiBufferNode();
	virtual ~MidiBufferNode();
	
	//Connector ids
	struct
	{
		NCID MIDI_ID = 0;
	} INPUTS;
	struct
	{
		NCID MIDI_ID = 0;
	} OUTPUTS;

	void setBuffer(const MidiData &new_data);

};

class StaticMidiBufferNode : public MidiBufferNode, public StaticBufferNode<Time, MidiData>
{
protected:

public:
	StaticMidiBufferNode(NodeGraph *parent_graph, Time initial_length = 0);
	virtual ~StaticMidiBufferNode();
	
	//virtual bool flushData(FlushPacket &info) override;

	//If output is null, passes a disposable copy of data.
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

};


class DynamicMidiBufferNode : public MidiBufferNode, public DynamicBufferNode<Time, MidiData>
{
protected:

	//std::vector<MidiNote*>			unfinishedNotes;		//Currently active(unfinished) notes (only if recording)
															// --> points to data in midiData

public:
	DynamicMidiBufferNode(NodeGraph *parent_graph, Time initial_length = 0);
	virtual ~DynamicMidiBufferNode();
	
	//virtual bool flushData(FlushPacket &info) override;

	//If output is null, passes a disposable copy of data.
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

};


#endif	//APOLLO_DYNAMIC_MIDI_BUFFER_NODE_H