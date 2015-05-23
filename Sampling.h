#ifndef APOLLO_SAMPLING_H
#define APOLLO_SAMPLING_H

#include "Timing.h"

typedef int MidiIndex;

typedef int16_t AudioSample;
typedef AudioSample AudioVelSample;

enum class SampleMethod
{
	INVALID			= -1,

	POINT			= 0,	//Samples the center point of the range
	AVERAGE,				//Averages the points within the range
	MAXIMUM_MAG,			//Finds the maximum magnitude of the points within the range
	PRESERVE_PEAKS,			//Finds the largest local maximum, or uses the average if there are none or the average is bigger
	MAXIMUM,				//Finds the maximum value within the range (each chunk overlaps by 1 sample)
	MINIMUM,				//Finds the minimum value within the range (each chunk overlaps by 1 sample)
	MAXIMUM_MAG_OR_AVERAGE,	//Finds min and max sample, and chooses one (or average)

	SUM,					//Finds the average, without dividing. Useful for velocities.


	COUNT
};

//Phases of a note being sampled
enum class NoteState : int
{
	NONE = 0,		//Note start

	ATTACKING = 1,
	DECAYING,
	SUSTAINING,
	RELEASING,

	DONE			//Note end
};

inline const NoteState operator+(const NoteState &ns, int inc)
{
	//int new_ns = static_cast<int>(ns) + inc;
	//return static_cast<NoteState>(new_ns*(new_ns < static_cast<int>(NoteState::COUNT)));

	return static_cast<NoteState>(static_cast<int>(ns) + inc);
}

inline const NoteState operator++(const NoteState &ns)
{
	return ns + 1;
}

struct SampleState;

//A stucture to control sample properties that are constant throughout wave sampling
struct SampleInfo
{
private:
	int			sampleRate = 0;
	double		sampleRateInv = 0.0;

	MidiIndex	index;

public:
	SampleInfo();
	SampleInfo(int sample_rate, MidiIndex midi_index);

	void setSampleRate(int sample_rate);
	int getSampleRate() const;

	//Returns inverse of sample rate (1.0/sampleRate)
	double getInvSampleRate() const;

	void setMidiIndex(MidiIndex midi_index);
	MidiIndex getMidiIndex() const;

	//next_note_state --> whather the state's NoteState should be incremented (and relevant variables reset)
	void stepState(SampleState &state, bool next_note_state) const;
};

//A structure to represent the current state of a sound wave being sampled
struct SampleState
{
private:
	const double	velocity = 0.0;					//Peak amplitude of sound wave

public:
	double			frequency	= 0.0,				//Current frequency of the sound wave			(in Hz)
					phase		= 0.0,				//Current phase of the sound wave				(0.0 to 1.0)
					amplitude	= 0.0,				//Current amplitude(volume) of the sound wave	(0.0 to 1.0)

					lastPhase = 0.0;				//Last phase of the wave
	AudioSample		lastSample = 0;					//Last sampled amplitude
	
	TimeRange		range = TimeRange(-1.0, -1.0);	//Range of the note (start --> end)				(seconds, or < 0 if not known)
	NoteState		state = NoteState::NONE;		//Which phase of the note this state is in (attack, decay, etc.)

	bool			started = false,				//Whether the note has started to be rendered
					newState = true;				//Set to true whenever the state has changed (and then back to false)

	double			ampVel = 0.0f;					//A variable to be used by sampling functions for keeping track of the amplitude's velocity

	SampleState();
	SampleState(double frequency_, double phase_, double amplitude_, double velocity, TimeRange note_range, NoteState note_state);
	~SampleState() = default;

	AudioVelSample getVelocity() const;

	void noteOff(double off_time);
	void noteReleased();

	//void reset();
};



#endif	//APOLLO_SAMPLING_H