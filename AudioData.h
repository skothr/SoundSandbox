#ifndef APOLLO_AUDIO_DATA_H
#define APOLLO_AUDIO_DATA_H

#include "Audio.h"
#include "Range.h"
#include "DataStatus.h"


class AudioChunk;
class AudioAmpChunk;
class AudioVelChunk;

//Base class for audio data
class AudioData
{
protected:
	s_time chunkSize;

public:
	AudioData(s_time chunk_size);
	virtual ~AudioData();
	
	s_time getChunkSize() const;

	virtual c_time getNumChunks() const = 0;

	virtual DataStatus getChunkStatus(c_time c_index) const = 0;

	virtual void loadZeros() = 0;
	virtual void resize(c_time new_num_chunks) = 0;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const = 0;

	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const = 0;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const = 0;
};

//Represents audio data with sample amplitudes
class AudioAmpData : public AudioData
{
protected:
	std::vector<AudioAmpChunk*> data;

public:
	AudioAmpData(s_time chunk_size, c_time num_chunks);
	//AudioSampleData(s_time chunk_size, const std::vector<AudioChunk*> &initial_data);
	virtual ~AudioAmpData();

	AudioAmpChunk* getChunk(c_time c_index);	//const?

	virtual c_time getNumChunks() const override;

	std::vector<AudioAmpChunk*>* getData();

	virtual DataStatus getChunkStatus(c_time c_index) const override;
	
	virtual void loadZeros() override;
	virtual void resize(c_time new_num_chunks) override;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const override;

	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const override;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const override;

	AudioAmpData& operator=(const AudioAmpData &other);
};

//Represents audio data with sample velocities and a seed value
class AudioVelData : public AudioData
{
protected:
	std::vector<AudioVelChunk*> data;

	AudioSample seed = 0.0f;

public:
	AudioVelData(s_time chunk_size, c_time num_chunks, AudioSample primary_seed);
	virtual ~AudioVelData();
	
	AudioVelChunk* getChunk(c_time c_index);	//const?

	virtual c_time getNumChunks() const override;
	
	std::vector<AudioVelChunk*>* getData();
	
	virtual DataStatus getChunkStatus(c_time c_index) const override;

	AudioSample getSeed() const;
	void setSeed(AudioSample new_seed);

	void updateChunkSeeds();
	
	virtual void loadZeros() override;
	virtual void resize(c_time new_num_chunks) override;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const override;
	
	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const override;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const override;

	AudioVelData& operator=(const AudioVelData &other);
};




#endif	//APOLLO_AUDIO_DATA_H