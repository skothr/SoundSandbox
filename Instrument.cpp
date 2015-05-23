#include "Instrument.h"

#include "Sampling.h"
#include <exception>

/////INSTRUMENT/////

Instrument::Instrument()
{ }

NoteSampleFunction Instrument::getSampleFunction()
{
	return std::bind(&Instrument::sample, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

NoteSampleFunction Instrument::getSampleVelFunction()
{
	return std::bind(&Instrument::sampleVel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}





/////WAVE SYNTH/////

WaveSynth::WaveSynth()
	: WaveSynth(0.01, 0.4, 0.3, 0.1, ADSRInterpType::LINEAR)
{ }
WaveSynth::WaveSynth(Time attack_time, Time decay_time, Time sustain_level, Time release_time, ADSRInterpType interp_type)
	: attackTime(attack_time), decayTime(decay_time), sustainLevel(sustain_level), releaseTime(release_time),
		interp(interp_type), attackInv(1.0/attack_time), decayInv(1.0/decay_time), releaseInv(1.0/release_time)
{
	setUpSampling();
}

WaveSynth::WaveSynth(const WaveSynthDesc &ws_desc)
	: WaveSynth(ws_desc.attackTime, ws_desc.decayTime, ws_desc.sustainLevel, ws_desc.releaseTime, ws_desc.interp)
{
	waveform = Waveform(ws_desc.waveform);
}

void WaveSynth::setUpSampling()
{
	switch(interp)
	{
	case ADSRInterp::LINEAR:
		//Reset unneeded variables
		break;

	case ADSRInterp::SPRING_SIM:
		//Set up spring system

		//ATTACK//



		break;
	default:
		break;
	}
}

Waveform* WaveSynth::getWaveform()
{
	return &waveform;
}

void WaveSynth::setInterpolationType(ADSRInterpType interp_type)
{
	interp = interp_type;
	setUpSampling();
}

NoteSampleFunction WaveSynth::getSampleFunction()
{
	switch(interp)
	{
	case ADSRInterp::LINEAR:
		return std::bind(&WaveSynth::sample, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	case ADSRInterp::SPRING_SIM:
		return std::bind(&WaveSynth::sampleSpring, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	default:
		return nullptr;
	}
}

NoteSampleFunction WaveSynth::getSampleVelFunction()
{
	return std::bind(&WaveSynth::sampleVel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

bool WaveSynth::sample(SampleState &state, const SampleInfo &info, AudioSample &out_value, bool add)
{
	/*
	double	s_attack = (double)(state.state == NoteState::ATTACKING),
			s_decay = (double)(state.state == NoteState::DECAYING),
			s_sustain = (double)(state.state == NoteState::SUSTAINING),
			s_release = (double)(state.state == NoteState::RELEASING);

	double	attack_vel = attackInv,
			decay_vel = (1.0 - sustainLevel)*decayInv,
			release_vel = releaseInv;
	
		decayInv = (1.0 - sustainLevel)/decayTime;
		releaseInv = (sustainLevel > 0.0 ? sustainLevel : 1.0)/releaseTime;

		state.attackVel = attackInv*s_attack;
		state.decayVel = 

	//Step amplitude
	state.amplitude += info.getInvSampleRate()*( state.attackVel -
												 state.decayVel -
												 state.releaseVel );

	bool next_note_state = ( (s_attack && state.amplitude >= 1.0)			||
							 (s_decay && state.amplitude <= sustainLevel)	||
							 (s_release && state.amplitude <= 0.0) );

	state.amplitude = ( next_note_state ?
						  (1.0*(s_attack) + sustainLevel*(s_decay)) :
						  state.amplitude);

	state.amplitude *= (!(s_release && next_note_state));
	
	//Sample waveform with adjusted amplitude
	out_value = out_value*(double)add + state.getVelocity()*waveform.sample(state);

	//Step state
	info.stepState(state, next_note_state);
	
	return (state.state == NoteState::DONE);
	*/

	bool next_note_state = false;

	switch(state.state)
	{
	case NoteState::ATTACKING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? attackInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude >= 1.0);
		state.newState = next_note_state;
		state.amplitude = (next_note_state ? 1.0 : state.amplitude);
		break;

	case NoteState::DECAYING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? -(1.0 - sustainLevel)*decayInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude <= sustainLevel);
		state.newState = next_note_state;
		state.amplitude = (next_note_state ? sustainLevel : state.amplitude);
		break;

	case NoteState::SUSTAINING:
		//Just keep sustaining, do nothing here
		state.ampVel = 0.0;
		state.newState = true;
		break;

	case NoteState::RELEASING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? -state.amplitude*releaseInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude <= 0.0);
		state.newState = next_note_state;
		state.amplitude *= !next_note_state;
		break;

	default:
		//Note is done
		return true;
	}

	state.ampVel *= !next_note_state;

	AudioSample s_amp = (AudioSample)(round(state.getVelocity()*waveform.sample(state)));

	//static FAudioSample acc = 0.0f;
	//acc += vel;

	//Sample waveform with adjusted amplitude
	out_value = out_value*add + s_amp;

	state.lastSample = s_amp;

	//Step state
	info.stepState(state, next_note_state);

	return (state.state == NoteState::DONE);
}

bool WaveSynth::sampleVel(SampleState &state, const SampleInfo &info, AudioVelSample &out_value, bool add)
{
	/*
	double	s_attack = (double)(state.state == NoteState::ATTACKING),
			s_decay = (double)(state.state == NoteState::DECAYING),
			s_sustain = (double)(state.state == NoteState::SUSTAINING),
			s_release = (double)(state.state == NoteState::RELEASING);

	double	attack_vel = attackInv,
			decay_vel = (1.0 - sustainLevel)*decayInv,
			release_vel = releaseInv;
	
		decayInv = (1.0 - sustainLevel)/decayTime;
		releaseInv = (sustainLevel > 0.0 ? sustainLevel : 1.0)/releaseTime;

		state.attackVel = attackInv*s_attack;
		state.decayVel = 

	//Step amplitude
	state.amplitude += info.getInvSampleRate()*( state.attackVel -
												 state.decayVel -
												 state.releaseVel );

	bool next_note_state = ( (s_attack && state.amplitude >= 1.0)			||
							 (s_decay && state.amplitude <= sustainLevel)	||
							 (s_release && state.amplitude <= 0.0) );

	state.amplitude = ( next_note_state ?
						  (1.0*(s_attack) + sustainLevel*(s_decay)) :
						  state.amplitude);

	state.amplitude *= (!(s_release && next_note_state));
	
	//Sample waveform with adjusted amplitude
	out_value = out_value*(double)add + state.getVelocity()*waveform.sample(state);

	//Step state
	info.stepState(state, next_note_state);
	
	return (state.state == NoteState::DONE);
	*/

	bool next_note_state = false;

	switch(state.state)
	{
	case NoteState::ATTACKING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? attackInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude >= 1.0);
		state.newState = next_note_state;
		state.amplitude = (next_note_state ? 1.0 : state.amplitude);
		break;

	case NoteState::DECAYING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? -(1.0 - sustainLevel)*decayInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude <= sustainLevel);
		state.newState = next_note_state;
		state.amplitude = (next_note_state ? sustainLevel : state.amplitude);
		break;

	case NoteState::SUSTAINING:
		//Just keep sustaining, do nothing here
		state.ampVel = 0.0;
		state.newState = true;
		break;

	case NoteState::RELEASING:
		//Set velocity if havent yet
		state.ampVel = (state.newState ? -state.amplitude*releaseInv : state.ampVel);
		
		//Step amplitude
		state.amplitude += info.getInvSampleRate()*state.ampVel;

		next_note_state = (state.amplitude <= 0.0);
		state.newState = next_note_state;
		state.amplitude *= !next_note_state;
		break;

	default:
		//Note is done
		return true;
	}

	state.ampVel *= !next_note_state;

	AudioSample s_amp = (AudioSample)(round(state.getVelocity()*waveform.sample(state)));

	//static FAudioSample acc = 0.0f;
	//acc += vel;

	AudioVelSample vel = (AudioVelSample)(s_amp - state.lastSample);

	//Sample waveform with adjusted amplitude
	out_value = out_value*add + vel;

	state.lastSample = s_amp;

	//Step state
	info.stepState(state, next_note_state);

	return (state.state == NoteState::DONE);
}

bool WaveSynth::sampleSpring(SampleState &state, const SampleInfo &info, AudioVelSample &out_value, bool add)
{
	return true;
}

InstrumentType WaveSynth::getType() const
{
	return InstrumentType::WAVESYNTH;
}

void WaveSynth::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new WaveSynthDesc()));
	WaveSynthDesc *desc = dynamic_cast<WaveSynthDesc*>(objDesc);
	
	desc->type = getType();

	desc->interp = interp;
	desc->waveform = *(WaveformDesc*)waveform.getDesc();

	desc->attackTime = attackTime;
	desc->decayTime = decayTime;
	desc->sustainLevel = sustainLevel;
	desc->releaseTime = releaseTime;
}