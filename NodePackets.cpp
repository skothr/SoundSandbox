#include "NodePackets.h"

#include "AudioChunk.h"
#include "MIDI.h"
#include "Samplers.h"

#include "Cursor.h"

/////NODE PACKET/////

NodePacket::NodePacket()
{ }



/////PULL PACKET/////

//PullPacket::PullPacket()
//{ }

PullPacket::PullPacket()
{ }


/////PUSH PACKET/////

PushPacket::PushPacket()
{ }


/////TIME PULL PACKET/////

TimePullPacket::TimePullPacket(Cursor *p_cursor)//, bool update_cursor)
	: pCursor(p_cursor)//, updateCursor(update_cursor)
{ }
/*
TimePullPacket::TimePullPacket(TimeRange global_range, Cursor *active_cursor)
	:globalRange(global_range), activeCursor(active_cursor)
{ }
*/

/////TIME PUSH PACKET/////

TimePushPacket::TimePushPacket()
{ }
/*
TimePushPacket::TimePushPacket(Cursor *active_cursor)
	: activeCursor(active_cursor)
{ }
*/

/////AUDIO PULL PACKET/////

AudioPullPacket::AudioPullPacket(Cursor *p_cursor, AudioVelChunk *data_)
	: TimePullPacket(p_cursor),//, update_cursor),
		data(data_)
{ }
AudioPullPacket::AudioPullPacket(Cursor *p_cursor, AudioVelChunk* data_, TransferMethod t_method)
	: TimePullPacket(p_cursor),//, update_cursor),
		data(data_), method(t_method)
{ }


/////AUDIO PUSH PACKET/////

AudioPushPacket::AudioPushPacket(AudioVelChunk *data_)//, AudioVelChunk *last_data, Time data_start)
	: TimePushPacket(),
		data(data_)//, lastData(last_data), dataStart(data_start)
{ }
AudioPushPacket::AudioPushPacket(AudioVelChunk* data_, /*AudioVelChunk *last_data, Time data_start,*/ TransferMethod method_)
	: TimePushPacket(),
		data(data_), /*lastData(last_data), dataStart(data_start),*/ method(method_)
{ }



/////MIDI PULL PACKET/////

MidiPullPacket::MidiPullPacket(Cursor *p_cursor)//, bool update_cursor)
	: TimePullPacket(p_cursor)//, update_cursor)
{ }
//
//MidiPullPacket::MidiPullPacket(Time global_time, const MidiSet &midi_notes)
//	: TimePullPacket(global_time),
//		notes(midi_notes)
//{ }


/////MIDI PUSH PACKET/////

MidiPushPacket::MidiPushPacket(MidiEvent e)
	: event(e)
{ }


/////MIDI EVENT PUSH PACKET/////
/*
MidiEventPushPacket::MidiEventPushPacket()
	: e(-1, MidiEventType::INVALID, 0, 0.0)
{ }
MidiEventPushPacket::MidiEventPushPacket(const MidiEvent &e_)
	: e(e_)
{ }
*/


/////INFO PACKET/////

InfoPacket::InfoPacket()
{ }




/////AUDIO INFO PACKET/////

AudioInfoPacket::AudioInfoPacket()
{ }
AudioInfoPacket::AudioInfoPacket(int bpm_, int sample_rate)
	: bpm(bpm_), sampleRate(sample_rate)
{ }



/////INSTRUMENT PACKET/////

InstrumentPacket::InstrumentPacket()
{ }
InstrumentPacket::InstrumentPacket(NoteSampleFunction sample_function, NoteSampleFunction sample_vel_function)
	: sample(sample_function), sampleVel(sample_vel_function)
{ }