#include "MIDI.h"

#include "AUtility.h"

#include <math.h>
#include <iostream>
#include <iomanip>

#include <cstdint>


MidiIndex			A4_INDEX = 69;
int					OCTAVE_SIZE = 12;

double FREQUENCIES[NUM_MIDI_NOTES];



#define TWELFTH_ROOT_OF_2 1.05946309436

void setA4(double a4_freq)
{
	double twelfth_root_of_2 = TWELFTH_ROOT_OF_2;//1.05946309436;//pow(2.0, 1.0/12.0);
	//std::cout << std::fixed << std::setprecision(18) << twelfth_root_of_2 << "\n\n";

	for(MidiIndex i = 0; i < NUM_MIDI_NOTES; i++)
		FREQUENCIES[i] = a4_freq * pow(twelfth_root_of_2, i - A4_INDEX);
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
		//if(e.velocity > 0)
		//{
			keyStates[e.midiIndex].noteOn = true;
			keyStates[e.midiIndex].event_t = e.time;
			keyStates[e.midiIndex].velocity = e.velocity;

			return true;
		//}
		//else it's a NOTE_OFF

	case MidiEventType::NOTE_OFF:
		keyStates[e.midiIndex].noteOn = false;
		keyStates[e.midiIndex].event_t = e.time;
		keyStates[e.midiIndex].velocity = e.velocity;
		keyStates[e.midiIndex].sustaining = sustainOn;

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



/////SAMPLE STOPPER/////

/*
SampleStopper::SampleStopper()
{ }
SampleStopper::SampleStopper(SampleState state_, Time note_offset)
	: state(state_), offset(note_offset)
{ }

bool SampleStopper::isValid()
{
	return true;//info.isValid();
}
*/


/////MIDI NOTE/////

MidiNote::MidiNote()
{ }

//Pass end_time if both start and end times are known at the time of initialization (finished note)
MidiNote::MidiNote(MidiIndex midi_index, int note_velocity, Time start_time, Time end_time)
	: index(midi_index), velocity(note_velocity), range(start_time, end_time)//, stoppers(1)
{
	finished = (range.end > 0.0);

	range.end =	(finished ? range.end : range.start);	//Set end to start if note isnt finished

	//setBaseStopper(SampleStopper(SampleState(getFrequency(midi_index), 0.0, 0.0, (double)velocity*(AUDIO_MAX_AMPLITUDE/127.0), range, NoteState::NONE), 0.0));
}

/*
void MidiNote::setBaseStopper(const SampleStopper &ss)
{
	stoppers[0] = ss;
	stoppers[0].offset = 0;	//Make sure offset is 0 (at very beginning of note)
}

void MidiNote::addStopper(const SampleStopper &ss)
{
	//Check range
	if(ss.offset >= range.length() || ss.offset == 0)
		return;

	for(unsigned int i = 1; i < stoppers.size() - 1; i++)
	{
		if(stoppers[i].offset == ss.offset)
		{
			stoppers[i] = ss;
			return;
		}
		else if(stoppers[i + 1].offset > ss.offset)
		{
			stoppers.insert(stoppers.begin() + i + 1, ss);
			return;
		}
	}

	//Add ss to end of list if none were bigger than it
	stoppers.push_back(ss);
}

SampleStopper MidiNote::getClosestStopper(Time t) const
{
	Time offset = t - range.start;

	for(unsigned int i = 0; i < stoppers.size(); i++)
	{
		if(i == stoppers.size() - 1 || stoppers[i + 1].offset > offset)
		{
			SampleStopper ss = stoppers[i];
			return ss;
		}
	}
}

void MidiNote::deleteStopperRange(TimeRange delete_range)
{
	Time	start_offset = delete_range.start - range.start,
			end_offset = delete_range.end - range.end;

	//Stopper index at which to start deleting
	int start_index = -1;

	for(unsigned int i = 0; i < stoppers.size(); i++)
	{
		if(i != 0 && start_index < 0 && stoppers[i].offset >= start_offset)
			start_index = (int)i;
		else if(start_index >= 0 && stoppers[i].offset >= end_offset)
		{
			//Erase range of stoppers
			stoppers.erase(stoppers.begin() + start_index, stoppers.begin() + i - 1);
			break;
		}
	}
}
*/
/*
void MidiNote::moveStart(Time offset)
{
	range.start += offset;

	//Move end too if note isnt finished yet (start == end)
	end += (Time)(!finished)*offset;
}

void MidiNote::moveEnd(Time offset)
{
	//Only move if note is finished
	end += (Time)(finished)*offset;
}

void MidiNote::moveNote(Time offset)
{
	moveStart(offset);
	moveEnd(offset);
}
*/

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




/////MIDI DATA/////

MidiData::MidiData()
{ }
/*
MidiData::MidiData(unsigned int size)
	: data(size)
{ }
*/
MidiData::MidiData(const std::vector<MidiNote> &initial_data)
{
	midiLock.lockWait();
	addNotes(initial_data);
	midiLock.unlock();
}

MidiData::MidiData(const std::vector<MidiNote*> &initial_data)
{
	midiLock.lockWait();
	addNotes(initial_data);
	midiLock.unlock();
}

MidiData::MidiData(const MidiData &other)
{
	midiLock.lockWait();

	for(auto mi : other.data)
	{
		data.emplace(mi.first, std::vector<MidiNote*>());
		std::vector<MidiNote*> &vec = data[mi.first];
		vec.reserve(mi.second.size());

		for(auto n : mi.second)
			vec.push_back(new MidiNote(*n));
	}

	nNotes = other.nNotes;
	updateSpan();

	midiLock.unlock();
}

MidiData::~MidiData()
{
	midiLock.lockWait();
	clearNotes();
	midiLock.unlock();
}

MidiData& MidiData::operator=(const MidiData &other)
{
	midiLock.lockWait();
	clearNotes();

	for(auto mi : other.data)
	{
		data.emplace(mi.first, std::vector<MidiNote*>());
		std::vector<MidiNote*> &vec = data[mi.first];
		vec.reserve(mi.second.size());

		for(auto n : mi.second)
			vec.push_back(new MidiNote(*n));
	}

	nNotes = other.nNotes;
	updateSpan();
	
	midiLock.unlock();

	return *this;
}

void MidiData::updateSpan()
{
	if(data.size() > 0)
	{
		span = TimeRange(-1.0, -1.0);

		for(auto mi : data)
		{
			for(auto n : mi.second)
			{
				span.start = ((span.start < n->range.start && span.start >= 0) ? span.start : n->range.start);
				span.end = ((span.end > n->range.end && span.start >= 0) ? span.end : n->range.end);
			}
		}
	}
	else
		span = TimeRange(0.0, 0.0);
}

int MidiData::numNotes() const
{
	return nNotes;
}

TimeRange MidiData::getSpan() const
{
	return span;
}

Time MidiData::getLength() const
{
	return span.length();
}

MidiNote* MidiData::addNote(const MidiNote &note)
{
	MidiNote *p_mn = new MidiNote(note);
	
	midiLock.lockWait();

	auto n_iter = data.find(p_mn->index);

	if(n_iter == data.end())
		data.emplace(p_mn->index, std::vector<MidiNote*>(1, p_mn));
	else
	{
		//Look for proper placement -- Go backwards, since it's more likely to be at the end
		for(int i = n_iter->second.size() - 1; i >= 0; i--)
		{
			if((p_mn->range.start < n_iter->second[i]->range.start) || (i == 0))
			{
				n_iter->second.insert(n_iter->second.begin() + i, p_mn);
				break;
			}
		}
	}

	//Update span
	span.start = (span.start < p_mn->range.start ? span.start : p_mn->range.start);
	span.end = (span.end > p_mn->range.end ? span.end : p_mn->range.end);

	nNotes++;
	
	midiLock.unlock();

	return p_mn;
}

std::vector<MidiNote*> MidiData::addNotes(const std::vector<MidiNote> &new_notes)
{
	std::vector<MidiNote*> p_new_notes;
	p_new_notes.reserve(new_notes.size());
	
	if(nNotes == 0 && new_notes.size() > 0)
	{
		span.start = new_notes[0].range.start;
		span.end = new_notes[0].range.end;
	}

	for(auto n : new_notes)
		p_new_notes.push_back(addNote(n));

	return p_new_notes;
}

std::vector<MidiNote*> MidiData::addNotes(const std::vector<MidiNote*> &new_notes)
{
	std::vector<MidiNote*> p_new_notes;
	p_new_notes.reserve(new_notes.size());

	if(nNotes == 0 && new_notes.size() > 0)
	{
		span.start = new_notes[0]->range.start;
		span.end = new_notes[0]->range.end;
	}

	for(auto n : new_notes)
		p_new_notes.push_back(addNote(*n));

	return p_new_notes;
}

void MidiData::clearNotes()
{
	midiLock.lockWait();

	for(auto &mi : data)
	{
		for(auto n : mi.second)
			AU::safeDelete(n);
		mi.second.clear();
	}
	data.clear();

	span = TimeRange(0.0, 0.0);
	nNotes = 0;
	
	midiLock.unlock();
}

AStatus MidiData::removeNote(MidiNote *note)
{
	AStatus status;
	status.setError(AS::EType::GENERAL, "The requested not is not contained in data, could not remove.");
	
	midiLock.lockWait();

	std::vector<MidiNote*> &n = data[note->index];

	//Search backward, since the note is most likely near the end
	for(int i = (int)n.size() - 1; i >= 0; i--)
	{
		if(n[i] == note)
		{
			n.erase(n.begin() + i);
			status.setSuccess();
			break;
		}
	}
	
	midiLock.unlock();

	return status;
}

std::vector<MidiNote*> MidiData::getNotes()
{
	midiLock.lockWait();

	std::vector<MidiNote*> notes;
	notes.reserve(nNotes);
	for(auto mi : data)
	{
		for(auto n : mi.second)
		{
			if(n->index >= 0)
				notes.push_back(n);
		}
	}
	
	midiLock.unlock();

	return notes;
}

ConstMidiData MidiData::getConstNotes() const
{
	//midiLock.lockWait();

	ConstMidiData notes;
	notes.reserve(nNotes);

	//Move all intersecting notes into packet data
	for(auto mi : data)
	{
		for(auto n : mi.second)
		{
			if(n->index >= 0)
				notes.push_back(n);
		}
	}
	
	//midiLock.unlock();

	return notes;
}

void MidiData::getNotes(TimeRange range, MidiData &notes)
{
	notes.clearNotes();
	
	midiLock.lockWait();

	//Move all intersecting notes into given MidiData (ignore index = -1 --> rest)
	for(auto mi : data)
	{
		for(auto n : mi.second)
		{
			if(n->index >= 0 && (n->range.start < range.end) && (n->range.end >= range.start))
				notes.addNote(*n);
		}
	}
	
	midiLock.unlock();
}

ConstMidiData MidiData::getConstNotes(TimeRange range) const
{
	ConstMidiData notes;
	
	//midiLock.lockWait();

	//Move all intersecting notes into packet data
	for(auto mi : data)
	{
		for(auto n : mi.second)
		{
			if(n->index >= 0 && (n->range.start < range.end) &&(n->range.end >= range.start))
				notes.push_back(n);
		}
	}
	
	//midiLock.unlock();

	return notes;
}



/////MIDI EVENT/////

MidiEvent::MidiEvent(double time_stamp, const std::vector<unsigned char> &message)
	: time(time_stamp), fTime(time_stamp), numDataBytes(message.size())
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


MidiEvent::MidiEvent(MidiIndex index, MidiEventType type_, unsigned int vel, double g_time)
	: midiIndex(index), type(type_), velocity(vel), time(g_time), fTime(g_time)
{
	
}
