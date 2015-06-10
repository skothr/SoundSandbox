#include "AudioDevice.h"

#include <iostream>
#include <cstdlib>

/////AUDIO DEVICE/////
RtAudio *AudioDevice::g_audio = nullptr;

AudioDevice::AudioDevice()
{ }

/*
AudioDevice::AudioDevice(const AudioDevDesc &ad_desc)
{
	init(getId(ad_desc.name), ad_desc.sampleRate, ad_desc.chunkSize, ad_desc.numChannels, nullptr);
}
*/

AudioDevice::~AudioDevice()
{
	if(audioDev)
	{
		stop();
		delete audioDev;
	}
	
	audioDev = nullptr;
}


bool AudioDevice::initDevices()
{
	bool status = true;

	if(!g_audio)
	{
		g_audio = new RtAudio();
	
		if(numDevices() < 1)
		{
			std::cout << "No AudioDevices to connect to.\n";
			status = false;
		}
	}
	else
	{
		std::cout << "AudioDevice has already been initialized.\n";
		status = false;
	}

	return status;
}

void AudioDevice::cleanupDevices()
{
	if(g_audio)
		delete g_audio;
	g_audio = nullptr;
}

unsigned int AudioDevice::numDevices()
{
	return (g_audio ? g_audio->getDeviceCount() : 0);
}

bool AudioDevice::printDevices()
{
	bool status = true;

	if(g_audio)
	{
		unsigned int n_devices = numDevices();
		if(n_devices != 1)
			std::cout << "There are " << n_devices << " audio output devices available.\n";
		else
			std::cout << "There is " << n_devices << " audio output device available.\n";

		if(n_devices != 0)
			std::cout << "\n------------------------\nAudio Devices:\n";

		for(unsigned int i = 0; i < n_devices; i++)
		{
			RtAudio::DeviceInfo info = g_audio->getDeviceInfo(i);

			std::string default_specifier = (info.isDefaultOutput ? "Default Output" : "");
			default_specifier += (info.isDefaultInput && info.isDefaultOutput ? ", " : "");
			default_specifier += (info.isDefaultInput ? "Default Input" : "");
			default_specifier = (info.isDefaultInput || info.isDefaultOutput ? ("(" + default_specifier + ")") : default_specifier);
		
			if(info.probed)
			{
				//Print device info

				std::cout << "\t" << i << ":\t" << info.name << "  " << default_specifier << "\n";
				std::cout << "\t        " << "Max output channels:\t" << info.outputChannels << "\n";
				std::cout << "\t        " << "Max input channels:\t" << info.inputChannels << "\n";
				std::cout << "\t        " << "Max duplex channels:\t" << info.duplexChannels << "\n";
				std::cout << "\t        " << "Supported sample rates:\n\t\t   ";

				int counter = 0;
				for(auto sr : info.sampleRates)
				{
					std::cout << sr << ", ";
					if(++counter % 4 == 0)
						std::cout << "\n\t\t   ";
				}
				std::cout << "\n";

				std::cout << "\t        " << "Native formats:\n";
				if(info.nativeFormats & RTAUDIO_SINT8)
					std::cout << "\t\t   8-bit Signed Integers\n";
				if(info.nativeFormats & RTAUDIO_SINT16)
					std::cout << "\t\t   16-bit Signed Integers\n";
				if(info.nativeFormats & RTAUDIO_SINT24)
					std::cout << "\t\t   24-bit Signed Integers\n";
				if(info.nativeFormats & RTAUDIO_SINT32)
					std::cout << "\t\t   32-bit Signed Integers\n";
				if(info.nativeFormats & RTAUDIO_FLOAT32)
					std::cout << "\t\t   32-bit Floats\n";
				if(info.nativeFormats & RTAUDIO_FLOAT64)
					std::cout << "\t\t   64-bit Floats\n";
			}
			std::cout << "\n";
		}
	}
	else
	{
		std::cout << "AudioDevice has yet to be successfully initialized.\n";
		status = false;
	}

	return status;
}

int AudioDevice::getId(std::string dev_name)
{
	unsigned int n_devices = numDevices();

	for(unsigned int i = 0; i < n_devices; i++)
	{
		RtAudio::DeviceInfo info = g_audio->getDeviceInfo(i);
		if(info.name == dev_name)
			return i;
	}

	return -1;
}

void AudioDevice::setCallback(AudioCallback callback_func)
{
	callback = callback_func;
}

void AudioDevice::start()
{
	if(!active)
	{
		try
		{
			active = true;
			audioDev->startStream();
		}
		catch(RtAudioError &e)
		{
			e.printMessage();
			return;
		}
	}
}

void AudioDevice::stop()
{
	if(active)
	{
		try
		{
			active = false;
			audioDev->stopStream();
		}
		catch(RtAudioError &e)
		{
			e.printMessage();
			return;
		}
	}
}

