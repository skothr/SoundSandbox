#ifndef MIDI_H
#define MIDI_H

#include "Timing.h"
#include "Filter.h"
#include "Sampling.h"
#include "OwnedMutex.h"
#include "AStatus.h"

#include <array>
#include <deque>
#include <unordered_map>
#include <unordered_set>


enum class MidiEventType : unsigned char
{
	INVALID					= 0x00,

	//Channel Voice Messages
	NOTE_OFF				= 0x80,
	NOTE_ON					= 0x90,
	POLYPHONIC_PRESSURE		= 0xA0,
	CONTROL_CHANGE			= 0xB0,
	PROGRAM_CHANGE			= 0xC0,
	CHANNEL_PRESSURE		= 0xD0,
	PITCH_BEND				= 0xE0,

	//System Common Messages
	CUSTOM_MESSAGE			= 0xF0,
	QUARTER_FRAME			= 0xF1,
	SONG_POSITION_POINTER	= 0xF2,
	SONG_SELECT				= 0xF3,
	TUNE_REQUEST			= 0xF6,
	END_CUSTOM_MESSAGE		= 0xF7,

	//System Real-time Messages
	TIMING_CLOCK			= 0xF8,
	TIMING_START			= 0xFA,
	TIMING_CONTINUE			= 0xFB,
	TIMING_STOP				= 0xFC,
	ACTIVE_SENSING			= 0xFE,
	RESET					= 0xFF
};

inline unsigned char getByte(MidiEventType t)
{
	return static_cast<unsigned char>(t);
}
inline MidiEventType getMidiEventType(unsigned char t)
{
	//Omit channel part (if applicable)
	return static_cast<MidiEventType>((t > getByte(MidiEventType::INVALID) && t < getByte(MidiEventType::CUSTOM_MESSAGE))
									  ? t & 0xF0
									  : t );
}

inline std::ostream& operator<<(std::ostream &os, MidiEventType t)
{
	os << "(" << getByte(t) << " --> ";

	switch(t)
	{
	case MidiEventType::INVALID:
		os << "INVALID";
		break;
	case MidiEventType::NOTE_OFF:
		os << "NOTE_OFF";
		break;
	case MidiEventType::NOTE_ON:
		os << "NOTE_ON";
		break;
	case MidiEventType::POLYPHONIC_PRESSURE:
		os << "POLYPHONIC_PRESSURE";
		break;
	case MidiEventType::CONTROL_CHANGE:
		os << "CONTROL_CHANGE";
		break;
	case MidiEventType::PROGRAM_CHANGE:
		os << "PROGRAM_CHANGE";
		break;
	case MidiEventType::CHANNEL_PRESSURE:
		os << "CHANNEL_PRESSURE";
		break;
	case MidiEventType::PITCH_BEND:
		os << "PITCH_BEND";
		break;
	case MidiEventType::CUSTOM_MESSAGE:
		os << "CUSTOM_MESSAGE";
		break;
	case MidiEventType::QUARTER_FRAME:
		os << "QUARTER_FRAME";
		break;
	case MidiEventType::SONG_POSITION_POINTER:
		os << "SONG_POSITION_POINTER";
		break;
	case MidiEventType::SONG_SELECT:
		os << "SONG_SELECT";
		break;
	case MidiEventType::TUNE_REQUEST:
		os << "TUNE_REQUEST";
		break;
	case MidiEventType::END_CUSTOM_MESSAGE:
		os << "END_CUSTOM_MESSAGE";
		break;
	case MidiEventType::TIMING_CLOCK:
		os << "TIMING_CLOCK";
		break;
	case MidiEventType::TIMING_START:
		os << "TIMING_START";
		break;
	case MidiEventType::TIMING_CONTINUE:
		os << "TIMING_CONTINUE";
		break;
	case MidiEventType::TIMING_STOP:
		os << "TIMING_STOP";
		break;
	case MidiEventType::ACTIVE_SENSING:
		os << "ACTIVE_SENSING";
		break;
	case MidiEventType::RESET:
		os << "RESET";
		break;
	default:
		os << "UNKNOWN";
	}

	os << ")";
	return os;
}


enum class MidiChannel : unsigned char
{
	CHANNEL1	= 0x00,
	CHANNEL2	= 0x01,
	CHANNEL3	= 0x02,
	CHANNEL4	= 0x03,
	CHANNEL5	= 0x04,
	CHANNEL6	= 0x05,
	CHANNEL7	= 0x06,
	CHANNEL8	= 0x07,
	CHANNEL9	= 0x08,
	CHANNEL10	= 0x09,
	CHANNEL11	= 0x0A,
	CHANNEL12	= 0x0B,
	CHANNEL13	= 0x0C,
	CHANNEL14	= 0x0D,
	CHANNEL15	= 0x0E,
	CHANNEL16	= 0x0F,

