#ifndef APOLLO_AUDIO_DATA_H
#define APOLLO_AUDIO_DATA_H

#include "Audio.h"
#include "Range.h"
#include "DataStatus.h"

class Path;

class AudioChunk;
class AudioAmpChunk;
class AudioVelChunk;

/////AUDIO DATA/////
//Base class for audio data//
class AudioData
{
protected:
	s_time chunkSize;

public:
	AudioData(s_time chunk_size = AUDIO_CHUNK_SIZE);
	virtual ~AudioData();
	
	s_time getChunkSize() const;

	virtual c_time getNumChunks() const = 0;

	virtual void loadZeros() = 0;
	virtual void resize(c_time new_num_chunks) = 0;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const = 0;

	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const = 0;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const = 0;
	/*
	//Square brackets return chunk pointer at c_index
	ChunkType* operator[](c_time c_index);
	const ChunkType* operator[](c_time c_index) const;

	//Parentheses return chunk reference at c_index
	ChunkType& operator()(c_time c_index);
	const ChunkType& operator()(c_time c_index) const;
	*/

	virtual void writeToFile(const Path &file_path) = 0;
	virtual void readFromFile(const Path &file_path) = 0;

};


//Represents audio data with sample amplitudes
class AudioAmpData : public AudioData
{
protected:
	std::vector<AudioAmpChunk*> data;

public:
	AudioAmpData(s_time chunk_size = AUDIO_CHUNK_SIZE, c_time num_chunks = 1);
	//AudioSampleData(s_time chunk_size, const std::vector<AudioChunk*> &initial_data);
	virtual ~AudioAmpData();

	virtual c_time getNumChunks() const override;

	std::vector<AudioAmpChunk*>* getData();


	virtual void loadZeros() override;
	virtual void resize(c_time new_num_chunks) override;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const override;

	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const override;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const override;

	AudioAmpData& operator=(const AudioAmpData &other);
	
	//Square brackets return chunk pointer at c_index
	AudioAmpChunk* operator[](c_time c_index);
	const AudioAmpChunk* operator[](c_time c_index) const;

	//Parentheses return chunk reference at c_index
	AudioAmpChunk& operator()(c_time c_index);
	const AudioAmpChunk& operator()(c_time c_index) const;
	
	virtual void writeToFile(const Path &file_path) override;
	virtual void readFromFile(const Path &file_path) override;
};

//Represents audio data with sample velocities and a seed value
class AudioVelData : public AudioData
{
protected:
	std::vector<AudioVelChunk*> data;
	AudioSample seed = 0.0f;

public:
	AudioVelData(s_time chunk_size = AUDIO_CHUNK_SIZE, c_time num_chunks = 1, AudioSample primary_seed = 0);
	AudioVelData(const AudioVelData &other);
	virtual ~AudioVelData();

	virtual c_time getNumChunks() const override;
	
	std::vector<AudioVelChunk*>* getData();
	
	AudioSample getSeed() const;
	void setSeed(AudioSample new_seed);

	void updateChunkSeeds();
	
	virtual void loadZeros() override;
	virtual void resize(c_time new_num_chunks) override;
	
	virtual void sampleData(std::vector<float> &out_amplitudes, float sample_start, float sample_step, int num_samples) const override;
	
	virtual void getAmpData(const std::vector<AudioAmpChunk*> &out_data, ChunkRange range) const override;
	virtual void getVelData(const std::vector<AudioVelChunk*> &out_data, ChunkRange range) const override;

	AudioVelData& operator=(const AudioVelData &other);
	
	//Square brackets return chunk pointer at c_index
	AudioVelChunk* operator[](c_time c_index);
	const AudioVelChunk* operator[](c_time c_index) const;

	//Parentheses return chunk reference at c_index
	AudioVelChunk& operator()(c_time c_index);
	const AudioVelChunk& operator()(c_time c_index) const;
	
	virtual void writeToFile(const Path &file_path) override;
	virtual void readFromFile(const Path &file_path) override;
};




#endif	//APOLLO_AUDIO_DATA_H