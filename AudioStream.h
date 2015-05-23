#ifndef APOLLO_AUDIO_STREAM
#define APOLLO_AUDIO_STREAM

#include <SFML/Audio.hpp>
#include <stdint.h>

#include "Track.h"
#include "Timing.h"
#include "Audio.h"

class AudioStream : public sf::SoundStream
{
private:
	int numChannels,
		sampleRate;

	Time *cursor;

	//bool needUpdate;

	AudioTrack *audioTrack;

	bool onGetData(Chunk &data);
	void onSeek(sf::Time timeOffset);

public:
	AudioStream(int num_channels, int sample_rate, Time *p_cursor);

	~AudioStream();

	void setTrack(AudioTrack *t);
	void setCursor(Time *p_cursor);
	//void setChunk(AudioSample *chunk);

	//bool needsUpdate();
};


#endif	//APOLLO_AUDIO_STREAM