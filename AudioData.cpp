#include "AudioData.h"

#include "AudioChunk.h"
#include "Interpolation.h"

#include <algorithm>


/////AUDIO DATA/////
AudioData::AudioData(s_time chunk_size)
	: chunkSize(chunk_size)
{ }

AudioData::~AudioData()
{

}

s_time AudioData::getChunkSize() const
{
	return chunkSize;
}

/////AUDIO AMP DATA/////

AudioAmpData::AudioAmpData(s_time chunk_size, c_time num_chunks)
	: AudioData(chunk_size),
		data(num_chunks, nullptr)
{
	for(auto &d : data)
		d = new AudioAmpChunk(chunk_size);
}

AudioAmpData::~AudioAmpData()
{
	for(auto d : data)
		if(d) delete d;
	data.clear();
}

AudioAmpChunk* AudioAmpData::getChunk(c_time c_index)
{
	return data[c_index];
}

c_time AudioAmpData::getNumChunks() const
{
	return data.size();
}

std::vector<AudioAmpChunk*>* AudioAmpData::getData()
{
	return &data;
}

DataStatus AudioAmpData::getChunkStatus(c_time c_index) const
{
	return data[c_index]->getStatus();
}


void AudioAmpData::loadZeros()
{
	for(auto d : data)
		d->loadZeros();
}

void AudioAmpData::resize(c_time new_num_chunks)
{
	if(new_num_chunks < data.size())
	{
		//Delete extra chunks
		for(c_time c = new_num_chunks; c < data.size(); c++)
		{
			if(data[c])
				delete data[c];
			data[c] = nullptr;
		}
		//Resize
		data.resize(new_num_chunks);
	}
	else if(new_num_chunks > data.size())
	{
		//Resize
		c_time c = data.size();
		data.resize(new_num_chunks);

		//Add new chunks
		for(; c < data.size(); c++)
			data[c] = new AudioAmpChunk(chunkSize);
	}
	//Else, no resize
}

void AudioAmpData::sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const
{
	out_amplitudes.clear();
	out_amplitudes.resize(num_samples);

	//Linear interpolation
	s_time	s_start = (s_time)floor(sample_start);

	c_time c_low = s_start/chunkSize;

	for(s_time si = 0; si < num_samples; si++)
	{
		float	sample = si*sample_step,
				sample_floor = floor(sample);

		s_time	s_low = (s_time)sample_floor - c_low*chunkSize,
				s_high = s_low + 1;

		bool wrap = (s_high >= chunkSize);

		s_high = (wrap ? 0 : s_high);

		c_time	c_high = (wrap ? c_low + 1 : c_low);

		float	low_amp = data[c_low]->data[s_low],
				high_amp = data[c_high]->data[s_high];

		out_amplitudes[si] = lerp(low_amp, high_amp, sample - sample_floor);

		c_low = c_high;
	}
}

void AudioAmpData::getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const
{
	c_time out_index = 0;
	for(c_time c = range.start; c < range.end; c++)
	{
		out_data[out_index]->setData(*data[c]);
		out_index++;
	}
}

void AudioAmpData::getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const
{
	AudioSample c_seed = (range.start > 0 ? data[range.start - 1]->data[chunkSize - 1] : 0);

	std::vector<AudioVelSample> vel_data;
	vel_data.reserve(chunkSize);

	c_time out_index = 0;
	for(c_time c = range.start; c < range.end; c++)
	{
		data[c]->getVelData(vel_data, c_seed);
		out_data[out_index]->setData(vel_data);

		c_seed = data[c]->data[chunkSize - 1];
	}
}

AudioAmpData& AudioAmpData::operator=(const AudioAmpData &other)
{
	chunkSize = other.chunkSize;

	for(auto d : data)
		if(d) delete d;

	data.resize(other.data.size());

	for(c_time c = 0; c < data.size(); c++)
		data[c] = new AudioAmpChunk(*other.data[c]);

	return *this;
}



/////AUDIO VEL DATA/////

AudioVelData::AudioVelData(s_time chunk_size, c_time num_chunks, AudioSample primary_seed)
	: AudioData(chunk_size),
		data(num_chunks, nullptr), seed(primary_seed)
{
	for(auto &d : data)
		d = new AudioVelChunk(chunk_size, seed);
}

AudioVelData::~AudioVelData()
{
	for(auto d : data)
		if(d) delete d;
	data.clear();
}

