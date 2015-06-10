#ifndef APOLLO_TIME_MAP_NODE_H
#define APOLLO_TIME_MAP_NODE_H

/*
#include "Node.h"
#include "Cursor.h"

class AudioTrackNode;
class MidiTrackNode;

//Used to relay any added or removed connections to a TimeMapDisplay
struct AudioMidiTrackPair
{
	AudioTrackNode *audio = nullptr;
	MidiTrackNode *midi = nullptr;

	AudioMidiTrackPair()
	{ }

	AudioMidiTrackPair(AudioTrackNode *AUDIO_TRACK, MidiTrackNode *MIDI_TRACK)
		: audio(AUDIO_TRACK), midi(MIDI_TRACK)
	{ }
};

typedef AudioMidiTrackPair AudioMidiPair;

//A node that maps tracks across time
class TimeMapNode : public virtual Node
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	Cursor cursor;
	bool playing = false;

	int sampleRate;

	std::unordered_map<NCID, AudioMidiPair> connectedBuffers;

	std::vector<AudioMidiPair>	addedBuffers,
								removedBuffers;

	c_time						maxBufferLength = 0;

public:
	TimeMapNode(NodeGraph *parent_graph, int sample_rate);
	//TimeMapNode(const TimeMapNDesc &tmn_desc);
	virtual ~TimeMapNode() = default;
	
	//Connector ids
	struct
	{
		NCID	AUDIO_ID = 0;
	} INPUTS;
	struct
	{
		NCID	AUDIO_ID = 0;
	} OUTPUTS;

	Cursor* getCursor();

	//virtual bool hasCursor() const override;
	//virtual bool isPlaying() override;
	void setPlaying(bool play);

	//Resets cursor to beginning (0)
	void reset();
	
	std::vector<AudioMidiPair>* getAddedBuffers();
	std::vector<AudioMidiPair>* getRemovedBuffers();
	const std::unordered_map<NCID, AudioMidiPair>* getConnectedBuffers();
	c_time getMaxBufferLength() const;
	
	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;
	
	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	//virtual bool flushData(FlushPacket &info) override;

	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;
	
protected:
	//virtual void updateDesc() override;

	friend class TimeMapDisplay;
};
*/



#endif	//APOLLO_TIME_MAP_NODE_H