	INVALID		= 0xFF
};

inline unsigned char getByte(MidiChannel c)
{
	return static_cast<unsigned char>(c);
}
inline MidiChannel getMidiChannel(unsigned char c)
{
	//Omit first half
	return static_cast<MidiChannel>(c == 0xFF ? 0xFF : (c & 0x0F));
}

//Values for data byte 2 
enum class MidiControlType : unsigned char
{
	BANK_SELECT			= 0x00,
	MODULATION_WHEEL	= 0x01,
	BREATH				= 0x02,
	FOOT				= 0x04,
	DATA_ENTRY_MSB		= 0x06,
	CHANNEL_VOLUME		= 0x07,
	BALANCE				= 0x08,
	PAN					= 0x0A,
	EXPRESSION			= 0x0B,
	EFFECT_1			= 0x0C,
	EFFECT_2			= 0x0D,
	
	PORTAMENTO_TIME		= 0x05,
	PORTAMENTO_TOGGLE	= 0x41,
	PORTAMENTO_CONTROL	= 0x54,

	SUSTAIN				= 0x40,
	SOSTENUTO			= 0x42,
	SOFT_PEDAL			= 0x43,
	LEGATO				= 0x44,
	HOLD_2				= 0x45,

	GENERAL_PURPOSE_1	= 0x10,
	GENERAL_PURPOSE_2	= 0x11,
	GENERAL_PURPOSE_3	= 0x12,
	GENERAL_PURPOSE_4	= 0x13,
	GENERAL_PURPOSE_5	= 0x50,
	GENERAL_PURPOSE_6	= 0x51,
	GENERAL_PURPOSE_7	= 0x52,
	GENERAL_PURPOSE_8	= 0x53,

	//TODO: LSB values?

	SOUND_1				= 0x46,
	SOUND_2				= 0x47,
	SOUND_3				= 0x48,
	SOUND_4				= 0x49,
	SOUND_5				= 0x4A,
	SOUND_6				= 0x4B,
	SOUND_7				= 0x4C,
	SOUND_8				= 0x4D,
	SOUND_9				= 0x4E,
	SOUND_10			= 0x4F,

	HR_VEL_PREFIX		= 0x58,

	EFFECT_1_DEPTH		= 0x5B,
	EFFECT_2_DEPTH		= 0x5C,
	EFFECT_3_DEPTH		= 0x5D,
	EFFECT_4_DEPTH		= 0x5E,
	EFFECT_5_DEPTH		= 0x5F,

	DATA_INCREMENT		= 0x60,
	DATA_DECREMENT		= 0x61,

	//Mode messages
	ALL_SOUND_OFF		= 0x78,
	RESET_CONTROLLERS	= 0x79,
	LOCAL_CONTROL_TOGGLE= 0x7A,
	ALL_NOTES_OFF		= 0x7B,
	OMNI_MODE_OFF		= 0x7C,
	OMNI_MODE_ON		= 0x7D,
	MONO_MODE_ON		= 0x7E,
	POLY_MODE_ON		= 0x7F,


	INVALID				= 0xFF
};

inline unsigned char getByte(MidiControlType ct)
{
	return static_cast<unsigned char>(ct);
}
inline MidiControlType getMidiControlType(unsigned char ct)
{
	return static_cast<MidiControlType>(ct);
}

enum class MidiPedalType
{
	INVALID = -1,

	SUSTAIN = 0,
	SOSTENUTO,

	SOFT_PEDAL,
	LEGATO
};


typedef int MidiIndex;

static const MidiIndex	NUM_MIDI_NOTES = 128;

extern MidiIndex		A4_INDEX;
extern int				OCTAVE_SIZE;

//Array to hold frequency for each midi note
extern double FREQUENCIES[NUM_MIDI_NOTES];

//Sets the frequency of A4
extern void setA4(double a4_freq);

//Gets the frequency for a specified midi index
extern double getFrequency(MidiIndex midi_index);

struct MidiEvent;

//Holds the state of a MIDI note
struct MidiNoteState
{
	bool	noteOn = false,
			sustaining = false;

	int		velocity = 0;
	Time	event_t = 0.0;

	MidiNoteState();

};

//Holds the states of a MIDI device
struct MidiDeviceState
{
	//Key (note) states
	std::array<MidiNoteState, NUM_MIDI_NOTES>	keyStates;

	//Other states
	bool										sustainOn = false;

	MidiDeviceState();

	//Returns whether the event had any effect on the current state
	bool applyEvent(const MidiEvent &e);
	//Returns whether any of the notes are on (excluding sustaining)
	bool anyNotesOn() const;
};

