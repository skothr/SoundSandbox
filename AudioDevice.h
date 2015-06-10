#ifndef APOLLO_AUDIO_DEVICE_H
#define APOLLO_AUDIO_DEVICE_H

#include "RtAudio.h"
#include "AStatus.h"
#include "Audio.h"

#include "AudioChunk.h"

#include <cinttypes>
#include <functional>
#include <vector>

struct BufferDesc
{
	//These are constant --> callback shouldnt change them
	const unsigned int			chunkSize,
								sampleRate,
								numChannels;

	//Callback should set these parameters
	unsigned int				numChunks = 1;
	std::vector<AudioAmpChunk>	data;			//Each vector element is for a channel, and should point to an
												//	array of chunkSize AudioSamples

	BufferDesc(unsigned int chunk_size, unsigned int sample_rate, unsigned int num_channels)
		: chunkSize(chunk_size), sampleRate(sample_rate), numChannels(num_channels), data(numChannels, AudioAmpChunk(chunk_size))
	{ }
};

typedef std::function<bool(BufferDesc &data, double elapsed_time)> AudioCallback;


//Base class
class AudioDevice
{
protected:
	static RtAudio	*g_audio;

	RtAudio			*audioDev = nullptr;
	AudioCallback	callback = nullptr;

	unsigned int	deviceId = 0,
					sampleRate = 0,
					chunkSize = 0,
					numChannels = 0;

	std::string		name = "";

	bool			active = false;

public:
	AudioDevice();
	virtual ~AudioDevice();

	static bool initDevices();
	static void cleanupDevices();
	static unsigned int numDevices();
	static bool printDevices();

	//Returns the current id of the device with the specified name (or -1 if it isn't currently connected)
	static int getId(std::string dev_name);

	virtual bool init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func) = 0;

	void setCallback(AudioCallback callback_func);

	void start();
	void stop();
	void toggleActive();

	bool isActive() const;
	
};


/////////////
//Audio OUTPUT device --> Sends sounds to speakers
/////////////
class AudioOutDevice : public AudioDevice
{
protected:
	static int audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device);

public:
	AudioOutDevice();
	AudioOutDevice(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func);
	virtual ~AudioOutDevice();

	virtual bool init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func) override;
};


/////////////
//Audio INPUT device --> Gets sound from microphones
/////////////
class AudioInDevice : public AudioDevice
{
protected:
	static int audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device);

public:
	AudioInDevice();
	AudioInDevice(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func);
	virtual ~AudioInDevice();

	virtual bool init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func);
};





#endif	//APOLLO_AUDIO_DEVICE_H