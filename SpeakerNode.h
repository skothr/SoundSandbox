#ifndef APOLLO_SPEAKER_NODE_H
#define APOLLO_SPEAKER_NODE_H

#include "Node.h"
#include "NodeBaseTypes.h"
#include "AudioDevice.h"
#include "AudioDataBuffer.h"
#include "Cursor.h"
#include "ThreadedObject.h"


#define TIME_SPEAKER false
//#define TIME_SPEAKER true


class ProjectTrackDisplay;


class SpeakerNode : public OutputNode, public ThreadedObject
{
protected:
	static const std::vector<NodeConnectorDesc> nc_descs;
	void initNode();

	AudioOutDevice						device;	
	//ProjectTrackDisplay					*trackDisp;

	//Global chunk number --> which chunk the speaker is rendering (through all time)
	//long								chunkNumber = 0;

	//Buffer holding a number of chunks that have been played.
	AudioAmpDataBuffer					buffer;

	Cursor								cursor;
	
	//AudioSample							sAmplitude = 0;		//Current amplitude of the speaker
															//	(to convert AudioVecData to AudioData)

	//Points to an accumulation of audio vel data for each chunk render
	AudioVelChunk						*activeVelChunk = nullptr;

	//Points to the chunk being rendered
	AudioAmpChunk						*activeChunk = nullptr;
	//Last to leave buffer (for upsampling buffer for drawing)
	AudioAmpChunk						*trailingChunk = nullptr;

	//Pushed data (added to pulled data)
	AudioVelChunk						pushChunk;

	virtual void onUpdate(const Time &dt) override;

	bool getData(BufferDesc &data);

	void initBuffers();


	//Clock used for global timing
	HRes_Clock				clock;
	//Time				g_time = 0.0;

	Time				lambda;	//Delay between when an event is played and when it is heard.


	int						numChannels = 1,
							sampleRate = 0;

	//Whether an update to data should be requested before it's pulled
	bool					requestUpdate = true;

	bool					stopThread = false;

	//Shifts each part of the buffer down to reflect a new chunk becoming active
	void shiftBuffer();
	//Same, but for pushBuffer
	//void shiftPushBuffer();

	bool deviceCallback(BufferDesc &data, double elapsed_time);

public:
	SpeakerNode(NodeGraph *parent_graph, unsigned int device_id, unsigned int sample_rate, unsigned int buffer_chunks, ProjectTrackDisplay *track_display);
	//SpeakerNode(const SpeakerNDesc &sn_desc);
	virtual ~SpeakerNode();
	
	//Connector indices
	struct
	{
		NCID AUDIO_ID = 0;
	} INPUTS;
	struct
	{ } OUTPUTS;

	virtual Cursor* getCursor() override;

	const AudioAmpData* getBuffer();
	//const AudioVelData* getPushBuffer() { return &pushBuffer; }
	const AudioChunk* getRenderChunk();
	const AudioChunk* getTrailingChunk();

	AudioOutDevice* getDevice();

	//void setTrackDisplay(ProjectTrackDisplay *track_disp);

	//virtual void onConnect(NodeConnector *nc) override;
	//virtual void onDisconnect(NodeConnector *nc) override;
	
	//virtual bool canPush() override;
	
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) override;

	virtual void shutDownThread() override;
};



#endif	//APOLLO_SPEAKER_NODE_H