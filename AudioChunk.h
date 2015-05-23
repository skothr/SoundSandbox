#ifndef APOLLO_AUDIO_CHUNK_H
#define APOLLO_AUDIO_CHUNK_H

#include "Audio.h"
#include "DataStatus.h"

class AudioAmpChunk;
class AudioVelChunk;

class AudioChunk
{
protected:
	s_time size;
	DataStatus status = DataStatus::DIRTY;

public:
	AudioChunk(s_time chunk_size);
	virtual ~AudioChunk();

	//TEMP//
	bool NOTE_ON = false;
	
	s_time getSize() const;
	DataStatus getStatus() const;
	void setStatus(DataStatus ds);

	virtual void loadZeros() = 0;
	
	virtual void getAmpData(std::vector<AudioSample> &out_data, AudioSample c_seed) const = 0;
	virtual void getVelData(std::vector<AudioVelSample> &out_data, AudioSample c_seed) const  = 0;
	
	friend class AudioData;
};

class AudioAmpChunk : public AudioChunk
{
protected:
	std::vector<AudioSample> data;

public:
	AudioAmpChunk(s_time chunk_size);
	AudioAmpChunk(const AudioAmpChunk &other);
	virtual ~AudioAmpChunk();
	
	const std::vector<AudioSample>* getData() const;
	std::vector<AudioSample>* getDataRef();
	virtual void loadZeros() override;

	void clamp(AudioSample min_amp, AudioSample max_amp);

	void offset(AudioSample offset);

	void setData(const AudioAmpChunk &other);
	void setData(const std::vector<AudioSample> &c_data);
	
	virtual void getAmpData(std::vector<AudioSample> &out_data, AudioSample c_seed = 0) const override;
	virtual void getVelData(std::vector<AudioVelSample> &out_data, AudioSample c_seed) const override;

	AudioAmpChunk& operator=(const AudioAmpChunk &other);

	AudioAmpChunk& operator+=(const AudioAmpChunk &rhs);
	const AudioAmpChunk& operator+(const AudioAmpChunk &rhs) const;
	AudioAmpChunk& operator-=(const AudioAmpChunk &rhs);
	const AudioAmpChunk& operator-(const AudioAmpChunk &rhs) const;
	
	AudioAmpChunk& operator+=(const AudioVelChunk &rhs);
	//const AudioAmpChunk& operator+(const AudioVelChunk &rhs) const;
	AudioAmpChunk& operator-=(const AudioVelChunk &rhs);
	//const AudioAmpChunk& operator-(const AudioVelChunk &rhs) const;
	
	friend class AudioData;
	friend class AudioAmpData;
	friend class AudioDataBuffer;
	friend class AudioAmpDataBuffer;
	
	friend class AudioVelChunk;
};

class AudioVelChunk : public AudioChunk
{
protected:
	std::vector<AudioVelSample> data;
	AudioVelSample				chunkStep = 0;	//Total velocity throughout this chunk (from start to end)

	AudioSample					seed;
	DataStatus					seedStatus = DataStatus::DIRTY;

public:
	AudioVelChunk(s_time chunk_size, AudioSample chunk_seed);
	AudioVelChunk(const AudioVelChunk &other);
	virtual ~AudioVelChunk();
	
	AudioVelSample getChunkStep() const;

	void updateChunkStep();

	void setSeed(AudioSample new_seed);
	AudioSample getSeed() const;

	const std::vector<AudioVelSample>* getData() const;
	std::vector<AudioVelSample>* getDataRef();
	virtual void loadZeros() override;
	
	void setData(const AudioVelChunk &other);
	void setData(const std::vector<AudioVelSample> &c_data);
	
	virtual void getAmpData(std::vector<AudioSample> &out_data, AudioSample c_seed) const override;
	virtual void getVelData(std::vector<AudioVelSample> &out_data, AudioSample c_seed) const override;

	AudioVelChunk& operator=(const AudioVelChunk &other);

	AudioVelChunk& operator+=(const AudioVelChunk &rhs);
	const AudioVelChunk& operator+(const AudioVelChunk &rhs) const;
	AudioVelChunk& operator-=(const AudioVelChunk &rhs);
	const AudioVelChunk& operator-(const AudioVelChunk &rhs) const;
	
	//AudioVelChunk& operator+=(const AudioAmpChunk &rhs);
	//const AudioVelChunk& operator+(const AudioAmpChunk &rhs) const;
	//AudioVelChunk& operator-=(const AudioAmpChunk &rhs);
	//const AudioVelChunk& operator-(const AudioAmpChunk &rhs) const;

	friend class AudioData;
	friend class AudioVelData;
	friend class AudioDataBuffer;
	friend class AudioVelDataBuffer;
	
	friend class RenderNode;
	friend class SpeakerNode;

	friend class AudioAmpChunk;
};


#endif	//APOLLO_AUDIO_CHUNK_H