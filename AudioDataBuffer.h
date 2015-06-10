#ifndef APOLLO_AUDIO_DATA_BUFFER_H
#define APOLLO_AUDIO_DATA_BUFFER_H

#include "Audio.h"
#include "AudioData.h"
#include "AudioChunk.h"

class AudioAmpDataBuffer : public AudioAmpData
{
protected:
	AudioAmpChunk *activeChunk = nullptr;
	bool doubleBuffered;

public:
	AudioAmpDataBuffer(s_time chunk_size, c_time buffer_size, bool double_buffer);
	virtual ~AudioAmpDataBuffer();

	//Returns the active chunk
	AudioAmpChunk* shiftBuffer();

	AudioAmpChunk* getActiveChunk();
	AudioAmpChunk* getNextChunk();
	AudioAmpChunk* getLastChunk();
};


class AudioVelDataBuffer : public AudioVelData
{
protected:
	AudioVelChunk *activeChunk = nullptr;
	bool doubleBuffered;

public:
	AudioVelDataBuffer(s_time chunk_size, c_time buffer_size, bool double_buffer);
	virtual ~AudioVelDataBuffer();
	
	//Returns the active chunk
	AudioVelChunk* shiftBuffer();

	AudioVelChunk* getActiveChunk();
	AudioVelChunk* getNextChunk();
	AudioVelChunk* getLastChunk();
};

#endif	//APOLLO_AUDIO_DATA_BUFFER_H