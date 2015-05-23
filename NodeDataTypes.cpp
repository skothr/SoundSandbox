#include "NodeDataTypes.h"

#include "AudioChunk.h"

/////NODE PACKET/////

NodePacket::NodePacket()
{ }



/////PULL PACKET/////

PullPacket::PullPacket()
{ }


/////PUSH PACKET/////

PushPacket::PushPacket()
{ }


/////FLUSH PACKET/////

FlushPacket::FlushPacket(const TimeRange &flush_time_range, const TimeRange &target_time_range,
						 const ChunkRange &flush_chunk_range, const ChunkRange &target_chunk_range,
						const TimeRange &global_time_range)
	: flushTimeRange(flush_time_range), targetTimeRange(target_time_range),
		flushChunkRange(flush_chunk_range), targetChunkRange(target_chunk_range),
		globalTimeRange(global_time_range)
{ }



/////AUDIO PULL PACKET/////

AudioPullPacket::AudioPullPacket()
{ }
AudioPullPacket::AudioPullPacket(AudioVelChunk** data_, ChunkRange chunk_range, TransferMethod method_, bool update_data)
	: TimePacket<c_time>(chunk_range),
		data(data_), method(method_), update(update_data)
{ }


/////AUDIO PUSH PACKET/////

AudioPushPacket::AudioPushPacket()
{ }
AudioPushPacket::AudioPushPacket(AudioVelChunk** data_, ChunkRange data_range, TransferMethod method_)
	: LiveTimePacket<c_time>(data_range),
		data(data_), method(method_)
{ }



/////MIDI PULL PACKET/////

MidiPullPacket::MidiPullPacket()
{ }
MidiPullPacket::MidiPullPacket(TimeRange time_range)
	: TimePacket<double>(time_range)
{ }


/////MIDI PUSH PACKET/////

MidiPushPacket::MidiPushPacket()
{ }
MidiPushPacket::MidiPushPacket(TimeRange data_range, const MidiEventQueue &events_)
	: LiveTimePacket<double>(data_range),
		events(events_)
{ }



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