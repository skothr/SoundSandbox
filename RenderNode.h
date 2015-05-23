#ifndef APOLLO_RENDER_NODE_H
#define APOLLO_RENDER_NODE_H

#include "AStatus.h"
#include <vector>

#include "Node.h"

#include "Audio.h"
#include "AudioChunk.h"
#include "AudioData.h"
#include "AudioDataBuffer.h"

#include "Timing.h"
#include "MIDI.h"
#include "Sampling.h"

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <mutex>

#define RENDER_BUFFER_SIZE 5

class Waveform;

//A list of none/one/multiple SampleStates for each MidiIndex.
typedef std::unordered_multimap<MidiIndex, SampleState> StateMultiMap;
//A map between a MidiIndex and 
typedef std::unordered_map<MidiIndex, std::vector<MidiEvent>> MidiEventMap;

struct MidiTracker
{
private:
	static bool indexValid(MidiIndex midi_index);

public:
	StateMultiMap								activeStates;	//States for the notes that are active
	MidiDeviceState								noteStates;		//State (on/off) of each note
	MidiEventQueue								events;			//Useable midi events that have occurred
	
	std::array<SampleInfo, NUM_MIDI_NOTES>		info;			//Info for each note
	std::vector<SampleModFunction>				mods;

	MidiTracker(int sample_rate);
	~MidiTracker() = default;
};

//A node that can render midi data into audio data.
class RenderNode : public virtual Node
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

protected:
	//Buffer for building up audio to push
	AudioVelDataBuffer						renderBuffer;
	AudioVelChunk							*activeChunk = nullptr;

	//Holds trackers for each connection
	std::unordered_map<NCID, MidiTracker*>	pullTrackers,	//Trackers for when pulling data
											pushTrackers;	//Trackers for when pushing data

	void initBuffer();

	//Adds the event to the given event list, keeping it in order of the time the events occurred
	void addMidiEvents(MidiEventQueue &e_q, const MidiEventQueue &src);

	//	NOT THREAD SAFE! Node should be locked before calling these functions
	void updateEvents(const MidiData &notes, TimeRange range, NCID connector, NCDir dir);
	void updateEvents(const MidiEventQueue &e_q, NCID connector, NCDir dir);

	void shiftBuffer();

	//Renders acquired data into audio data
	//	NOT THREAD SAFE! Node should be locked before calling this function
	//	offset --> offset from beginning of track to current time (only used for pulling data)
	//bool render(NoteSampleFunction &sample, const TimeRange &t_range, const ChunkRange &c_range, AudioVelChunk **output_data, TransferMethod method, NCDir dir);
	bool render(NoteSampleFunction &sample, AudioVelChunk **chunk, const TimeRange &t_range, const ChunkRange &c_range, TransferMethod method, NCDir dir);

	int sampleRate = 0;

public:
	RenderNode(int sample_rate);
	RenderNode(const RenderNDesc &rn_desc);
	virtual ~RenderNode();
	
	//Connector ids
	struct
	{
		NCID	MIDI_ID = 0,
				INSTRUMENT_ID = 0;
	} INPUTS;
	struct
	{
		NCID	AUDIO_ID = 0;
	} OUTPUTS;

	void setSampleRate(int sample_rate);
	int getSampleRate();

	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;

	//virtual bool canPull() override;
	//virtual bool canPush() override;
	//virtual bool canFlush() override;

	const AudioVelDataBuffer* getBuffer();

	virtual bool flushData(FlushPacket &info) override;

	virtual bool pullData(PullPacket &output, NCID this_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};


#endif	//APOLLO_RENDER_NODE_H