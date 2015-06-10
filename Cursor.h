#ifndef APOLLO_CURSOR_H
#define APOLLO_CURSOR_H

//#include "Saveable.h"
#include "Timing.h"

class Cursor// : public Saveable
{
private:
	SampleRange sampleRange;
	ChunkRange	chunkRange;
	TimeRange	timeRange;
	//TimeRange	globalTimeRange;

	int			sampleRate;
	s_time		chunkSize;
	c_time		chunkStep;

	double		sampleRateInv,
				chunkSizeInv;
	
	bool		active = false,
				just_activated = false;

	//Updates other ranges based on new sample start value
	void update(s_time new_sample_start);

public:
	Cursor(int sample_rate, s_time chunk_size, c_time chunk_step);
	//Cursor(const CursorDesc &c_desc);

	//Steps cursor position based off current values
	void step();

	//Steps only the globalTimeRange.
	//void stepGlobal();

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

	//TimeRange getGlobalTimeRange() const;

	//These functions switch back and forth between global/local cursor times
	//Time convertToLocal(Time global_t) const;
	//Time convertToGlobal(Time local_t) const;
	//TimeRange convertToLocal(TimeRange global_range) const;
	//TimeRange convertToGlobal(TimeRange local_range) const;

	void setActive(bool is_active);
	bool isActive() const;
	
protected:
	//virtual void updateDesc() override;

	friend class SpeakerNode;
};



#endif	//APOLLO_CURSOR_H