/*
struct PedalEvent
{
	MidiPedalType type;
	bool on;
	Time time;

	PedalEvent();
	PedalEvent(MidiPedalType type_, bool pedal_on, Time g_time);
};

//Holds event data for each note.
struct MidiNoteEvents
{
	//Note events
	std::array<std::vector<MidiEvent>, NUM_MIDI_NOTES> events;
	//Sustain pedal events
	std::vector<PedalEvent> sus_events;

};
*/

/*
//SampleStopper -- holds information about where the sample info left off from
//	the last updated chunk in a MidiNote
struct SampleStopper
{
	SampleState state;
	Time		offset = 0.0;

	SampleStopper();
	SampleStopper(SampleState state_, Time note_offset);

	bool isValid();
};
*/

//MidiNote -- Holds info for a note.
struct MidiNote
{
	MidiIndex	index = -1;
	int			velocity = -1;

	TimeRange	range = TimeRange(0.0, 0.0);

	bool		finished = false;

	//std::vector<SampleStopper> stoppers;

	MidiNote();
	//Pass end_time if both start and end times are known at the time of initialization (finished note)
	MidiNote(MidiIndex midi_index, int note_velocity, Time start_time, Time end_time = -1.0);

	//void setBaseStopper(const SampleStopper &ss);
	//void addStopper(const SampleStopper &ss);
	//Deletes stoppers within the given range (track time, not offset from start of note)
	//void deleteStopperRange(TimeRange delete_range);
	//Gets the closest stopper behind the given time (track time, not offset from start of note)
	//SampleStopper getClosestStopper(Time time) const;

	/*
	void moveStart(Time offset);
	void moveEnd(Time offset);
	void moveNote(Time offset);
	*/

	void moveNote(TimeRange d_r);

	void setFinished(Time end_time);
	bool isFinished() const;
};

//Vector of MidiNote pointers (used for MIDI communication)
//typedef std::vector<MidiNote*> PartialMidiData;
typedef std::vector<const MidiNote*> ConstMidiData;

//Used for midi storage
class MidiData
{
private:
	//Each vector is in order of start time.
	std::unordered_map<MidiIndex, std::vector<MidiNote*>> data;
	TimeRange span = TimeRange(0.0, 0.0);
	int nNotes = 0;
	
	OwnedMutex		midiLock;

public:
	MidiData();
	//MidiData(unsigned int size);
	MidiData(const std::vector<MidiNote> &initial_data);
	MidiData(const std::vector<MidiNote*> &initial_data);
	MidiData(const MidiData &other);
	~MidiData();

	MidiData& operator=(const MidiData &other);
	
	void updateSpan();

	int numNotes() const;
	TimeRange getSpan() const;
	Time getLength() const;

	//Returns pointer to added note
	MidiNote* addNote(const MidiNote &note);
	//Returns vector of pointers to added notes
	std::vector<MidiNote*> addNotes(const std::vector<MidiNote> &new_notes);
	std::vector<MidiNote*> addNotes(const std::vector<MidiNote*> &new_notes);

	void clearNotes();
	AStatus removeNote(MidiNote *note);

	std::vector<MidiNote*> getNotes();
	ConstMidiData getConstNotes() const;
	void getNotes(TimeRange range, MidiData &notes);
	ConstMidiData getConstNotes(TimeRange range) const;
};


//TODO: Speed up finding midi notes within a chunk. (if possible)

/*
//Vector of MidiNotes (used for MIDI storage)
typedef std::vector<MidiNote*> MidiChunk;

//Combination of MidiChunk and a vector of indices (used to tell if there are notes present in each chunk,
//	and what the index of the first note that intersects is)
struct MidiData
{
	MidiChunk notes;
	//Each element represents whether a chunk contains any MidiNotes (non-zero)
	//	and the index of the first note that intersects (if any)
	std::vector<unsigned int> filled_regions;

	MidiData(MidiChunk &chunk, 
};
*/


struct MidiEvent
{
	MidiEventType	type = MidiEventType::INVALID;
	MidiChannel		channel = MidiChannel::INVALID;
	unsigned int	numDataBytes = 0;

	Time			time = 0.0,		//Time the event occurred
					fTime = 0.0;	//Time the event is put into effect (functional time)
									//	(e.g. playing it --> could be offset for live input)

	union
	{
		unsigned char	data1;
		MidiIndex		midiIndex = 0;
	};

	union
	{
		unsigned char	data2;
		MidiControlType control;
		unsigned int	velocity = 0;
	};

	//Use this if event is coming directly from device
	MidiEvent(Time time_stamp, const std::vector<unsigned char> &message);
	//Use this if creating event manually
	MidiEvent(MidiIndex index, MidiEventType type_, unsigned int vel, Time g_time);

};

typedef std::deque<MidiEvent> MidiEventQueue;
typedef std::unordered_set<MidiIndex> MidiNoteSet;

#endif	//MIDI_H