AudioVelChunk* AudioVelData::getChunk(c_time c_index)
{
	return data[c_index];
}

c_time AudioVelData::getNumChunks() const
{
	return data.size();
}

std::vector<AudioVelChunk*>* AudioVelData::getData()
{
	return &data;
}

DataStatus AudioVelData::getChunkStatus(c_time c_index) const
{
	return data[c_index]->getStatus();
}

AudioSample AudioVelData::getSeed() const
{
	return seed;
}

void AudioVelData::setSeed(AudioSample new_seed)
{
	seed = new_seed;
	updateChunkSeeds();
}

void AudioVelData::updateChunkSeeds()
{
	AudioSample c_seed = seed;

	for(auto c : data)
	{
		c->seed = c_seed;
		c->updateChunkStep();
		c_seed += c->chunkStep;
	}
}

void AudioVelData::loadZeros()
{
	for(auto c : data)
		c->loadZeros();
}

void AudioVelData::resize(c_time new_num_chunks)
{
	if(new_num_chunks < data.size())
	{
		//Delete extra chunks
		for(c_time c = new_num_chunks; c < data.size(); c++)
		{
			if(data[c])
				delete data[c];
			data[c] = nullptr;
		}
		//Resize
		data.resize(new_num_chunks);
	}
	else if(new_num_chunks > data.size())
	{
		//updateChunkSeeds();

		//Resize
		c_time c = data.size();
		data.resize(new_num_chunks);

		//Add new chunks
		AudioSample c_seed = data[c - 1]->seed + data[c - 1]->chunkStep;
		for(; c < data.size(); c++)
			data[c] = new AudioVelChunk(chunkSize, c_seed);

	}
	//Else, no resize
}


void AudioVelData::sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const
{
	out_amplitudes.clear();
	out_amplitudes.resize(num_samples);


	float sample = sample_start;
	c_time last_c = sample_start/chunkSize;

	/*
	//Linear interpolation
	s_time	s_low = (s_time)floor(sample);
	c_time c_low = s_low/chunkSize;
	
	s_time s_offset = s_low % chunkSize;

	s_low += s_low % chunkSize;
	s_low -= c_low*chunkSize;
	*/

	std::vector<AudioSample> amp_data;
	data[last_c]->getAmpData(amp_data, data[last_c]->getSeed());

	//AudioSample amp_low = data[c_low]->getSeed(),
	//			amp_high = amp_low;

	c_time ci = 0;
	for(s_time si = 0; si < num_samples; si++)
	{
		float sample_floor = floor(sample);
		
		c_time	c = (s_time)sample_floor / chunkSize;

		if(c != last_c)
			data[c]->getAmpData(amp_data, data[c]->getSeed());
		
		s_time s_low = (s_time)sample_floor % chunkSize;
		s_time s_high = s_low + 1;
		
		float amp_low = amp_data[s_low];

		if(s_low >= chunkSize - 1)
		{
			s_high = 0;
			c++;
			
			data[c]->getAmpData(amp_data, data[c]->getSeed());
		}

		float amp_high = amp_data[s_high];

		out_amplitudes[si] = lerp(amp_low, amp_high, sample - sample_floor);

		sample += sample_step;
		last_c = c;
	}
}


void AudioVelData::getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const
{
	//AudioSample c_seed = data[range.start]->seed; //TODO: chunks other than first!!

	std::vector<AudioSample> amp_data;
	amp_data.reserve(chunkSize);

	c_time out_index = 0;
	for(c_time c = range.start; c < range.end; c++)
	{
		data[c]->getAmpData(amp_data, data[c]->seed);
		out_data[out_index]->setData(amp_data);

		//c_seed = amp_data[chunkSize - 1];
		out_index++;
	}
}

void AudioVelData::getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const
{
	c_time out_index = 0;
	for(c_time c = range.start; c < range.end; c++)
	{
		out_data[out_index]->setData(*data[c]);
		out_index++;
	}
}

AudioVelData& AudioVelData::operator=(const AudioVelData &other)
{
	chunkSize = other.chunkSize;
	seed = other.seed;

	for(auto d : data)
		if(d) delete d;

	data.resize(other.data.size());

	for(c_time c = 0; c < data.size(); c++)
		data[c] = new AudioVelChunk(*other.data[c]);

	return *this;
}