#ifndef APOLLO_RENDER_NODE_H
#define APOLLO_RENDER_NODE_H

#include <vector>
#include <unordered_set>

#include "Node.h"

#include "Audio.h"
#include "AudioChunk.h"
#include "AudioData.h"
#include "AudioDataBuffer.h"

#include "Timing.h"
#include "MIDI.h"
#include "Sampling.h"
#include "MidiData.h"

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <mutex>

#define RENDER_BUFFER_SIZE 5

class Waveform;
class MidiData;

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
	~MidiTracker();
};

//A node that can render midi data into audio data.
class RenderNode : public Node
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	bool pushedThisChunk = false;

	Time	renderTime = 0.0,	//Time that this node has rendered up to (Global time)
				chunkStart = 0.0;	//Time that the current active chunk started (Global time)

protected:

	static std::unordered_set<RenderNode*> renderNodes;
	static void renderNodeFlush();

	//Buffer for building up audio to push
	AudioVelDataBuffer						renderBuffer;
	AudioVelChunk							*activeChunk = nullptr,
											*lastActiveChunk = nullptr;
	bool									zeroChunk = true;

	//Holds trackers for each connection
	std::unordered_map<NCID, MidiTracker*>	pullTrackers,	//Trackers for when pulling data
											pushTrackers;	//Trackers for when pushing data

	void initBuffer();

	//Adds the event to the given event list, keeping it in order of the time the events occurred
	void addMidiEvents(MidiEventQueue &e_q, const MidiEventQueue &src);

	//	NOT THREAD SAFE! Node should be locked before calling these functions
	//void updateEvents(const MidiData &notes, TimeRange range, NCID connector, NCDir dir);
	//void updateEvents(const MidiEventQueue &e_q, NCID connector, NCDir dir);

	void pushEvent(MidiEvent e, NCID connector);
	void addPulledEvents(const MidiSet &notes, TimeRange t_range, NCID connector);

	void shiftBuffer();

	//Renders acquired data into audio data
	//	NOT THREAD SAFE! Node should be locked before calling this function
	//	offset --> offset from beginning of track to current time (only used for pulling data)
	//bool render(NoteSampleFunction &sample, const TimeRange &t_range, const ChunkRange &c_range, AudioVelChunk **output_data, TransferMethod method, NCDir dir);
	bool render(NoteSampleFunction &sample, AudioVelChunk *chunk, TimeRange g_range, TransferMethod method, bool flush);

	//Renders pushed audio up the the given global time.
	bool renderUpTo(NoteSampleFunction &sample, Time g_time);

	//bool pushRender(NoteSampleFunction &sample, AudioVelChunk *chunk, TimeRange g_range, TransferMethod method);

	int sampleRate = 0;

public:
	RenderNode(NodeGraph *parent_graph, int sample_rate);
	//RenderNode(const RenderNDesc &rn_desc);
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

	//virtual bool flushData(FlushPacket &info) override;

	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) override;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;
	
	void flushEvents();
	
protected:
	//virtual void updateDesc() override;

	//TEMP (only for cursor stuff)
	friend class SpeakerNode;
};


#endif	//APOLLO_RENDER_NODE_H