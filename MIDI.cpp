#include "MIDI.h"

#include "AUtility.h"

#include <math.h>
#include <iostream>
#include <iomanip>

#include <cstdint>


MidiIndex			A4_INDEX = 69;
int					OCTAVE_SIZE = 12;

double FREQUENCIES[NUM_MIDI_NOTES];



static const double TWELFTH_ROOT_OF_2 = 1.05946309436;

void setA4(double a4_freq)
{
	//TWELFTH_ROOT_OF_2;//1.05946309436;//pow(2.0, 1.0/12.0);
	//std::cout << std::fixed << std::setprecision(18) << twelfth_root_of_2 << "\n\n";

	for(MidiIndex i = 0; i < NUM_MIDI_NOTES; i++)
		FREQUENCIES[i] = a4_freq * pow(TWELFTH_ROOT_OF_2, i - A4_INDEX);
}

double getFrequency(MidiIndex midi_index)
{
	if(midi_index < NUM_MIDI_NOTES)
		return FREQUENCIES[midi_index];
	else
		return FREQUENCIES[A4_INDEX] * pow(TWELFTH_ROOT_OF_2, midi_index - A4_INDEX);
}



/////MIDI NOTE STATE/////

MidiNoteState::MidiNoteState()
{ }



/////MIDI NOTE STATES/////

MidiDeviceState::MidiDeviceState()
{ }

bool MidiDeviceState::applyEvent(const MidiEvent &e)
{
	//Apply event to state
	switch(e.type)
	{
	case MidiEventType::NOTE_ON:
		if(e.midiIndex >= 0)
		{
		//if(e.velocity > 0)
		//{
			keyStates[e.midiIndex].noteOn = true;
			keyStates[e.midiIndex].event_t = e.time;
			keyStates[e.midiIndex].velocity = e.velocity;

			return true;
		//}
		//else it's a NOTE_OFF
		}

	case MidiEventType::NOTE_OFF:
		if(e.midiIndex >= 0)
		{
			keyStates[e.midiIndex].noteOn = false;
			keyStates[e.midiIndex].event_t = e.time;
			keyStates[e.midiIndex].velocity = e.velocity;
			keyStates[e.midiIndex].sustaining = sustainOn;
		}

		return true;

	case MidiEventType::CONTROL_CHANGE:
		switch(e.control)
			{
			case MidiControlType::SUSTAIN:
				if(!(sustainOn = (e.velocity >= 64)))
				{
					//Turn off sustaining notes
					for(auto &s : keyStates)
						s.sustaining = false;
				}
				return true;

			default:
				return false;
		}

	default:
		return false;
	}
}

bool MidiDeviceState::anyNotesOn() const
{
	for(auto n : keyStates)
	{
		if(n.noteOn)
			return true;
	}

	return false;
}


/////PEDAL EVENT/////
/*
PedalEvent::PedalEvent()
{ }
PedalEvent::PedalEvent(MidiPedalType type_, bool pedal_on, Time g_time)
	: type(type_), on(pedal_on), time(g_time)
{ }
*/

/////MIDI NOTE EVENTS/////



/////MIDI NOTE/////

MidiNote::MidiNote()
{ }

//Pass end_time if both start and end times are known at the time of initialization (finished note)
MidiNote::MidiNote(MidiIndex midi_index, int note_velocity, Time start_time, Time end_time)
	: index(midi_index), velocity(note_velocity), range(start_time, end_time)
{
	finished = (range.end > 0.0);

	range.end =	(finished ? range.end : range.start);	//Set end to start if note isnt finished
}


void MidiNote::moveNote(TimeRange d_r)
{
	range.start += d_r.start;
	range.end += d_r.end;
}

void MidiNote::setFinished(Time end_time)
{
	//Only change end time if note isnt already finished
	range.end = (finished ? range.end : end_time);
	finished = true;
}

bool MidiNote::isFinished() const
{
	return finished;
}



/////MIDI EVENT/////

MidiEvent::MidiEvent(Time time_stamp, const std::vector<unsigned char> &message)
	: time(time_stamp), numDataBytes(message.size())
{
	if(numDataBytes > 0)
	{
		unsigned char status = message[0];

		type = getMidiEventType(status);
		data1 = (numDataBytes > 1) ? message[1] : 0x00;
		data2 = (numDataBytes > 2) ? message[2] : 0x00;

		channel = (type > MidiEventType::INVALID && type < MidiEventType::CUSTOM_MESSAGE)
					? getMidiChannel(status)
					: MidiChannel::INVALID;
		
		//If velocity == 0, that should be a NOTE_OFF
		type = (type == MidiEventType::NOTE_ON && velocity == 0)
					? MidiEventType::NOTE_OFF
					: type;
	}
}


MidiEvent::MidiEvent(MidiIndex index, MidiEventType type_, unsigned int vel, Time g_time)
	: midiIndex(index), type(type_), velocity(vel), time(g_time)
{
	
}
