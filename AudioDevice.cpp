#include "AudioDevice.h"

#include <iostream>
#include <cstdlib>

/////AUDIO DEVICE/////
RtAudio *AudioDevice::g_audioOut = nullptr;

AudioDevice::AudioDevice()
{ }

AudioDevice::AudioDevice(const AudioDevDesc &ad_desc)
{
	init(getId(ad_desc.name), ad_desc.sampleRate, ad_desc.chunkSize, ad_desc.numChannels, nullptr);
}

AudioDevice::~AudioDevice()
{
	try
	{
		audioOut->stopStream();
	}
	catch(RtAudioError &e)
	{
		e.printMessage();
	}

	if(audioOut->isStreamOpen())
		audioOut->closeStream();

	delete audioOut;
}


AStatus AudioDevice::initDevices()
{
	AStatus status;

	if(!g_audioOut)
	{
		g_audioOut = new RtAudio();
	
		if(numDevices() < 1)
		{
			std::cout << "NO AUDIO DEVICES!!\n";
			status.setWarning(AS::WType::GENERAL, "No AudioDevices to connect to.");
		}
	}
	else
		status.setWarning(AS::WType::ALREADY_INITIALIZED, "AudioDevice has already been initialized.");

	return status;
}

unsigned int AudioDevice::numDevices()
{
	return (g_audioOut ? g_audioOut->getDeviceCount() : 0);
}

AStatus AudioDevice::printDevices()
{
	AStatus status;

	if(g_audioOut)
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
			RtAudio::DeviceInfo info = g_audioOut->getDeviceInfo(i);

			std::string default_specifier = (info.isDefaultOutput ? "Default Output" : "");
			default_specifier += (info.isDefaultInput && info.isDefaultOutput ? ", " : "");
			default_specifier += (info.isDefaultInput ? "Default Input" : "");
			default_specifier = (info.isDefaultInput || info.isDefaultOutput ? ("(" + default_specifier + ")") : default_specifier);
		
			if(info.probed)
			{
				//Print device info

				std::cout << "\t" << i + 1 << ":\t" << info.name << "  " << default_specifier << "\n";
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
		status.setError(AS::ErrorType::GENERAL, "AudioDevice has yet to be successfully initialized.");

	return status;
}

int AudioDevice::getId(std::string dev_name)
{
	unsigned int n_devices = numDevices();

	for(unsigned int i = 0; i < n_devices; i++)
	{
		RtAudio::DeviceInfo info = g_audioOut->getDeviceInfo(i);
		if(info.name == dev_name)
			return i;
	}

	return -1;
}

AStatus AudioDevice::init(unsigned int device_id, unsigned int sample_rate, unsigned int chunk_size, unsigned int num_channels, AudioCallback callback_func)
{
	AStatus status;

	deviceId = device_id;
	sampleRate = sample_rate;
	chunkSize = chunk_size;
	numChannels = num_channels;
	callback = callback_func;
	
	unsigned int n_devices = numDevices();

	if(n_devices > 0 && device_id < n_devices)
	{
		RtAudio::DeviceInfo info = g_audioOut->getDeviceInfo(deviceId);

		audioOut = new RtAudio();

		RtAudio::StreamParameters param;
		param.deviceId = deviceId;//audioOut->getDefaultOutputDevice();
		param.nChannels = numChannels;
		param.firstChannel = 0;

		try
		{
			audioOut->openStream(&param, nullptr, RTAUDIO_SINT16, sampleRate, &chunkSize, &AudioDevice::audioCallback, (void*)this);
		}
		catch(RtAudioError &e)
		{
			status.setError(AS::ErrorType::GENERAL, e.getMessage());
		}
	}
	else
		status.setError(AS::ErrorType::INDEX_OUT_OF_BOUNDS, "The requested device ID does not exist.");

	return status;
}

int AudioDevice::audioCallback(void *out_buffer, void *in_buffer, unsigned int buffer_size, double stream_time, RtAudioStreamStatus status, void *p_device)
{
	AudioDevice *p_dev = (AudioDevice*)p_device;
	if(p_dev)
	{
		AudioSample *buffer = (AudioSample*)out_buffer;

		if(status)
			std::cout << "Stream underflow!!\n";	//??

		//Get audio data from device's callback
		AStatus status;

		BufferDesc chunk(buffer_size, p_dev->sampleRate, p_dev->numChannels);
		if(p_dev->callback)
			status = p_dev->callback(chunk, stream_time);

		if(statusGood(status))
		{
			//Copy data to buffer
			for(s_time t = 0; t < buffer_size; t++)
			{
				for(int channel = 0; channel < p_dev->numChannels; channel++)
				{
					//Interlace data
					buffer[t*p_dev->numChannels + channel] = (chunk.data[channel])[t];
				}
			}

			return 0;
		}
		else
			std::cout << "AUDIO DEVICE CALLBACK FAILED.\n";
	}

	return 1;
}

void AudioDevice::setCallback(AudioCallback callback_func)
{
	callback = callback_func;
}

void AudioDevice::play()
{
	if(!playing)
	{
		try
		{
			audioOut->startStream();
		}
		catch(RtAudioError &e)
		{
			e.printMessage();
			return;
		}
		playing = true;
	}
}

void AudioDevice::stop()
{
	if(playing)
	{
		try
		{
			audioOut->stopStream();
		}
		catch(RtAudioError &e)
		{
			e.printMessage();
			return;
		}
		playing = false;
	}
}

void AudioDevice::togglePlay()
{
	if(playing)
		stop();
	else
		play();
}

bool AudioDevice::isPlaying() const
{
	return playing;
}

void AudioDevice::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new AudioDevDesc()));
	AudioDevDesc *desc = dynamic_cast<AudioDevDesc*>(objDesc);
	
	desc->name = name;
	desc->sampleRate = sampleRate;
	desc->chunkSize = chunkSize;
	desc->numChannels = numChannels;
}