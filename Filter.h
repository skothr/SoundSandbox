#ifndef APOLLO_FILTER_H
#define APOLLO_FILTER_H

#include <functional>

#include "Sampling.h"
#include "Audio.h"
#include "Timing.h"

class AudioVelChunk;

//Function used to modify a SampleInfo so as to apply a base effect (before rendering a waveform)
typedef std::function<void (SampleState& state, const SampleInfo &info, double t)> SampleModFunction;
//Function used to modify audio data
typedef std::function<void (const AudioVelChunk **input, AudioVelChunk **output, double t)> AudioModFunction;

class Modifier
{
private:

public:
	Modifier();

	virtual void modify(SampleState &state, const SampleInfo &info, double t) = 0;
};

class ConstMod : public Modifier
{
private:

public:
	ConstMod();
	virtual void modify(SampleState &state, const SampleInfo &info, double t);
};



#endif	//APOLLO_FILTER_H