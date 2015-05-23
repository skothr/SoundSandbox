#include "Sampling.h"

#include "MIDI.h"
#include "Timing.h"

/////SAMPLE INFO/////

SampleInfo::SampleInfo()
{ }

SampleInfo::SampleInfo(int sample_rate, MidiIndex midi_index)
	: sampleRate(sample_rate), sampleRateInv(1.0/(double)sampleRate), index(midi_index)
{ }

void SampleInfo::setSampleRate(int sample_rate)
{
	sampleRate = sample_rate;
	sampleRateInv = 1.0/(double)sampleRate;
}
int SampleInfo::getSampleRate() const
{
	return sampleRate;
}

double SampleInfo::getInvSampleRate() const
{
	return sampleRateInv;
}

void SampleInfo::setMidiIndex(MidiIndex midi_index)
{
	index = midi_index;
}
MidiIndex SampleInfo::getMidiIndex() const
{
	return index;
}

void SampleInfo::stepState(SampleState &state, bool next_note_state) const
{
	//Save old phase
	state.lastPhase = state.phase;
	
	//Step phase
	state.phase += state.frequency*sampleRateInv;

	//Clamp to between 0.0 and 1.0
	state.phase = (state.phase >= 1.0 ? state.phase - 1.0 : state.phase);
	
	//Step note state if next_note_state is true
	state.state = state.state + (int)next_note_state;

	state.started = true;
}


/////SOUND SAMPLE/////

SampleState::SampleState()
{ }
SampleState::SampleState(double frequency_, double phase_, double amplitude_, double velocity_, TimeRange note_range, NoteState note_state)
	: frequency(frequency_), phase(phase_), amplitude(amplitude_), velocity(velocity_), range(note_range), state(note_state)
{ }

AudioVelSample SampleState::getVelocity() const
{
	return velocity;
}

void  SampleState::noteOff(double off_time)
{
	//Now releasing note
	//state = NoteState::RELEASING;
	range.end = ((range.end < 0.0 || range.end > off_time) ? off_time : range.end);
}

void SampleState::noteReleased()
{
	state = NoteState::RELEASING;
	newState = true;
}

/*
void SampleState::reset()
{
	amplitude = 0.0;
	ampVel = 0.0;
	lastPhase = 0.0;
	lastSample = 0;
	newState = false;
	phase = 0.0;
}
*/