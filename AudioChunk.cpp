#include "AudioChunk.h"

#include <algorithm>

/////AUDIO CHUNK/////

AudioChunk::AudioChunk(s_time chunk_size)
	: size(chunk_size)
{

}

AudioChunk::~AudioChunk()
{ }

s_time AudioChunk::getSize() const
{
	return size;
}

DataStatus AudioChunk::getStatus() const
{
	return status;
}

void AudioChunk::setStatus(DataStatus ds)
{
	status = ds;
}




/////AUDIO AMP CHUNK/////

AudioAmpChunk::AudioAmpChunk(s_time chunk_size)
	: AudioChunk(chunk_size),
		data(chunk_size, 0)
{

}

AudioAmpChunk::AudioAmpChunk(const AudioAmpChunk &other)
	: AudioChunk(other.size)
{
	data.reserve(size);
	data.insert(data.begin(), other.data.begin(), other.data.end());
}

AudioAmpChunk::~AudioAmpChunk()
{ }

const std::vector<AudioSample>* AudioAmpChunk::getData() const
{
	return &data;
}

std::vector<AudioSample>* AudioAmpChunk::getDataRef()
{
	return &data;
}

void AudioAmpChunk::loadZeros()
{
	for(auto &s : data)
		s = 0;
}

void AudioAmpChunk::clamp(AudioSample min_amp, AudioSample max_amp)
{
	for(auto &s : data)
		s = std::min(max_amp, std::max(min_amp, s));
}

void AudioAmpChunk::offset(AudioSample offset)
{
	for(auto &s : data)
		s += offset;
}

void AudioAmpChunk::setData(const AudioAmpChunk &other)
{
	size = other.size;
	status = other.status;
	NOTE_ON = other.NOTE_ON;

	data.clear();
	data.reserve(size);
	data.insert(data.end(), other.data.begin(), other.data.end());
}

void AudioAmpChunk::setData(const std::vector<AudioSample> &c_data)
{
	size = c_data.size();

	data.clear();
	data.reserve(size);
	data.insert(data.end(), c_data.begin(), c_data.end());
}

void AudioAmpChunk::getAmpData(std::vector<AudioSample> &out_data, AudioSample c_seed) const
{
	out_data.clear();
	out_data.reserve(size);
	out_data.insert(out_data.end(), data.begin(), data.end());
}

void AudioAmpChunk::getVelData(std::vector<AudioVelSample> &out_data, AudioSample c_seed) const
{
	out_data.clear();
	out_data.resize(size);

	AudioSample c_amp = c_seed;

	for(s_time s = 0; s < size; s++)
	{
		AudioVelSample vel = data[s] - c_amp;
		out_data[s] = vel;
		c_amp += vel;
	}
}

AudioAmpChunk& AudioAmpChunk::operator=(const AudioAmpChunk &other)
{
	setData(other);
	return *this;
}

AudioAmpChunk& AudioAmpChunk::operator+=(const AudioAmpChunk &rhs)
{
	for(s_time s = 0; s < size; s++)
		data[s] += rhs.data[s];

	return *this;
}

const AudioAmpChunk& AudioAmpChunk::operator+(const AudioAmpChunk &rhs) const
{
	return (AudioAmpChunk(*this) += rhs);
}

AudioAmpChunk& AudioAmpChunk::operator-=(const AudioAmpChunk &rhs)
{
	for(s_time s = 0; s < size; s++)
		data[s] -= rhs.data[s];

	return *this;
}

const AudioAmpChunk& AudioAmpChunk::operator-(const AudioAmpChunk &rhs) const
{
	return (AudioAmpChunk(*this) -= rhs);
}
	
AudioAmpChunk& AudioAmpChunk::operator+=(const AudioVelChunk &rhs)
{
	std::vector<AudioSample> rhs_data;
	rhs.getAmpData(rhs_data, 0);		//TODO: Use correct seed??

	for(s_time s = 0; s < size; s++)
		data[s] += rhs_data[s];

	return *this;
}

AudioAmpChunk& AudioAmpChunk::operator-=(const AudioVelChunk &rhs)
{
	std::vector<AudioSample> rhs_data;
	rhs.getAmpData(rhs_data, 0);		//TODO: Use correct seed??

	for(s_time s = 0; s < size; s++)
		data[s] -= rhs_data[s];

	return *this;
}


