#include "AudioData.h"

#include "AudioChunk.h"
#include "Interpolation.h"

#include <algorithm>
#include <fstream>

#include "FileSystem.h"
#include "Path.h"


/////AUDIO DATA/////

AudioData::AudioData(s_time chunk_size)
	: chunkSize(chunk_size)
{ }

AudioData::~AudioData()
{ }

s_time AudioData::getChunkSize() const
{
	return chunkSize;
}
/*
ChunkType* AudioData::operator[](c_time c_index)
{
	return data[c_index];
}

const ChunkType* AudioData::operator[](c_time c_index) const
{
	return data[c_index];
}

ChunkType& AudioData::operator()(c_time c_index)
{
	return *data[c_index];
}

const ChunkType& AudioData::operator()(c_time c_index) const
{
	return *data[c_index];
}
*/
/////AUDIO AMP DATA/////

AudioAmpData::AudioAmpData(s_time chunk_size, c_time num_chunks)
	: AudioData(chunk_size), data(num_chunks, nullptr)
{
	for(auto &d : data)
		d = new AudioAmpChunk(chunk_size);
}

AudioAmpData::~AudioAmpData()
{ }


c_time AudioAmpData::getNumChunks() const
{
	return data.size();
}

std::vector<AudioAmpChunk*>* AudioAmpData::getData()
{
	return &data;
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


//Square brackets return chunk pointer at c_index
AudioAmpChunk* AudioAmpData::operator[](c_time c_index)
{
	return data[c_index];
}

const AudioAmpChunk* AudioAmpData::operator[](c_time c_index) const
{
	return data[c_index];
}

//Parentheses return chunk reference at c_index
AudioAmpChunk& AudioAmpData::operator()(c_time c_index)
{
	return *data[c_index];
}

const AudioAmpChunk& AudioAmpData::operator()(c_time c_index) const
{
	return *data[c_index];
}


void AudioAmpData::writeToFile(const Path &file_path)
{
	std::ofstream out(file_path.getSystem(), std::ofstream::out);

	for(c_time c = 0; c < data.size(); c++)
	{
		AudioAmpChunk &curr_chunk = *data[c];
		for(s_time s = 0; s < chunkSize; s++)
		{
			FileSystem::writeFile(out, curr_chunk[s]);
		}
	}

	out.close();
}

void AudioAmpData::readFromFile(const Path &file_path)
{
	data.clear();

	std::ifstream in(file_path.getSystem(), std::ifstream::in);
	//TODO: Store chunk size, etc. in file header

	while(!in.eof())
	{
		for(c_time c = 0; !in.eof(); c++)
		{
			AudioAmpChunk *curr_chunk = new AudioAmpChunk(chunkSize);
			data.push_back(curr_chunk);

			for(s_time s = 0; s < chunkSize && !in.eof(); s++)
			{
				FileSystem::readFile(in, (*curr_chunk)[s]);
			}
		}
	}

	in.close();
}


/////AUDIO VEL DATA/////

AudioVelData::AudioVelData(s_time chunk_size, c_time num_chunks, AudioSample primary_seed)
	: AudioData(chunk_size),
		data(num_chunks, nullptr), seed(primary_seed)
{
	for(auto &d : data)
		d = new AudioVelChunk(chunk_size, seed);
}

AudioVelData::AudioVelData(const AudioVelData &other)
	: AudioData(other.chunkSize),
		data(other.chunkSize, nullptr), seed(other.seed)
{
	for(int i = 0; i < chunkSize; i++)
		data[i] = new AudioVelChunk(*other.data[i]);
}

AudioVelData::~AudioVelData()
{
	for(auto d : data)
		if(d) delete d;
	data.clear();
}

c_time AudioVelData::getNumChunks() const
{
	return data.size();
}

std::vector<AudioVelChunk*>* AudioVelData::getData()
{
	return &data;
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


//Square brackets return chunk pointer at c_index
AudioVelChunk* AudioVelData::operator[](c_time c_index)
{
	return data[c_index];
}

const AudioVelChunk* AudioVelData::operator[](c_time c_index) const
{
	return data[c_index];
}

//Parentheses return chunk reference at c_index
AudioVelChunk& AudioVelData::operator()(c_time c_index)
{
	return *data[c_index];
}

const AudioVelChunk& AudioVelData::operator()(c_time c_index) const
{
	return *data[c_index];
}


void AudioVelData::writeToFile(const Path &file_path)
{
	std::ofstream out(file_path.getSystem(), std::ofstream::out);

	for(c_time c = 0; c < data.size(); c++)
	{
		AudioVelChunk &curr_chunk = *data[c];
		for(s_time s = 0; s < chunkSize; s++)
		{
			FileSystem::writeFile(out, curr_chunk[s]);
		}
	}

	out.close();
}

void AudioVelData::readFromFile(const Path &file_path)
{
	data.clear();

	std::ifstream in(file_path.getSystem(), std::ifstream::in);
	//TODO: Store chunk size, etc. in file header

	AudioSample last_seed = 0;	//TODO: Store seed with vel data

	while(!in.eof())
	{
		for(c_time c = 0; !in.eof(); c++)
		{
			AudioVelChunk *curr_chunk = new AudioVelChunk(chunkSize, last_seed);
			data.push_back(curr_chunk);

			for(s_time s = 0; s < chunkSize && !in.eof(); s++)
			{
				FileSystem::readFile(in, (*curr_chunk)[s]);
			}

			data[c]->updateChunkStep();
			last_seed += data[c]->chunkStep;
		}
	}

	in.close();
}