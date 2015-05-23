#ifndef APOLLO_IONODES_H
#define APOLLO_IONODES_H

#include "NodeBaseTypes.h"
#include "MidiDevice.h"
#include "Instrument.h"
#include "AudioDevice.h"

#include "Audio.h"
#include "AudioChunk.h"
#include "AudioData.h"
#include "AudioDataBuffer.h"

#include <array>

class ProjectTrackDisplay;

/////INPUT/////

class InstrumentNode : public ReadNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	static WaveSynth defaultInstrument;

protected:
	Instrument	*instrument;
	//Whether the instrument needs to be deleted 
	bool		needDelete;

public:
	InstrumentNode();
	//cpy_instrument denotes whether the given instrument should be copied or just used as the given pointer
	InstrumentNode(Instrument *instrument_, bool cpy_instrument);
	InstrumentNode(const InstrumentNDesc &in_desc);
	virtual ~InstrumentNode();

	//Connector ids
	struct
	{ } INPUTS;
	struct
	{
		NCID INSTRUMENT_ID = 0;
	} OUTPUTS;

	Instrument* getInstrument();
	
	//virtual bool canPull() override;
	//virtual bool canFlush() override;

	virtual bool pullData(PullPacket &output, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};

class MidiDeviceNode : public ReadNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

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

	virtual void onUpdate(Time dt) override;

public:
	MidiDeviceNode(MidiPort port);
	MidiDeviceNode(const MidiDeviceNDesc &mdn_desc);
	virtual ~MidiDeviceNode();
	
	//Connector ids
	struct
	{ } INPUTS;
	struct
	{
		NCID MIDI_ID = 0;
	} OUTPUTS;

	MidiDevice* getDevice();
	AStatus midiCallback(MidiEvent e);
	
	virtual void onConnect(NCID this_nc, NCID other_nc) override;
	virtual void onDisconnect(NCID this_nc, NCID other_nc) override;

	//virtual bool canPull() override;
	//virtual bool canFlush() override;
	
	//Event times pushed are relative to given relative time range (e.g. relative to a track)
	virtual bool flushData(FlushPacket &info) override;
	virtual bool pullData(PullPacket &output, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};

/////OUTPUT/////

class SpeakerNode : public WriteNode
{
private:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	AudioDevice							device;	
	ProjectTrackDisplay					*trackDisp;

	//Global chunk number --> which chunk the speaker is rendering (through all time)
	long								chunkNumber = 0;

	//Buffer holding a number of chunks that have been played.
	AudioAmpDataBuffer					buffer;
	
	//AudioSample							sAmplitude = 0;		//Current amplitude of the speaker
															//	(to convert AudioVecData to AudioData)

	//Points to an accumulation of audio vel data for each chunk render
	AudioVelChunk						*activeVelChunk = nullptr;

	//Points to the chunk being rendered
	AudioAmpChunk						*activeChunk = nullptr;
	//Last to leave buffer (for upsampling buffer for drawing)
	AudioAmpChunk						*trailingChunk = nullptr;

	//Buffer of pushed data (added to pulled data)
	AudioVelDataBuffer					pushBuffer;
	//Number of chunks in pushBuffer that are active (need to be played)
	int									numPushedChunks = 0;
	
	//Clock used for timing
	HRes_Clock							clock;

	virtual void onUpdate(double dt) override;

	AStatus getData(BufferDesc &data);

	void initBuffers();

protected:
	int						numChannels = 1,
							sampleRate = 0;

	//Whether an update to data should be requested before it's pulled
	bool					requestUpdate = true;

	//Shifts each part of the buffer down to reflect a new chunk becoming active
	void shiftBuffer();
	//Same, but for pushBuffer
	void shiftPushBuffer();

	AStatus deviceCallback(BufferDesc &data, double elapsed_time);

public:
	SpeakerNode(unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks, ProjectTrackDisplay *track_display);
	SpeakerNode(const SpeakerNDesc &sn_desc);
	virtual ~SpeakerNode();
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{ } OUTPUTS;

	const AudioAmpData* getBuffer();
	//const AudioVelData* getPushBuffer() { return &pushBuffer; }
	const AudioChunk* getRenderChunk();
	const AudioChunk* getTrailingChunk();

	AudioDevice* getDevice();

	void setTrackDisplay(ProjectTrackDisplay *track_disp);

	//virtual void onConnect(NodeConnector *nc) override;
	//virtual void onDisconnect(NodeConnector *nc) override;
	
	//virtual bool canPush() override;
	
	virtual bool pushData(PushPacket &input, NCID this_id) override;
	
protected:
	virtual void updateDesc() override;
};



#endif	//APOLLO_IONODES_H