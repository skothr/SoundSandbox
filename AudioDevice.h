#ifndef APOLLO_AUDIO_DEVICE_H
#define APOLLO_AUDIO_DEVICE_H

#include "RtAudio.h"
#include "AStatus.h"
#include "Audio.h"

#include "Saveable.h"

#include <cinttypes>
#include <functional>
#include <vector>

struct BufferDesc
{
	//These are constant --> callback shouldnt change them
	const unsigned int	chunkSize,
						sampleRate,
						numChannels;

	//Callback should set these parameters
	unsigned int		numChunks = 1;
	std::vector<const AudioSample*> data;	//Each vector element is for a channel, and should point to an
											//	array of chunkSize AudioSamples

	BufferDesc(unsigned int chunk_size, unsigned int sample_rate, unsigned int num_channels)
		: chunkSize(chunk_size), sampleRate(sample_rate), numChannels(num_channels), data(numChannels, nullptr)
	{ }
};

typedef std::function<AStatus(BufferDesc &data, double elapsed_time)> AudioCallback;

class AudioDevice : public Saveable
{
private:
	static RtAudio	*g_audioOut;

	RtAudio			*audioOut = nullptr;
	AudioCallback	callback = nullptr;

	unsigned int	deviceId = 0,
					sampleRate = 0,
					chunkSize = 0,
					numChannels = 0;

	std::string		name = "";

	bool			playing = false;

public:
	AudioDevice();
	AudioDevice(const AudioDevDesc &ad_desc);
	virtual ~AudioDevice();

	static AStatus initDevices();
	static unsigned int numDevices();
	static AStatus printDevices();

	//Returns the current id of the device with the specified name (or -1 if it isn't currently connected)
	static int getId(std::string dev_name);

	AStatus init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func);

	static int audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device);

	void setCallback(AudioCallback callback_func);

	void play();
	void stop();
	void togglePlay();

	bool isPlaying() const;
	

protected:
	virtual void updateDesc() override;
};



#endif	//APOLLO_AUDIO_DEVICE_H