#ifndef APOLLO_INSTRUMENT_H
#define APOLLO_INSTRUMENT_H

#include "Audio.h"
#include "Interpolation.h"
#include "Waveform.h"
#include "Sampling.h"

#include <functional>

enum class InstrumentType
{
	INVALID = -1,

	WAVESYNTH = 0,

	COUNT
};

struct SampleState;
struct SampleInfo;

enum class ADSRInterpType
{
	INVALID = -1,

	LINEAR = 0,
	SPRING_SIM,

	COUNT
};
typedef ADSRInterpType ADSRInterp;

//Instrument base class
class Instrument
{
private:
protected:
public:
	Instrument();
	virtual ~Instrument() = default;

	//May be overriden if multiple sample functions are required
	virtual NoteSampleFunction getSampleFunction();
	virtual NoteSampleFunction getSampleVelFunction();

	//Returns whether the note is finished
	virtual bool sample(SampleState &state, const SampleInfo &info, AudioSample &out_value, bool add) = 0;
	virtual bool sampleVel(SampleState &state, const SampleInfo &info, AudioVelSample &out_value, bool add) = 0;
	virtual InstrumentType getType() const = 0;
};

//Instrument that samples a waveform for its base tambre (ADSR envelope)
class WaveSynth : public Instrument
{
private:

protected:
	Waveform			waveform;			//Waveform that is sampled for the base shape of the sound wave

	Time			attackTime,			//Amount of time for sound to reach peak volume level (seconds)
						decayTime,			//Amount of time after peak for sound to reach sustainLevel	(seconds)
						sustainLevel,		//Volume level that holds until key is released	(0.0 to 1.0)
						releaseTime;		//Time for volume level to go from sustainLevel to zero after key is released (seconds)
	
	Time			attackInv = 0.0,	//Inverse of attackTime
						decayInv = 0.0,		//Inverse of decayTime
						releaseInv = 0.0;	//Inverse of releaseTime

	//Constants for simulating spring motion through each state (to be set later)
	double				attackK = 1.0,
						attackB = 1.0,
						attackM = 1.0,

						decayK = 1.0,
						decayB = 1.0,
						decayM = 1.0,

						releaseK = 1.0,	//Release constants may have to be calculated on the fly (starting from arbitrary height)
						releaseB = 1.0,
						releaseM = 1.0;

	ADSRInterpType		interp = ADSRInterp::LINEAR;	//How the amplitude is interpolated within each stage

	//Sets up sampling variables depending on the interpolation type
	void setUpSampling();

public:

	WaveSynth();
	WaveSynth(Time attack_time, Time decay_time, Time sustain_level, Time release_time, ADSRInterpType interp_type);
	//WaveSynth(const WaveSynthDesc &ws_desc);
	virtual ~WaveSynth() = default;

	Waveform* getWaveform();

	void setInterpolationType(ADSRInterpType interp_type);
	
	virtual NoteSampleFunction getSampleFunction() override;
	virtual NoteSampleFunction getSampleVelFunction() override;

	//Default sample function -- LINEAR
	virtual bool sample(SampleState &state, const SampleInfo &info, AudioSample &out_value, bool add) override;
	virtual bool sampleVel(SampleState &state, const SampleInfo &info, AudioVelSample &out_value, bool add) override;
	bool sampleSpring(SampleState &state, const SampleInfo &info, AudioVelSample &out_value, bool add);

	virtual InstrumentType getType() const;
	
protected:
	//virtual void updateDesc() override;
};


#endif	//APOLLO_INSTRUMENT_H