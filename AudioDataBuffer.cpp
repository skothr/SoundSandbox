#include "AudioDataBuffer.h"


/////AUDIO AMP DATA BUFFER/////

AudioAmpDataBuffer::AudioAmpDataBuffer(s_time chunk_size, c_time buffer_size, bool double_buffer)
	: AudioAmpData(chunk_size, buffer_size),
		doubleBuffered(double_buffer), activeChunk(double_buffer ? new AudioAmpChunk(chunk_size) : nullptr)
{

}

AudioAmpDataBuffer::~AudioAmpDataBuffer()
{ }

AudioAmpChunk* AudioAmpDataBuffer::shiftBuffer()
{
	std::rotate(data.begin(), data.begin() + 1, data.end());
	if(activeChunk)
		std::swap(activeChunk, data[data.size() - 1]);

	return activeChunk;
}


AudioAmpChunk* AudioAmpDataBuffer::getActiveChunk()
{
	return activeChunk;
}

AudioAmpChunk* AudioAmpDataBuffer::getNextChunk()
{
	return data[0];
}

AudioAmpChunk* AudioAmpDataBuffer::getLastChunk()
{
	return data[data.size() - 1];
}


/////AUDIO VEL DATA BUFFER/////

AudioVelDataBuffer::AudioVelDataBuffer(s_time chunk_size, c_time buffer_size, bool double_buffer)
	: AudioVelData(chunk_size, buffer_size, 0),
		doubleBuffered(double_buffer), activeChunk(double_buffer ? new AudioVelChunk(chunk_size, 0) : nullptr)
{ }

AudioVelDataBuffer::~AudioVelDataBuffer()
{
	if(activeChunk)
		delete activeChunk;
}

AudioVelChunk* AudioVelDataBuffer::shiftBuffer()
{
	if(data.size() > 0)
	{
		std::rotate(data.begin(), data.begin() + 1, data.end());

		if(data.size() > 1)
		{
			if(activeChunk)
			{
				std::swap(activeChunk, data[data.size() - 1]);
				activeChunk->seed = data[data.size() - 1]->seed + data[data.size() - 1]->chunkStep;
			}
			else
				data[data.size() - 1]->seed = data[data.size() - 2]->seed + data[data.size() - 2]->chunkStep;
		}
		else if(activeChunk)
		{
			std::swap(activeChunk, data[0]);
			activeChunk->seed = data[0]->seed + data[0]->chunkStep;
		}

		seed = data[0]->seed;

		return activeChunk;
	}
}

AudioVelChunk* AudioVelDataBuffer::getActiveChunk()
{
	return activeChunk;
}

AudioVelChunk* AudioVelDataBuffer::getNextChunk()
{
	return data[0];
}

AudioVelChunk* AudioVelDataBuffer::getLastChunk()
{
	return data[data.size() - 1];
}