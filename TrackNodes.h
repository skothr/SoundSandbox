#ifndef APOLLO_TRACK_NODE_H
#define APOLLO_TRACK_NODE_H

#include "NodeBaseTypes.h"
#include "Timing.h"
#include "MIDI.h"
#include "Filter.h"

#include "Audio.h"
#include "AudioChunk.h"
#include "AudioData.h"

#include <mutex>


class MidiTrackNode;
class ProjectTrackDisplay;
class Cursor;

//Represents a buffer for audio data.
//This class should only receive relative times and ranges
//	(time = 0 is the beginning of the track and time = length is the end, no matter where the track is in global time)
class AudioTrackNode : public TrackNode<c_time>
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

protected:
	//Data
	AudioVelData audioData;

	std::vector<ChildNodeTree> childNodes;

	//Corresponding MidiTrackNode
	MidiTrackNode *midiNode = nullptr;

	//Properties
	int sampleRate = 0;

	//Cursor *cursor;
	
	virtual void onLengthChanged(c_time max_length_change) override;

	//Updates everything within the range, regardless of update status.
	AStatus update(ChunkRange cr);

	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;

	//Returns a range that represents the indices of the first and last statuses that intersect the given range
	//Range<int> getIntersectingStatusRange(ChunkRange r);

	ProjectTrackDisplay *display = nullptr;

public:
	AudioTrackNode(int sample_rate, c_time track_length = 0);
	AudioTrackNode(const AudioTrackNDesc &atn_desc);
	virtual ~AudioTrackNode();
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{
		NCID AUDIO_ID = 0;
	} OUTPUTS;

	void rangeTest();

	MidiTrackNode* getMidiNode();
	//const StatusList* getStatus();

	const AudioVelData* getData() const;
	
	std::vector<ChildNodeTree> getChildren();
	
	//Updates everything within the range that needs updating.
	AStatus updateRange(ChunkRange cr);

	virtual void clearRange(ChunkRange r, bool compress) override;

	virtual void onUpdate(double dt) override;

	//virtual AStatus setDirty(ChunkRange r) override;
	//
	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	//void setDisplay(ProjectTrackDisplay *new_display);
	
	virtual bool pullData(PullPacket &output, NCID this_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id) override;

	virtual void update(double dt) override;
	
protected:
	virtual void updateDesc() override;
};

class MidiTrackNode : public TrackNode<Time>
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	virtual void onUpdate(double dt) override;

protected:

	//TODO: Keep track of sustain/other non-note midi stuff

	MidiData						midiData;				//Track's data
	std::vector<MidiNote*>			unfinishedNotes;		//Currently active(unfinished) notes (only if recording)
															// --> points to data in midiData
	
	//TODO: Dont hardcode sample rate!! badbadbad!!!
	int								sampleRate = 44100;

	std::vector<SampleModFunction>	initial_mods;

	virtual void onLengthChanged(double max_time_changed) override;
	virtual void onRecordChanged(Cursor *p_old_cursor, bool recording) override;

	//c_range is adjusted to cover all chunks that events applied to
	void applyEvents(const MidiEventQueue &events, const TimeRange &range, ChunkRange &c_range);
	

public:
	MidiTrackNode(c_time track_length = 0);
	MidiTrackNode(const MidiTrackNDesc &mtn_desc);
	virtual ~MidiTrackNode();
	
	//Connector ids
	struct
	{
		NCID MIDI_ID = 0;
	} INPUTS;
	struct
	{
		NCID MIDI_ID = 0;
	} OUTPUTS;

	void setBuffer(const MidiData &data);
	
	virtual void clearRange(TimeRange r, bool compress) override;

	const MidiData* getData() const;
	
	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	virtual bool flushData(FlushPacket &info) override;

	//If output is null, passes a disposable copy of data.
	virtual bool pullData(PullPacket &output, NCID this_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};

class ModTrackNode : public TrackNode<c_time>, public ModNode
{
private:
protected:
	std::vector<TrackPoint> points;

public:
	//ModTrackNode();
	ModTrackNode(c_time track_length = 0);
	virtual ~ModTrackNode() = default;
	
	//virtual AStatus pullData(NodePacket &output);	// = 0;
	//virtual AStatus pushData(NodePacket &input, NCID nc_id);	// = 0;

	void reset();
};

class AudioModTrackNode : public ModTrackNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

protected:
	AudioModFunction modify = nullptr;
	
	virtual void onLengthChanged(c_time max_time_changed) override;

public:
	AudioModTrackNode(AudioModFunction mod_function, c_time track_length = 0);
	AudioModTrackNode(const AudioModTrackNDesc &amtn_desc);
	virtual ~AudioModTrackNode() = default;
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{
		NCID AUDIO_ID = 0;
	} OUTPUTS;
	
	virtual void clearRange(ChunkRange r, bool compress) override;
	
	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	virtual bool flushData(FlushPacket &info) override;
	
	virtual bool pullData(PullPacket &output, NCID this_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};

class MidiModTrackNode : public ModTrackNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

protected:
	SampleModFunction modify = nullptr;
	
	virtual void onLengthChanged(c_time max_time_changed) override;

public:
	MidiModTrackNode(SampleModFunction mod_function, c_time track_length = 0);
	MidiModTrackNode(const MidiModTrackNDesc &mmtn_desc);
	virtual ~MidiModTrackNode() = default;
	
	//Connector indices
	struct
	{
		NCID MIDI_ID = 0;
	} INPUTS;
	struct 
	{
		NCID MIDI_ID = 0;
	} OUTPUTS;
	
	virtual void clearRange(ChunkRange r, bool compress) override;
	
	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	virtual bool flushData(FlushPacket &info) override;
	
	virtual bool pullData(PullPacket &output, NCID this_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};


#endif	//APOLLO_TRACK_NODE_H