#include "AudioStream.h"

AudioStream::AudioStream(int num_channels, int sample_rate, Time *p_cursor)
	: sf::SoundStream(), audioTrack(nullptr), numChannels(num_channels), sampleRate(sample_rate), cursor(p_cursor)
{
	initialize(numChannels, sampleRate);
}

AudioStream::~AudioStream()
{

}

sf::Int16 samples[AUDIO_CHUNK_SIZE];

bool AudioStream::onGetData(Chunk &data)
{
	if(!audioTrack)
	{
		data.sampleCount = 0;
		return false;
	}

	data.samples = audioTrack->getChunk(cursor->getTime(), AUDIO_CHUNK_SIZE, std::vector<Modifier*>());

	if(!data.samples)
	{
		data.sampleCount = 0;
		return false;
	}
	else
	{
		data.sampleCount = AUDIO_CHUNK_SIZE;
		cursor->setTime(cursor->getTime() + AUDIO_CHUNK_SIZE);
		return true;
	}
}

void AudioStream::onSeek(sf::Time timeOffset)
{
	return;
}

void AudioStream::setTrack(AudioTrack *t)
{
	audioTrack = t;
	audioTrack->setCursor(cursor);
}

void AudioStream::setCursor(Time *p_cursor)
{
	cursor = p_cursor;
}


/*
void AudioStream::setChunk(AudioSample *chunk)
{
	for(int i = 0; i < AUDIO_CHUNK_SIZE; i++)
	{
		//chunkSamples[i] = chunk[i];
	}
	needUpdate = false;
	return;
}

bool AudioStream::needsUpdate()
{
	return needUpdate;
}
*/