void AudioDevice::toggleActive()
{
	if(active)
		stop();
	else
		start();
}

bool AudioDevice::isActive() const
{
	return active;
}




/////AUDIO OUT DEVICE/////
AudioOutDevice::AudioOutDevice()
{ }

AudioOutDevice::AudioOutDevice(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func)
	: AudioDevice()
{
	init(device_id, sample_rate, chunk_size, num_channels, callback_func);
}

AudioOutDevice::~AudioOutDevice()
{ }

int AudioOutDevice::audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device)
{
	AudioOutDevice *p_dev = (AudioOutDevice*)p_device;
	if(p_dev)
	{
		AudioSample *buffer = (AudioSample*)out_buffer;

		if(status)
			std::cout << "Stream underflow!!\n";	//??

		//Get audio data from device's callback
		bool stat = false;

		BufferDesc data(buffer_size, p_dev->sampleRate, p_dev->numChannels);
		if(p_dev->callback)
			stat = p_dev->callback(data, stream_time);

		if(stat)
		{
			//Copy data to buffer
			for(c_time c = 0; c < p_dev->numChannels; c++)
				for(s_time s = 0; s < buffer_size; s++)
					//Interlace data
					buffer[s*p_dev->numChannels + c] = data.data[c][s];

			return 0;
		}
		else
			std::cout << "AUDIO OUT DEVICE CALLBACK FAILED.\n";
	}

	return 1;
}


bool AudioOutDevice::init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func)
{
	bool status = true;

	deviceId = device_id;
	sampleRate = sample_rate;
	chunkSize = chunk_size;
	numChannels = num_channels;
	callback = callback_func;
	
	unsigned int n_devices = numDevices();

	if(n_devices > 0 && device_id < n_devices)
	{
		RtAudio::DeviceInfo info = g_audio->getDeviceInfo(deviceId);

		audioDev = new RtAudio();

		RtAudio::StreamParameters param;
		param.deviceId = deviceId;//audioOut->getDefaultOutputDevice();
		param.nChannels = numChannels;
		param.firstChannel = 0;

		try
		{
			audioDev->openStream(&param, nullptr, RTAUDIO_SINT16, sampleRate, &chunkSize, &AudioOutDevice::audioCallback, (void*)this);
		}
		catch(RtAudioError &e)
		{
			std::cout << e.getMessage() << "\n";
			status = false;
		}
	}
	else
	{
		std::cout << "The requested device ID does not exist.\n";
		status = false;
	}

	return status;
}

/////AUDIO IN DEVICE/////
AudioInDevice::AudioInDevice()
{ }

AudioInDevice::AudioInDevice(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func)
	: AudioDevice()
{
	init(device_id, sample_rate, chunk_size, num_channels, callback_func);
}

AudioInDevice::~AudioInDevice()
{ }

int AudioInDevice::audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device)
{
	AudioInDevice *p_dev = (AudioInDevice*)p_device;
	if(p_dev)
	{
		AudioSample *buffer = (AudioSample*)in_buffer;

		if(status)
			std::cout << "Stream underflow!!\n";	//??

		//Get audio data from input buffer
		bool stat = false;

		BufferDesc data(buffer_size, p_dev->sampleRate, p_dev->numChannels);

		//Copy input buffer to data
		for(c_time c = 0; c < p_dev->numChannels; c++)
			for(s_time s = 0; s < p_dev->chunkSize; s++)
				data.data[c][s] = buffer[s*p_dev->numChannels + c];

		//Callback
		if(p_dev->callback)
			stat = p_dev->callback(data, stream_time);

		if(stat)
			return 0;	//Success
		else
			std::cout << "AUDIO IN DEVICE CALLBACK FAILED.\n";
	}

	return 1;
}



bool AudioInDevice::init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func)
{
	bool status = true;

	deviceId = device_id;
	sampleRate = sample_rate;
	chunkSize = chunk_size;
	numChannels = num_channels;
	callback = callback_func;
	
	unsigned int n_devices = numDevices();

	if(n_devices > 0 && device_id < n_devices)
	{
		RtAudio::DeviceInfo info = g_audio->getDeviceInfo(deviceId);

		audioDev = new RtAudio();

		RtAudio::StreamParameters param;
		param.deviceId = deviceId;//audioOut->getDefaultOutputDevice();
		param.nChannels = numChannels;
		param.firstChannel = 0;

		try
		{
			audioDev->openStream(nullptr, &param, RTAUDIO_SINT16, sampleRate, &chunkSize, &AudioInDevice::audioCallback, (void*)this);
		}
		catch(RtAudioError &e)
		{
			std::cout << e.getMessage() << "\n";
			status = false;
		}
	}
	else
	{
		std::cout << "The requested device ID does not exist.\n";
		status = false;
	}

	return status;
}