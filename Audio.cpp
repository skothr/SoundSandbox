#include "Audio.h"

#include "Sampling.h"
#include "Interpolation.h"

#include <cstdint>

s_time AUDIO_CHUNK_SIZE = 512;
float AUDIO_MAX_AMPLITUDE = 16000.0f;




/*
/////DATA CHUNK BASE CLASS/////

DataChunk::DataChunk(s_time chunk_size)
	: size(chunk_size)
{ }

DataChunk::DataChunk(const DataChunk &other)
	: size(other.size), status(other.status), NOTE_ON(other.NOTE_ON)
{
	setPointers(other.prevChunk, other.nextChunk);
}

DataStatus DataChunk::getStatus() const
{
	return status;
}

void DataChunk::setStatus(DataStatus d_status)
{
	status = d_status;
}

s_time DataChunk::getSize() const
{
	return size;
}

void DataChunk::setPointers(DataChunk *p_prev, DataChunk *p_next)
{
	prevChunk = p_prev;
	nextChunk = p_next;
}



/////AUDIO CHUNK/////

AudioChunk::AudioChunk(s_time chunk_size)
	: DataChunk(chunk_size),
		aData(chunk_size, 0)
{ }

AudioChunk::AudioChunk(const AudioChunk &other)
	: DataChunk(other)
{
	//size = other.size;
	//status = other.status;
	//NOTE_ON = other.NOTE_ON;
	
	aData.clear();
	aData.reserve(size);
	aData.insert(aData.end(), other.aData.begin(), other.aData.end());

	setPointers(other.prevChunk, other.nextChunk);

	//aData.reserve(size);
	//for(auto a : other.aData)
	//	aData.push_back(a);
}

void AudioChunk::loadZeros()
{
	for(auto &a : aData)
		a = 0;
}

const AudioSample* AudioChunk::getData() const
{
	return aData.data();
}

void AudioChunk::getData(AudioChunkData &output) const
{
	output.clear();
	output.reserve(size);
	for(auto a : aData)
		output.push_back(a);
}

void AudioChunk::setData(const AudioChunkData &new_data)
{
	for(unsigned int i = 0; i < size; i++)
		aData[i] = new_data[i];
}

AudioVelChunk AudioChunk::getVelChunk() const
{
	AudioVelChunk output(size, getPreviousSample());
	output.setData(getVelData());
	return output;
}

//void AudioChunk::getVelChunk(AudioVelChunk &output) const
//{
//	FAudioSample last_sample = getPreviousSample();
//	output.setSeed(last_sample);
//	output.setData(getVelData());
//}

AudioVelChunkData AudioChunk::getVelData() const
{
	AudioVelChunkData output;
	getVelData(output);
	return output;
}

void AudioChunk::getVelData(AudioVelChunkData &output) const
{
	output.clear();
	output.reserve(size);

	AudioSample last_sample = getPreviousSample();

	for(auto a : aData)
	{
		output.push_back(a - last_sample);
		last_sample = a;
	}
}

AudioSample AudioChunk::getSample(s_time index) const
{
	return aData[index];
}

AudioSample& AudioChunk::getSampleRef(s_time index)
{
	return aData[index];
}

void AudioChunk::setSample(s_time index, AudioSample s)
{
	aData[index] = s;
}


AudioSample AudioChunk::getPreviousSample() const
{
	return (prevChunk ? prevChunk->getLastSample() : 0);
}
AudioSample AudioChunk::getFirstSample() const
{
	return aData[0];
}
AudioSample AudioChunk::getLastSample() const
{
	return aData[size - 1];
}
AudioSample AudioChunk::getNextSample() const
{
	return (nextChunk ? nextChunk->getFirstSample() : getLastSample());
}


AudioChunk& AudioChunk::operator=(const AudioChunk &other)
{
	size = other.size;
	status = other.status;
	NOTE_ON = other.NOTE_ON;

	aData.clear();
	aData.reserve(size);
	aData.insert(aData.end(), other.aData.begin(), other.aData.end());

	setPointers(other.prevChunk, other.nextChunk);
	//setPointers(nullptr, nullptr);

	return *this;
}

AudioSample& AudioChunk::operator[](unsigned int index)
{
	return aData[index];
}

const AudioSample& AudioChunk::operator[](unsigned int index) const
{
	return aData[index];
}

AudioChunk AudioChunk::operator-() const
{
	AudioChunk neg(*this);
	for(auto &a : neg.aData)
		a = -a;

	return neg;
}

AudioChunk& AudioChunk::operator+=(const AudioChunk &rhs)
{
	for(unsigned int i = 0; i < size; i++)
		aData[i] += rhs.aData[i];

	return *this;
}
AudioChunk AudioChunk::operator+(const AudioChunk &rhs) const
{
	return AudioChunk(*this) += rhs;
}
	
AudioChunk& AudioChunk::operator-=(const AudioChunk &rhs)
{
	for(unsigned int i = 0; i < size; i++)
		aData[i] -= rhs.aData[i];

	return *this;
}
AudioChunk AudioChunk::operator-(const AudioChunk &rhs) const
{
	return AudioChunk(*this) -= rhs;
}



/////AUDIO VEL CHUNK/////

AudioVelChunk::AudioVelChunk(s_time chunk_size, AudioSample seed)
	: DataChunk(chunk_size),
		vData(chunk_size, 0), seedSample(seed), totalVel(0)
{ }

AudioVelChunk::AudioVelChunk(const AudioVelChunk &other)
	: DataChunk(other),
		seedSample(other.seedSample), seedStatus(other.seedStatus), totalVel(other.totalVel),
		updating(false), dirtying(false)
{
	vData.clear();
	vData.reserve(size);
	vData.insert(vData.end(), other.vData.begin(), other.vData.end());

	setPointers(other.prevChunk, other.nextChunk);
}

void AudioVelChunk::setDirtySeed()
{
	seedStatus = DataStatus::DIRTY;
	if(nextVelChunk && !dirtying)
	{
		dirtying = true;
		nextVelChunk->setDirtySeed();
		//nextVelChunk->
	}
	dirtying = false;
}

void AudioVelChunk::loadZeros()
{
	totalVel = 0;
	for(auto &v : vData)
		v = 0;

	setSeed(prevVelChunk ? prevVelChunk->getLastSample() : seedSample);
	
	//if(nextVelChunk) 
	//	nextVelChunk->setSeed(getLastSample());

	//if(nextVelChunk) nextVelChunk->setDirtySeed();
}

void AudioVelChunk::setSeed(AudioSample seed)
{
	seedSample = seed;
	seedStatus = DataStatus::CLEAN;

	if(nextVelChunk)
		//nextVelChunk->setDirtySeed();
		nextVelChunk->setSeed(getLastSample());
}

AudioSample AudioVelChunk::getSeed() const
{
	return seedSample;
}

void AudioVelChunk::updateInside()
{
	//AudioVelSample old_total_vel = totalVel;

	totalVel = 0;
	for(auto v : vData)
		totalVel += v;

	//AudioVelSample total_vel_diff = totalVel - old_total_vel;

	//setSeed(seedSample);

	if(nextVelChunk)// && abs(total_vel_diff) > 0)
		//nextVelChunk->setDirtySeed();
		nextVelChunk->setSeed(getLastSample());
}

void AudioVelChunk::updateOutside()
{
	//if(seedStatus != DataStatus::CLEAN && prevVelChunk && !updating)
	//{
	//	updating = true;
	//	if(prevVelChunk)
	//		prevVelChunk->updateOutside();
	//}
	//updating = false;

	//AudioSample old_seed = seedSample;

	//setSeed(prevChunk ? prevChunk->getLastSample() : seedSample);

	//seedSample = (prevChunk ? prevChunk->getLastSample() : 0);
	//seedStatus = DataStatus::CLEAN;
	
	//AudioVelSample dVel = (nextChunk && nextChunk->getStatus() == DataStatus::CLEAN ? seedSample - old_seed : 0);

	//vData[0] -= dVel;
	//totalVel += dVel;
}

void AudioVelChunk::updateAll()
{
	//updateOutside();
	//updateInside();
}

const AudioVelSample* AudioVelChunk::getData() const
{
	return (const AudioVelSample*)vData.data();
}

void AudioVelChunk::getData(AudioVelChunkData &output) const
{
	output.clear();
	output.reserve(size);
	for(auto v : vData)
		output.push_back(v);
}

void AudioVelChunk::setData(const AudioVelChunkData &new_data)
{
	//AudioVelSample old_total_vel = totalVel;

	totalVel = 0;
	for(unsigned int i = 0; i < size; i++)
		totalVel += (vData[i] = new_data[i]);
	
	//AudioVelSample total_vel_diff = totalVel - old_total_vel;

	if(nextVelChunk)// && abs(total_vel_diff) > 0)
		//nextVelChunk->setDirtySeed();
		nextVelChunk->setSeed(getLastSample());
}

AudioChunk AudioVelChunk::getAudioChunk() const
{
	AudioChunk output(size);
	output.setData(getAudioData());

	return output;
}

AudioChunkData AudioVelChunk::getAudioData() const
{
	AudioChunkData output;
	getAudioData(output);

	return output;
}

void AudioVelChunk::getAudioData(AudioChunkData &output) const
{
	output.clear();
	output.reserve(size);

	AudioSample amplitude = getPreviousSample();

	for(auto v : vData)
		output.push_back(amplitude += v);
}

AudioVelSample AudioVelChunk::getSample(s_time index) const
{
	return vData[index];
}

AudioVelSample& AudioVelChunk::getSampleRef(s_time index)
{
	return (AudioVelSample&)vData[index];
}

void AudioVelChunk::setSample(s_time index, AudioVelSample v)
{
	totalVel += (v - vData[index]);
	vData[index] = v;

	if(nextVelChunk)
		//nextVelChunk->setDirtySeed();
		nextVelChunk->setSeed(getLastSample());
}

void AudioVelChunk::setPointers(DataChunk *p_prev, DataChunk *p_next)
{
	DataChunk::setPointers(p_prev, p_next);

	prevVelChunk = dynamic_cast<AudioVelChunk*>(prevChunk);
	nextVelChunk = dynamic_cast<AudioVelChunk*>(nextChunk);

	//updateOutside();
	setSeed(prevVelChunk ? prevVelChunk->getLastSample() : seedSample);
}

void AudioVelChunk::setPrevChunk(DataChunk *p_prev)
{
	prevChunk = p_prev;
	prevVelChunk = dynamic_cast<AudioVelChunk*>(prevChunk);

	//setSeed(seedSample);
}

void AudioVelChunk::setNextChunk(DataChunk *p_next)
{
	nextChunk = p_next;
	nextVelChunk = dynamic_cast<AudioVelChunk*>(nextChunk);

	//if(nextVelChunk)
	//	nextVelChunk->setSeed(getLastSample());
}

AudioSample AudioVelChunk::getPreviousSample() const
{
	return prevVelChunk ? prevVelChunk->getLastSample() : seedSample;
}
AudioSample AudioVelChunk::getFirstSample() const
{
	return getPreviousSample() + vData[0];
}
AudioSample AudioVelChunk::getLastSample() const
{
	return getPreviousSample() + totalVel;
}
AudioSample AudioVelChunk::getNextSample() const
{
	return (nextChunk ? nextChunk->getFirstSample() : getLastSample());
}


AudioVelChunk& AudioVelChunk::operator=(const AudioVelChunk &other)
{
	size = other.size;
	status = other.status;
	NOTE_ON = other.NOTE_ON;

	seedSample = other.seedSample;
	seedStatus = other.seedStatus;

	totalVel = other.totalVel;
	
	vData.clear();
	vData.reserve(size);
	vData.insert(vData.end(), other.vData.begin(), other.vData.end());

	updating = false;
	dirtying = false;

	setPointers(other.prevChunk, other.nextChunk);
	//setPointers(nullptr, nullptr);

	return *this;
}

//AudioVelSample& AudioVelChunk::operator[](unsigned int index)
//{
//	return (AudioVelSample&)vData[index];
//}

const AudioVelSample& AudioVelChunk::operator[](unsigned int index) const
{
	return vData[index];
}


AudioVelChunk AudioVelChunk::operator-() const
{
	AudioVelChunk neg(*this);
	for(auto &a : neg.vData)
		a = -a;

	neg.totalVel = -neg.totalVel;

	return neg;
}


AudioVelChunk& AudioVelChunk::operator+=(const AudioVelChunk &rhs)
{
	//TODO: Need to take seeds into account?
	totalVel += rhs.totalVel;
	for(unsigned int i = 0; i < size; i++)
		vData[i] += rhs.vData[i];

	return *this;
}
AudioVelChunk AudioVelChunk::operator+(const AudioVelChunk &rhs) const
{
	return AudioVelChunk(*this) += rhs;
}
	
AudioVelChunk& AudioVelChunk::operator-=(const AudioVelChunk &rhs)
{
	totalVel -= rhs.totalVel;
	for(unsigned int i = 0; i < size; i++)
		vData[i] -= rhs.vData[i];

	return *this;
}
AudioVelChunk AudioVelChunk::operator-(const AudioVelChunk &rhs) const
{
	return AudioVelChunk(*this) -= rhs;
}

AudioVelChunk& AudioVelChunk::operator+=(const AudioChunk &rhs)
{
	AudioVelChunk avd = rhs.getVelChunk();

	//totalVel += rhs.getFirstSample()
	const AudioSample *rhs_data = rhs.getData();

	AudioSample last_sample = getPreviousSample();

	for(unsigned int i = 0; i < size; i++)
	{
		vData[i] += (rhs_data[i] - last_sample);
		last_sample += vData[i];
	}

	return *this;
}

AudioVelChunk& AudioVelChunk::operator-=(const AudioChunk &rhs)
{
	AudioVelChunk avd = rhs.getVelChunk();
	const AudioSample *rhs_data = rhs.getData();

	totalVel -= rhs.getLastSample() - rhs.getFirstSample();
	
	AudioSample last_sample = getPreviousSample();

	for(unsigned int i = 0; i < size; i++)
	{
		vData[i] -= (rhs_data[i] - last_sample);
		last_sample += vData[i];
	}

	return *this;
}


////Resizes the given AudioData chunks to be the size determined by AUDIO_CHUNK_SIZE.
////TODO: May not be 100% complete for all cases. Recheck.
//AudioData resizeAudioChunks(AudioData old_data)
//{
//	//Make new AudioData
//	AudioData new_data(old_data.size());
//
//	unsigned int old_i = 0;
//	s_time old_offset = 0;
//	for(unsigned int i = 0; i < new_data.size(); i++)
//	{
//		//Allocate new chunk
//		new_data[i] = new AudioChunk(AUDIO_CHUNK_SIZE);
//
//		s_time start = 0;
//		//Loop through new chunk data, fill with data from old data
//		while(start < AUDIO_CHUNK_SIZE)
//		{
//			//Find length of data to sample this iteration
//			s_time length = old_data[old_i]->size - old_offset;
//
//			//Assert range inside both chunks
//			if(start + length >= AUDIO_CHUNK_SIZE)
//				length = AUDIO_CHUNK_SIZE - start;
//
//			if(old_offset + length >= old_data[old_i]->size)
//			{
//				length = old_data[old_i]->size - old_offset;
//				old_offset -= old_data[old_i]->size;
//				old_i++;
//			}
//
//			//Copy data
//			new_data[i]->setData(SampleRange(start, start + length),
//								 old_data[old_i]->getData(SampleRange(old_offset, old_offset + length)),
//								 (i > 0 ? new_data[i - 1]->prevValue : old_data[0]->prevValue));
//
//			//Increment positions
//			old_offset += length;
//			start += length;
//		}
//	}
//
//	return new_data;
//}


//Samples the given AudioChunks
AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add)
{
	AStatus status;

	
	//if(!src || !src[0] || !dst || !dst[0])
	//{
	//	status.setError(AS::ErrorType::GENERAL, "Either source or dest is null.");
	//	return status;
	//}
	

	s_time	src_chunk_size = src[0]->size,
			dst_chunk_size = dst[0]->size;

	float	dst_to_src = (float)src_chunk_size/(float)dst_chunk_size;

	if(src_chunk_size == dst_chunk_size)	//Chunks are the same size, directly copy chunks
	{
		for(c_time c = 0; c < num_chunks; c++)
			*dst[c] = *src[c];
	}
	else if(dst_to_src < 1.0 || method == SampleMethod::POINT)	//Decompressing data, only interpolation needed (no sampling)  -OR- point sampling
	{
		for(c_time c = 0; c < num_chunks; c++)
		{
			const AudioChunkData	&src_c_data = src[c]->aData;
			AudioChunkData			&dst_c_data = dst[c]->aData;

			for(s_time t = 0; t < dst_chunk_size; t++)
			{
				float src_t = (t + 0.5f)*dst_to_src;

				s_time	low_sample = floor(src_t),
						high_sample = ceil(src_t);

				AudioSample low_amp = src_c_data[low_sample],
							high_amp;

				if(low_sample == high_sample)
				{
					//src_offset is an integer --> no interpolation
					dst_c_data[t] = dst_c_data[t]*add + low_amp;
				}
				else
				{
					high_amp = (high_sample < src_chunk_size ? src_c_data[high_sample] : src[c]->getNextSample());

					Point2f low((float)low_sample, (float)low_amp),
							high((float)high_sample, (float)high_amp);
					
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)lerp(low_amp, high_amp, src_t - (float)low_sample);
				}
			}
		}
	}
	else	//Compressing data, need to (down)sample
	{
		//Number src samples around each dst sample to average together
		float	compression_radius = dst_to_src*(1.0f/2.0f),
				src_to_dst = 1.0f/dst_to_src;
		
		float	last_val = 0.0f,
				vel = 0.0f;

		switch(method)
		{
		//Already handled (same as uncompressing data above)
		//case SampleMethod::POINT:
		//	break;
		case SampleMethod::AVERAGE:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const AudioChunkData	&src_c_data = src[c]->aData;
				AudioChunkData			&dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					float	src_t_start = t*dst_to_src,
							src_t_end = (t + 1)*dst_to_src;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					src_sample_end -= (src_sample_start + dst_to_src <= src_sample_end ? 1 : 0);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					float	mult = start_overlap,
							sum = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample <= src_sample_end; src_sample++)
					{
						sum += src_c_data[src_sample]*mult;
						mult = (src_sample + 1 == src_sample_end) ? end_overlap : 1.0f;
					}
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)(sum*src_to_dst);
				}
			}
			break;

		case SampleMethod::MAXIMUM_MAG:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample>	&src_c_data = src[c]->aData;
				std::vector<AudioSample>		 &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);

					float max = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val*s_val > max*max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)max;
				}
			}
			break;

		case SampleMethod::MAXIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->aData;
				const float prev_sample = src[c]->getPreviousSample(),
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->aData[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					//Include previous and next samples in the maximum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							max = (prev_overlap > next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val > max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)max;
				}
			}
			break;

		case SampleMethod::MINIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->aData;
				const float prev_sample = src[c]->getPreviousSample(),
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->aData[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					//Include previous and next samples in the minimum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							min = (prev_overlap < next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						min = (s_val < min) ? s_val : min;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)min;
				}
			}
			break;

		default:
			status.setError(AS::ErrorType::GENERAL, "Error while sampling chunks -- Invalid sampling method.");
			break;
		}
	}

	return status;
}

//Samples the given AudioVelChunks
AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add)
{
	AStatus status;

	//if(!src || !src[0] || !dst || !dst[0])
	//{
	//	status.setError(AS::ErrorType::GENERAL, "Either source or dest is null.");
	//	return status;
	//}

	s_time	src_chunk_size = src[0]->size,
			dst_chunk_size = dst[0]->size;

	float	dst_to_src = (float)src_chunk_size/(float)dst_chunk_size;

	if(src_chunk_size == dst_chunk_size)	//Chunks are the same size, directly copy chunks
	{
		//std::cout << "EQUAL SIZES\n";
		for(c_time c = 0; c < num_chunks; c++)
			*dst[c] = src[c]->getAudioChunk();
	}
	else if(dst_to_src < 1.0 || method == SampleMethod::POINT)	//Decompressing data, only interpolation needed (no sampling)  -OR- point sampling
	{
		//std::cout << "POINT --> " << dst_to_src << "\n";
		for(c_time c = 0; c < num_chunks; c++)
		{
			const AudioVelChunkData &src_c_data = src[c]->vData;
			AudioChunkData &dst_c_data = dst[c]->aData;
			
			AudioSample		s_amp = src[c]->getFirstSample();
			s_time			amp_t = 0;

			for(s_time t = 0; t < dst_chunk_size; t++)
			{
				float src_t = (t + 0.5f)*dst_to_src;

				s_time	low_sample = floor(src_t),
						high_sample = ceil(src_t);

				for(; amp_t < low_sample; amp_t++)
					s_amp += src_c_data[amp_t + 1];
				
				AudioSample low_amp = s_amp,
							high_amp;

				if(low_sample == high_sample)
					dst_c_data[t] = dst_c_data[t]*add + low_amp;
				else
				{
					s_amp = ((++amp_t < src_chunk_size) ? s_amp + src_c_data[amp_t] : src[c]->getNextSample());
					high_amp = s_amp;

					//Point2f low((float)low_sample, (float)low_amp),
					//		high((float)high_sample, (float)high_amp);

					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)lerp(low_amp, high_amp, src_t - (float)low_sample);
				}
			}
		}
	}
	else	//Compressing data, need to downsample
	{
		//Number src samples around each dst sample to average together
		float	src_to_dst = 1.0f/dst_to_src;

		switch(method)
		{
		//Already handled (same as uncompressing data above)
		//case SampleMethod::POINT:
		//	break;
		case SampleMethod::AVERAGE:
			for(c_time c = 0; c < num_chunks; c++)
			{
				AudioSample		s_amp = src[c]->getFirstSample();
				s_time			amp_t = 0;

				const AudioVelChunkData &src_c_data = src[c]->vData;
				AudioChunkData &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					float	src_t_start = t*dst_to_src,
							src_t_end = (t + 1)*dst_to_src;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);

					src_sample_end -= (src_sample_start + dst_to_src <= src_sample_end ? 1 : 0);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					float	mult = start_overlap,
							sum = 0;
					for(s_time src_sample = src_sample_start; src_sample <= src_sample_end; src_sample++)
					{
						s_amp = (amp_t < src_sample ? s_amp + src_c_data[++amp_t] : s_amp);
						sum += (float)s_amp*mult;
						
						mult = (src_sample + 1 == src_sample_end) ? end_overlap : 1.0f;
					}

					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)(sum*src_to_dst);
				}
			}
			break;

		case SampleMethod::MAXIMUM_MAG:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const AudioVelChunkData &src_c_data = src[c]->vData;
				std::vector<AudioSample> &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					float	src_t_start = t*dst_to_src,
							src_t_end = (t + 1)*dst_to_src;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);

					src_sample_end -= (src_sample_start + dst_to_src <= src_sample_end ? 1 : 0);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);

					float max = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val*s_val > max*max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)max;
				}
			}
			break;

		case SampleMethod::MAXIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const AudioVelChunkData &src_c_data = src[c]->vData;
				const float prev_sample = src[c]->getPreviousSample(),
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->vData[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					float	src_t_start = t*dst_to_src,
							src_t_end = (t + 1)*dst_to_src;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);

					src_sample_end -= (src_sample_start + dst_to_src <= src_sample_end ? 1 : 0);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					//Include previous and next samples in the maximum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							max = (prev_overlap > next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val > max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)max;
				}
			}
			break;

		case SampleMethod::MINIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const AudioVelChunkData &src_c_data = src[c]->vData;
				const float prev_sample = src[c]->getPreviousSample(),
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->vData[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->aData;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					float	src_t_start = t*dst_to_src,
							src_t_end = (t + 1)*dst_to_src;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);

					src_sample_end -= (src_sample_start + dst_to_src <= src_sample_end ? 1 : 0);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = (src_t_end - src_sample_end);
					
					//Include previous and next samples in the minimum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							min = (prev_overlap < next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						min = (s_val < min) ? s_val : min;
					}
					dst_c_data[t] = dst_c_data[t]*add + (FAudioSample)min;
				}
			}
			break;
			
		//case SampleMethod::SUM:
		//	for(c_time c = 0; c < num_chunks; c++)
		//	{
		//		const std::vector<AudioVelSample> &src_c_data = src[c]->data;
		//		std::vector<AudioSample> &dst_c_data = dst[c]->data;

		//		for(s_time t = 0; t < dst_chunk_size; t++)
		//		{
		//			//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
		//			//	No shift after -- easier to determine amount of overlap with each sample
		//			float src_t_center = (t + 0.5f)*dst_to_src;

		//			float	src_t_start = src_t_center - compression_radius,
		//					src_t_end = src_t_center + compression_radius;
		//			s_time	src_sample_start = floor(src_t_start),
		//					src_sample_end = floor(src_t_end);
		//			
		//			//Amount of overlap with first/last src samples (0 < overlap < 1)
		//			float	start_overlap = 1.0f - (src_t_start - src_sample_start),
		//					end_overlap = (src_t_end - src_sample_end);
		//			
		//			float	mult = start_overlap,
		//					sum = 0.0f;
		//			for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
		//			{
		//				amplitude += src_c_data[src_sample]*mult;
		//				sum += amplitude;
		//				mult = (src_sample + 2 == src_sample_end) ? end_overlap : 1.0f;
		//			}
		//			dst[c]->data[t] = dst_c_data[t]*add + (AudioSample)sum;
		//		}
		//	}
		//	break;
			

		default:
			status.setError(AS::ErrorType::GENERAL, "Error while sampling chunks -- Invalid sampling method.");
			break;
		}
	}

	return status;
}








//OLD AUDIOCHUNK SAMPLE FUNCTION//

/*
//Samples the given AudioChunks
AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add)
{
	AStatus status;

	if(!src || !src[0] || !dst || !dst[0])
	{
		status.setError(AS::ErrorType::GENERAL, "Either source or dest is null.");
		return status;
	}

	s_time	src_chunk_size = src[0]->size,
			dst_chunk_size = dst[0]->size;

	static const AudioChunk *defaultChunk = new AudioChunk(src_chunk_size);

	float	dst_to_src = (float)src_chunk_size/(float)dst_chunk_size;

	if(src_chunk_size == dst_chunk_size)	//Chunks are the same size, directly copy chunks
	{
		for(c_time c = 0; c < num_chunks; c++)
			*dst[c] = *src[c];
	}
	else if(dst_to_src < 1.0 || method == SampleMethod::POINT)	//Decompressing data, only interpolation needed (no sampling)  -OR- point sampling
	{
		for(c_time c = 0; c < num_chunks; c++)
		{
			for(s_time t = 0; t < dst_chunk_size; t++)
			{
				//Data represents the amplitude at the middle of the sample, so need to offset t by 0.5 after dst_to_src conversion
				float src_t = (float)t*dst_to_src - 0.5f;

				AudioSample data_sample;

				if(ceil(src_t) == src_t)
				{
					//src_offset is an integer --> no interpolation
					data_sample = src[c]->data[src_t];
				}
				else
				{
					s_time	low_index	= (s_time)floor(src_t),
							high_index	= (s_time)ceil(src_t);

					Point2f low((float)low_index,
								(float)(low_index < 0 ? src[c]->prevValue
													  : src[c]->data[low_index])),
							high((float)high_index,
								 (float)(high_index >= src_chunk_size ? (c + 1 < num_chunks ? src[c + 1]->data[high_index - src_chunk_size]
																							: defaultChunk->data[high_index - src_chunk_size])
																	  : src[c]->data[high_index]));
					
					data_sample = static_cast<AudioSample>(lerp(src_t, low, high));
				}

				if(add)	dst[c]->data[t] += data_sample;
				else	dst[c]->data[t] = data_sample;
				
			}
		}
	}
	else	//Compressing data, need to (down)sample
	{
		//Number src samples around each dst sample to average together
		float	compression_radius = dst_to_src*(1.0f/2.0f),
				src_to_dst = 1.0f/dst_to_src;
		
		float	last_val = 0.0f,
				vel = 0.0f;

		switch(method)
		{
		//Already handled (same as uncompressing data above)
		//case SampleMethod::POINT:
		//	break;
		case SampleMethod::AVERAGE:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = 1.0f - (src_sample_end - src_t_end);
					
					float	mult = start_overlap,
							sum = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						sum += src_c_data[src_sample]*mult;
						mult = (src_sample + 2 == src_sample_end) ? end_overlap : 1.0f;
					}
					dst[c]->data[t] = dst_c_data[t]*add + (AudioSample)(sum*src_to_dst);
				}
			}
			break;

		case SampleMethod::MAXIMUM_MAG:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = 1.0f - (src_sample_end - src_t_end);

					float max = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val*s_val > max*max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)max;
				}
			}
			break;

		case SampleMethod::PRESERVE_PEAKS:
			//CURRENTLY ONLY FINDS MAXIMUM MAGNITUDE PEAKS
			last_val = (float)src[0]->data[0],	//Last sample value
			vel = 0.0f;							//Direction the sample value is going
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					const float		src_t_center = (t + 0.5f)*dst_to_src,
									src_t_start = src_t_center - compression_radius,
									src_t_end = src_t_center + compression_radius;

					const s_time	src_sample_start = floor(src_t_start),
									src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					const float	start_overlap = 1.0f - (src_t_start - src_sample_start),
								end_overlap = 1.0f - (src_sample_end - src_t_end);

					float	max = 0.0f,
							//min = FLT_MAX,
							sum = 0.0f;
					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						float old_vel = vel;
						vel = s_val - last_val;

						//If local maximum (vel and old_vel opposite signs, and vel is toward zero (opposite sign as s_val))
						max = (vel*old_vel < 0.0 && vel*s_val < 0.0 && s_val*s_val > max*max) ? s_val : max;
						//min = (

						last_val = s_val;
						sum += s_val;
					}
					float avg = sum*src_to_dst;
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)(max*max > avg*avg ? max : avg);
				}
			}
			break;

		case SampleMethod::MAXIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				const float prev_sample = src[c]->prevValue,
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->data[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = 1.0f - (src_sample_end - src_t_end);
					
					//Include previous and next samples in the maximum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							max = (prev_overlap > next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						max = (s_val > max) ? s_val : max;
					}
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)max;
				}
			}
			break;

		case SampleMethod::MINIMUM:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				const float prev_sample = src[c]->prevValue,
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->data[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = 1.0f - (src_sample_end - src_t_end);
					
					//Include previous and next samples in the minimum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							min = (prev_overlap < next_overlap) ? prev_overlap : next_overlap;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						min = (s_val < min) ? s_val : min;
					}
					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)min;
				}
			}
			break;

		case SampleMethod::MAXIMUM_MAG_OR_AVERAGE:
			for(c_time c = 0; c < num_chunks; c++)
			{
				const std::vector<AudioSample> &src_c_data = src[c]->data;
				const float prev_sample = src[c]->prevValue,
							next_sample = (c + 1 < num_chunks) ? src[c + 1]->data[0] : src_c_data[src_chunk_size - 1];

				std::vector<AudioSample> &dst_c_data = dst[c]->data;

				for(s_time t = 0; t < dst_chunk_size; t++)
				{
					//Data represents the amplitude at the middle of the sample, so need to offset by 0.5 before dst_to_src conversion
					//	No shift after -- easier to determine amount of overlap with each sample
					float src_t_center = (t + 0.5f)*dst_to_src;

					float	src_t_start = src_t_center - compression_radius,
							src_t_end = src_t_center + compression_radius;
					s_time	src_sample_start = floor(src_t_start),
							src_sample_end = floor(src_t_end);
					
					//Amount of overlap with first/last src samples (0 < overlap < 1)
					float	start_overlap = 1.0f - (src_t_start - src_sample_start),
							end_overlap = 1.0f - (src_sample_end - src_t_end);
					
					//Include previous and next samples in the minimum, to connect samples
					float	prev_overlap = (float)(t == 0 ? prev_sample : src_c_data[src_sample_start - 1]),
							next_overlap = (float)(t + 1 == dst_chunk_size ? next_sample : src_c_data[src_sample_end]),
							
							min = (prev_overlap < next_overlap) ? prev_overlap : next_overlap,
							max = (prev_overlap > next_overlap) ? prev_overlap : next_overlap,
							avg = 0.0f;

					for(s_time src_sample = src_sample_start; src_sample < src_sample_end; src_sample++)
					{
						float s_val = (float)src_c_data[src_sample];
						min = (s_val < min) ? s_val : min;
						max = (s_val > max) ? s_val : max;
						avg += s_val;
					}
					avg *= src_to_dst;

					float value = ((min <= 0.0f && max <= 0.0f) ? min : ((min >= 0.0f && max >= 0.0f) ? max : avg));

					dst_c_data[t] = dst_c_data[t]*add + (AudioSample)value;
				}
			}
			break;

		default:
			status.setError(AS::ErrorType::GENERAL, "Error while sampling chunks -- Invalid sampling method.");
			break;
		}
	}

	return status;
}
*/