/////AUDIO VEL CHUNK/////

AudioVelChunk::AudioVelChunk(s_time chunk_size, AudioSample chunk_seed)
	: AudioChunk(chunk_size),
		data(chunk_size, 0), seed(chunk_seed)
{

}

AudioVelChunk::AudioVelChunk(const AudioVelChunk &other)
	: AudioChunk(other.size),
		chunkStep(other.chunkStep), seed(other.seed), seedStatus(other.seedStatus)
{
	data.reserve(size);
	data.insert(data.begin(), other.data.begin(), other.data.end());
}

AudioVelChunk::~AudioVelChunk()
{ }


const std::vector<AudioVelSample>* AudioVelChunk::getData() const
{
	return &data;
}

std::vector<AudioVelSample>* AudioVelChunk::getDataRef()
{
	return &data;
}

AudioVelSample AudioVelChunk::getChunkStep() const
{
	return chunkStep;
}

void AudioVelChunk::updateChunkStep()
{
	chunkStep = 0;
	for(auto s : data)
		chunkStep += s;
}

void AudioVelChunk::setSeed(AudioSample new_seed)
{
	seed = new_seed;
}

AudioSample AudioVelChunk::getSeed() const
{
	return seed;
}

void AudioVelChunk::loadZeros()
{
	for(auto &s : data)
		s = 0;
	chunkStep = 0;
}

void AudioVelChunk::setData(const AudioVelChunk &other)
{
	size = other.size;
	status = other.status;
	NOTE_ON = other.NOTE_ON;

	chunkStep = other.chunkStep;
	seed = other.seed;
	seedStatus = other.seedStatus;

	data.clear();
	data.reserve(size);
	data.insert(data.end(), other.data.begin(), other.data.end());
}


void AudioVelChunk::setData(const std::vector<AudioVelSample> &c_data)
{
	size = c_data.size();

	data.clear();
	data.resize(size);

	chunkStep = 0;
	for(s_time s = 0; s < size; s++)
		chunkStep += (data[s] = c_data[s]);

}

void AudioVelChunk::getAmpData(std::vector<AudioSample> &out_data, AudioSample c_seed) const
{
	out_data.clear();
	out_data.resize(size);

	AudioSample amplitude = c_seed;

	for(s_time s = 0; s < size; s++)
	{
		amplitude += data[s];
		out_data[s] = amplitude;
	}
}

void AudioVelChunk::getVelData(std::vector<AudioVelSample> &out_data, AudioSample c_seed) const
{
	out_data.clear();
	out_data.reserve(size);
	out_data.insert(out_data.end(), data.begin(), data.end());
}

AudioVelChunk& AudioVelChunk::operator=(const AudioVelChunk &other)
{
	setData(other);
	return *this;
}

AudioVelChunk& AudioVelChunk::operator+=(const AudioVelChunk &rhs)
{
	for(s_time s = 0; s < size; s++)
		data[s] += rhs.data[s];

	seed += rhs.seed;
	chunkStep += rhs.chunkStep;

	return *this;
}

const AudioVelChunk& AudioVelChunk::operator+(const AudioVelChunk &rhs) const
{
	return (AudioVelChunk(*this) += rhs);
}

AudioVelChunk& AudioVelChunk::operator-=(const AudioVelChunk &rhs)
{
	for(s_time s = 0; s < size; s++)
		data[s] -= rhs.data[s];

	seed -= rhs.seed;
	chunkStep -= rhs.chunkStep;

	return *this;
}

const AudioVelChunk& AudioVelChunk::operator-(const AudioVelChunk &rhs) const
{
	return (AudioVelChunk(*this) -= rhs);
}
	
/*
AudioVelChunk& AudioVelChunk::operator+=(const AudioAmpChunk &rhs)
{
	std::vector<AudioSample> this_data;
	getAmpData(this_data, 0);		//TODO: Use correct seed??

	for(c_time c = 0; c < size; c++)
		data[c] += rhs.data[c];

	return *this;
}

AudioVelChunk& AudioVelChunk::operator-=(const AudioAmpChunk &rhs)
{
	std::vector<AudioSample> rhs_data;
	rhs.getAmpData(rhs_data, 0);		//TODO: Use correct seed??

	for(c_time c = 0; c < size; c++)
		data[c] -= rhs_data[c];

	return *this;
}
*/