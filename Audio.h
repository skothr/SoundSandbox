#ifndef APOLLO_AUDIO_H
#define APOLLO_AUDIO_H

#include "Timing.h"

//Determines the size of an audio chunk.
extern s_time	AUDIO_CHUNK_SIZE,
				DEFAULT_SAMPLE_RATE;
//Determines the cutoff for audio amplitude (value of a sample)
extern float AUDIO_MAX_AMPLITUDE;

//16-bit int -- represents the amplitude of an audio wave at a point in time (sample)
typedef int16_t AudioSample;
//AudioSample as a float, for decimal storage
typedef double FAudioSample;

//float -- represents a change in amplitude between two AudioSamples
typedef AudioSample AudioVelSample;
typedef FAudioSample FAudioVelSample;






/*
class AudioChunk;
class AudioVelChunk;

//Vector of AudioChunks (used for AudioChunk storage and communication)
typedef std::vector<AudioChunk*> AudioData;
typedef std::vector<AudioVelChunk*> AudioVelData;

typedef std::vector<AudioSample>	AudioChunkData;
typedef std::vector<AudioVelSample>	AudioVelChunkData;


class DataChunk
{
private:
protected:
	s_time		size;						//Number of samples/data points in this chunk
	DataStatus	status = DataStatus::DIRTY;	//Status of this chunk

public:
	DataChunk	*prevChunk = nullptr,		//Previous chunk in track/buffer/etc
				*nextChunk = nullptr;		//Next chunk in track/buffer/etc

	//TEST//
	bool		NOTE_ON = false;

	DataChunk(s_time chunk_size);
	DataChunk(const DataChunk &other);
	virtual ~DataChunk() = default;

	DataStatus getStatus() const;
	virtual void setStatus(DataStatus d_status);

	s_time getSize() const;

	//Sets previous/next chunk pointers
	virtual void setPointers(DataChunk *p_prev, DataChunk *p_next);

	//Last sample of previous chunk
	virtual AudioSample getPreviousSample() const = 0;
	//First sample of this chunk
	virtual AudioSample getFirstSample() const = 0;
	//Last sample of this chunk
	virtual AudioSample getLastSample() const = 0;
	//First sample of next chunk
	virtual AudioSample getNextSample() const = 0;

	
	virtual ChunkType getType() const = 0;


	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
};

class AudioChunk : public DataChunk
{
private:
protected:
	AudioChunkData		aData;		//Amplitude data

public:
	AudioChunk(s_time chunk_size);
	AudioChunk(const AudioChunk &other);
	virtual ~AudioChunk() = default;
	
	//Sets all data to 0
	void loadZeros();

	//Audio(amplitude) data
	const AudioSample* getData() const;
	void getData(AudioChunkData &output) const;
	void setData(const AudioChunkData &new_data);
	
	//Velocity data
	AudioVelChunk getVelChunk() const;
	//void getVelChunk(AudioVelChunk &output) const;
	AudioVelChunkData getVelData() const;
	void getVelData(AudioVelChunkData &output) const;

	//Get/set individual samples
	AudioSample getSample(s_time index) const;
	AudioSample& getSampleRef(s_time index);
	void setSample(s_time index, AudioSample s);
	
	virtual AudioSample getPreviousSample() const override;
	virtual AudioSample getFirstSample() const override;
	virtual AudioSample getLastSample() const override;
	virtual AudioSample getNextSample() const override;

	virtual ChunkType getType() const override
	{ return ChunkType::AUDIO; }
	
	//////////////////////////////
	///// OPERATOR OVERLOADS /////
	//////////////////////////////

	AudioChunk& operator=(const AudioChunk &other);

	AudioSample& operator[](unsigned int index);
	const AudioSample& operator[](unsigned int index) const;

	AudioChunk operator-() const;

	AudioChunk& operator+=(const AudioChunk &rhs);
	AudioChunk operator+(const AudioChunk &rhs) const;
	
	AudioChunk& operator-=(const AudioChunk &rhs);
	AudioChunk operator-(const AudioChunk &rhs) const;

	
	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
};

class AudioVelChunk : public DataChunk
{
	friend class AudioChunk;

private:
	AudioVelChunk	*prevVelChunk = nullptr,
					*nextVelChunk = nullptr;

	//To prevent infinite recursion when updating chunk loops
	bool			updating = false,
					dirtying = false;

	//Marks seed as dirty, and propogates to all chunks down the list (forward)
	void setDirtySeed();

protected:
	AudioVelChunkData	vData;							//Sample derivative data
	AudioVelSample		totalVel = 0.0f;				//Velocity from first to last sample

	AudioSample			seedSample = 0;					//Seed for this chunk (to turn vel data into amp data)
	DataStatus			seedStatus = DataStatus::CLEAN;	//Status of the seed sample

public:
	AudioVelChunk(s_time chunk_size, AudioSample seed);
	AudioVelChunk(const AudioVelChunk &other);
	virtual ~AudioVelChunk() = default;
	
	//Sets all data to 0
	void loadZeros();

	//Sets seed (only if prevChunk is null)
	void setSeed(AudioSample seed);
	AudioSample getSeed() const;

	//Updates values dependent only on the inside of this chunk (e.g. totalVel)
	void updateInside();
	//Updates values dependent only on the outside of this chunk (e.g. seed)
	void updateOutside();
	//Updates all values
	void updateAll();
	
	//Velocity data
	const AudioVelSample* getData() const;
	void getData(AudioVelChunkData &output) const;
	void setData(const AudioVelChunkData &new_data);

	//Audio(amplitude) data
	AudioChunk getAudioChunk() const;
	//void getAudioChunk(AudioChunk &output) const;
	AudioChunkData getAudioData() const;
	void getAudioData(AudioChunkData &output) const;
	
	//Get/set individual samples
	AudioVelSample getSample(s_time index) const;
	AudioVelSample& getSampleRef(s_time index);
	void setSample(s_time index, AudioVelSample v);
	
	//WARNING: These won't update seed.
	virtual AudioSample getPreviousSample() const override;
	virtual AudioSample getFirstSample() const override;
	virtual AudioSample getLastSample() const override;
	virtual AudioSample getNextSample() const override;

	//Sets previous/next chunk pointers and updates seed (if applicable)
	virtual void setPointers(DataChunk *p_prev, DataChunk *p_next) override;
	void setPrevChunk(DataChunk *p_prev);
	void setNextChunk(DataChunk *p_next);
	
	//////////////////////////////
	///// OPERATOR OVERLOADS /////
	//////////////////////////////

	AudioVelChunk& operator=(const AudioVelChunk &other);

	//AudioVelSample& operator[](unsigned int index);
	const AudioVelSample& operator[](unsigned int index) const;
	
	AudioVelChunk operator-() const;
	
	AudioVelChunk& operator+=(const AudioVelChunk &rhs);
	AudioVelChunk operator+(const AudioVelChunk &rhs) const;
	
	AudioVelChunk& operator-=(const AudioVelChunk &rhs);
	AudioVelChunk operator-(const AudioVelChunk &rhs) const;

	AudioVelChunk& operator+=(const AudioChunk &rhs);
	AudioVelChunk& operator-=(const AudioChunk &rhs);


	virtual ChunkType getType() const
	{ return ChunkType::AUDIO_VEL; }
	
	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
};




/*
//Base class for Audio Chunk types
class AChunk
{
	//Default value for when a chunk's prevChunk pointer is null
	static const FAudioSample DEFAULT_VALUE;

	//Returns last sample of prevChunk
	AudioSample getPreviousSample() const;
	//Returns first sample of nextChunk
	AudioSample getNextSample() const;
	//Returns first sample of this chunk
	virtual AudioSample getFirstSample() = 0;
	//Returns last sample of this chunk
	virtual AudioSample getLastSample() = 0;
	
	//Sets previous/next chunk pointers
	virtual void setPointers(AChunk *p_prev, AChunk *p_next);
	
	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
};

//Represents a chunk of playable audio data --> each point represents an audio sample
class AudioChunk : public AChunk
{
private:

protected:

public:
	std::vector<AudioSample>	data;			//Sample data

	AudioChunk(s_time chunk_size);
	AudioChunk(const AudioChunk &other);
	virtual ~AudioChunk() = default;
	
	//Returns first sample of this chunk
	virtual AudioSample getFirstSample() override;
	//Returns last sample of this chunk
	virtual AudioSample getLastSample() override;
	
	//Returns the resulting AudioVelChunk for this chunk
	AudioVelChunk getAudioVelChunk() const;
	void getAudioVelChunk(AudioVelChunk &output) const;

	const AudioSample* getData(SampleRange r) const;
	void setData(SampleRange r, const AudioSample *new_data);

	void loadZeros();

	AudioSample getSample(s_time index) const;
	void setSample(s_time index, AudioSample s);

	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);

	virtual AChunkType getType() const override
	{ return AChunkType::AUDIO; }

	//////////////////////////////
	///// OPERATOR OVERLOADS /////
	//////////////////////////////

	//ASSIGNMENT//
	AudioChunk& operator=(const AudioChunk &other);

	//NEGATION//
	AudioChunk operator-() const;

	//ADDITION//
	AudioChunk& operator+=(const AudioChunk &rhs);
	AudioChunk& operator+=(const AudioVelChunk &rhs);
	AudioChunk operator+(const AudioChunk &rhs) const;
	AudioChunk operator+(const AudioVelChunk &rhs) const;

	//SUBTRACTION//
	AudioChunk& operator-=(const AudioChunk &rhs);
	AudioChunk& operator-=(const AudioVelChunk &rhs);
	AudioChunk operator-(const AudioChunk &rhs) const;
	AudioChunk operator-(const AudioVelChunk &rhs) const;

	//SCALING
	AudioChunk& operator*=(const float &rhs);
	AudioChunk operator*(const float &rhs) const;
	AudioChunk& operator/=(const float &rhs);
	AudioChunk operator/(const float &rhs) const;
};

//Represents the derivative of an AudioChunk --> each point represents the amplitude change since the last sample
class AudioVelChunk : public AChunk
{
	friend class AudioChunk;

private:
protected:
	std::vector<AudioVelSample>	vData;		//Sample derivative data

	FAudioSample	prevSample = 0,			//First sample in this chunk
					lastSample = 0;			//Last sample in this chunk

public:
	bool	prevClean = false,			//Whether firstSample is clean (updated)
			lastClean = false,			//Whether lastSample is clean (updated)
			cleaning = false;			//Whether this node is currently cleaning its first/last samples

	AudioVelChunk(s_time chunk_size);
	AudioVelChunk(const AudioVelChunk &other);
	virtual ~AudioVelChunk() = default;

	//Returns first sample of this chunk
	virtual AudioSample getFirstSample() override;
	//Returns last sample of this chunk
	//WARNING -- Has to compute every value in chunk. Use sparingly.
	virtual AudioSample getLastSample() override;

	//Updates previous chunks in order to calculate firstSample, as well as body/lastSample
	void updateAll();
	//Updates lastSample using firstSample (assumes firstSample is already clean!!)
	void updateBody();

	//Sets previous/next chunk pointers and updates values.
	virtual void setPointers(AChunk *p_prev, AChunk *p_next) override;
	
	//Returns the resulting AudioChunk when this chunk is applied with the given seed value
	//																(last value in preceding chunk)
	AudioChunk getAudioChunk();
	//WARNING: Wont update outdated values
	AudioChunk getAudioChunkConst() const;
	void getAudioChunk(AudioChunk &output);


	const AudioVelSample* getData(SampleRange r) const;
	void setData(SampleRange r, const AudioVelSample *new_data);

	void loadZeros();
	
	AudioVelSample getSample(s_time index) const;
	AudioVelSample& getSampleReference(s_time index);
	void setSample(s_time index, AudioVelSample s);
	
	friend AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
	friend AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);

	virtual AChunkType getType() const
	{ return AChunkType::AUDIO_VEL; }



	//////////////////////////////
	///// OPERATOR OVERLOADS /////
	//////////////////////////////

	//ASSIGNMENT//
	AudioVelChunk& operator=(const AudioVelChunk &other);

	//NEGATION//
	AudioVelChunk operator-() const;

	//ADDITION//
	AudioVelChunk& operator+=(const AudioVelChunk &rhs);
	AudioVelChunk& operator+=(const AudioChunk &rhs);
	AudioVelChunk operator+(const AudioVelChunk &rhs) const;
	AudioVelChunk operator+(const AudioChunk &rhs) const;

	//SUBTRACTION//
	AudioVelChunk& operator-=(const AudioVelChunk &rhs);
	AudioVelChunk& operator-=(const AudioChunk &rhs);
	AudioVelChunk operator-(const AudioVelChunk &rhs) const;
	AudioVelChunk operator-(const AudioChunk &rhs) const;

	//SCALING
	AudioVelChunk& operator*=(const float &rhs);
	AudioVelChunk operator*(const float &rhs) const;
	AudioVelChunk& operator/=(const float &rhs);
	AudioVelChunk operator/(const float &rhs) const;
};


//Resizes the AudioChunks in data.
//AudioData resizeAudioChunks(AudioData &data);

//Samples the given AudioChunks
AStatus sampleChunks(const AudioChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
//Samples the given AudioVelChunks
AStatus sampleChunks(const AudioVelChunk **src, AudioChunk **dst, c_time num_chunks, SampleMethod method, bool add);
*/

#endif	//APOLLO_AUDIO_H