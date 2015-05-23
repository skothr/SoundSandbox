#ifndef APOLLO_CURSOR_H
#define APOLLO_CURSOR_H

#include "Saveable.h"
#include "Timing.h"

class Cursor : public Saveable
{
private:
	SampleRange sampleRange;
	ChunkRange	chunkRange;
	TimeRange	timeRange;

	int			sampleRate;
	s_time		chunkSize;
	c_time		chunkStep;

	double		sampleRateInv,
				chunkSizeInv;

	//Updates other ranges based on new sample start value
	void update(s_time new_sample_start);

public:
	bool		active = false;

	Cursor(int sample_rate, s_time chunk_size, c_time chunk_step);
	Cursor(const CursorDesc &c_desc);

	//Steps cursor position based off current values
	void step();

	void setSampleRate(int new_sample_rate);
	void setChunkSize(s_time new_chunk_size);
	void setChunkStep(c_time new_chunk_step);

	int getSampleRate() const;
	s_time getChunkSize() const;
	c_time getChunkStep() const;
	
	void setTimeSamples(s_time sample_time);
	void setTimeChunks(c_time chunk_time);
	void setTimeSeconds(double time);

	SampleRange getSampleRange() const;
	ChunkRange getChunkRange() const;
	TimeRange getTimeRange() const;
	
protected:
	virtual void updateDesc() override;
};



#endif	//APOLLO_CURSOR_H