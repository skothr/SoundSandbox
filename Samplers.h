#ifndef APOLLO_SAMPLERS_H
#define APOLLO_SAMPLERS_H

#include <vector>

#include "MIDI.h"
#include "Audio.h"
#include "Waveform.h"
#include "Filter.h"

class MidiSampler
{
private:
	std::vector<MidiNote> notes;

	Waveform *waveform;

public:
	MidiSampler();
	MidiSampler(Waveform *wave);

	void setWaveform(Waveform *wave);

	void getChunk(AudioSample *p_data, s_time data_start, int num_samples, int sample_rate, const std::vector<Modifier*> &mods);

};

#endif	//APOLLO_SAMPLERS_H