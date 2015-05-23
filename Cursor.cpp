#include "Cursor.h"



/////CURSOR/////

Cursor::Cursor(int sample_rate, s_time chunk_size, c_time chunk_step)
	: sampleRate(sample_rate), chunkSize(chunk_size), chunkStep(chunk_step),
		sampleRateInv(1.0/(double)sample_rate), chunkSizeInv(1.0/(double)chunk_size),
		sampleRange(0, chunk_size*chunk_step), chunkRange(0, chunk_step), timeRange(0.0, 0.0)
{
	timeRange.end = sampleRateInv;
}

Cursor::Cursor(const CursorDesc &c_desc)
	: Cursor(c_desc.sampleRate, c_desc.chunkSize, c_desc.chunkStep)
{ }

void Cursor::update(s_time new_sample_start)
{
	sampleRange.start = new_sample_start;
	sampleRange.end = sampleRange.start + chunkSize*chunkStep;

	chunkRange.start = sampleRange.start*chunkSizeInv;
	chunkRange.end = chunkRange.start + chunkStep;

	timeRange.start = (Time)sampleRange.start*sampleRateInv;
	timeRange.end = (Time)sampleRange.end*sampleRateInv;
}

void Cursor::step()
{
	update(sampleRange.start + chunkSize*chunkStep);
}

void Cursor::setSampleRate(int new_sample_rate)
{
	//TODO: Adjust sample start time to compensate for new sample rate?

	sampleRate = new_sample_rate;
	sampleRateInv = 1.0/(double)sampleRate;
	
	//Update other values
	timeRange.start = (Time)sampleRange.start*sampleRateInv;
	timeRange.end = (Time)sampleRange.end*sampleRateInv;
}

void Cursor::setChunkSize(s_time new_chunk_size)
{
	chunkSize = new_chunk_size;
	chunkSizeInv = 1.0/(double)chunkSize;

	//Update other values
	sampleRange.end = sampleRange.start + chunkSize*chunkStep;
	timeRange.end = sampleRange.end*sampleRateInv;
}

void Cursor::setChunkStep(c_time new_chunk_step)
{
	chunkStep = new_chunk_step;

	//Update otehr values
	sampleRange.end = sampleRange.start + chunkSize*chunkStep;
	chunkRange.end = chunkRange.start + chunkStep;
	timeRange.end = sampleRange.end*sampleRateInv;
}


int Cursor::getSampleRate() const
{
	return sampleRate;
}

s_time Cursor::getChunkSize() const
{
	return chunkSize;
}

c_time Cursor::getChunkStep() const
{
	return chunkStep;
}


void Cursor::setTimeSamples(s_time sample_time)
{
	update(sample_time);
}

void Cursor::setTimeChunks(c_time chunk_time)
{
	update((s_time)((double)chunk_time*chunkSize));
}

void Cursor::setTimeSeconds(double time)
{
	update((s_time)(time*sampleRateInv));
}

SampleRange Cursor::getSampleRange() const
{
	return sampleRange;
}

ChunkRange Cursor::getChunkRange() const
{
	return chunkRange;
}

TimeRange Cursor::getTimeRange() const
{
	return timeRange;
}

void Cursor::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new CursorDesc()));
	CursorDesc *desc = dynamic_cast<CursorDesc*>(objDesc);
	
	desc->sampleRate = sampleRate;
	desc->s_range = sampleRange;
	desc->chunkSize = chunkSize;
	desc->chunkStep = chunkStep;
}

