#ifndef APOLLO_NODE_DATA_TYPES_H
#define APOLLO_NODE_DATA_TYPES_H

#include "MIDI.h"
#include "Instrument.h"
#include "ExpandEnum.h"
#include "Sampling.h"
#include "Range.h"

#include "Audio.h"

class AudioVelChunk;

enum class NodeDataType
{
	INVALID			= -1,
	NONE			= 0x00,

	PUSH			= 0x01,
	PULL			= 0x02,

	TIME			= 0x04,

	AUDIO			= 0x08,
	MIDI			= 0x10,

	INFO			= 0x20,

	FLUSH			= 0x40,

	INSTRUMENT		= 0x80,
	AUDIO_INFO		= 0x100
};

typedef NodeDataType NodeData;

EXPAND_ENUM_CLASS_OPERATORS(NodeDataType)
EXPAND_ENUM_CLASS_VALID(NodeDataType, NodeData::NONE)
EXPAND_ENUM_CLASS_PRINT(NodeDataType,
						({ NodeData::INVALID, NodeData::NONE, NodeData::PULL, NodeData::PUSH, NodeData::TIME, NodeData::AUDIO, NodeData::MIDI, NodeData::INFO, NodeData::INSTRUMENT, NodeData::AUDIO_INFO }),
						({ "INVALID", "NONE", "PULL", "PUSH", "TIME", "AUDIO", "MIDI", "INFO", "INSTRUMENT", "AUDIO_INFO" }) )

enum class TransferMethod
{
	INVALID = -1,
	COPY = 0,
	DIRECT,
	ADD
};

// Base Packet class, a pointer to which is used to transfer data between nodes.
struct NodePacket
{
	NodePacket();
	virtual ~NodePacket() = default;

	virtual NodeDataType getType() = 0;
};

struct PullPacket : public virtual NodePacket
{
	PullPacket();
	virtual ~PullPacket() = default;

	virtual NodeDataType getType() override
	{ return NodeData::PULL; }
};

struct PushPacket : public NodePacket
{
	PushPacket();
	virtual ~PushPacket() = default;

	virtual NodeDataType getType() override
	{ return NodeData::PUSH; }
};

//Packet requesting a flush
struct FlushPacket : public NodePacket
{
	TimeRange			flushTimeRange,			//relative time range to flush data from
						targetTimeRange;		//relative time range to flush data to
	ChunkRange			flushChunkRange,		//relative range of chunks to flush from
						targetChunkRange;		//relative range of chunks to flush to

	const TimeRange		globalTimeRange;		//Range in global time --> ONLY USE FOR EVENT COMPARISON

	//const Time			globalOffset;			//Offset from global time to speaker time


	//FlushPacket(TimeRange time_range, ChunkRange chunk_range);
	FlushPacket(const TimeRange &flush_time_range, const TimeRange &target_time_range,
				const ChunkRange &flush_chunk_range, const ChunkRange &target_chunk_range,
				const TimeRange &global_time_range);
	virtual ~FlushPacket() = default;

	virtual NodeDataType getType() override
	{ return NodeData::FLUSH; }
};

//Packet to send data that spans some time (e.g. track data)
template<typename T>
struct TimePacket : public PullPacket
{
	Range<T>	range,			//Requested (relative) range of data
				globalRange;	//Requested (global) range of data

	TimePacket();
	TimePacket(Range<T> r);
	virtual ~TimePacket() = default;

	virtual NodeDataType getType() override
	{ return PullPacket::getType() | NodeData::TIME; }
};

template<typename T>
TimePacket<T>::TimePacket()
{ }

template<typename T>
TimePacket<T>::TimePacket(Range<T> r)
	: range(r)
{ }


//Packet to send data in real time (usually push)
template<typename T>
struct LiveTimePacket : public PushPacket
{
	Range<T>	range;	//Range of pushed data

	LiveTimePacket();
	LiveTimePacket(Range<T> r);
	virtual ~LiveTimePacket() = default;

	virtual NodeDataType getType() override
	{ return PushPacket::getType() | NodeData::TIME; }
};

template<typename T>
LiveTimePacket<T>::LiveTimePacket()
{ }

template<typename T>
LiveTimePacket<T>::LiveTimePacket(Range<T> data_range)
	: range(data_range)
{ }



//Packet to pull audio data
struct AudioPullPacket : public TimePacket<c_time>
{
	AudioVelChunk**		data = nullptr;
	TransferMethod		method = TransferMethod::COPY;
	bool				update = true;

	SampleMethod		sampleMethod = SampleMethod::POINT;

	TimeRange			tRange;	//Range corresponding to given chunk range

	AudioPullPacket();
	AudioPullPacket(AudioVelChunk** data_, ChunkRange chunk_range, TransferMethod method_, bool update_data);
	virtual ~AudioPullPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePacket<c_time>::getType() | NodeData::AUDIO; }
};

//Packet to push audio data
struct AudioPushPacket: public LiveTimePacket<c_time>
{
	AudioVelChunk**		data = nullptr;
	TransferMethod		method = TransferMethod::COPY;

	AudioPushPacket();
	AudioPushPacket(AudioVelChunk** data_, ChunkRange data_range, TransferMethod method_);
	virtual ~AudioPushPacket() = default;

	virtual NodeDataType getType() override
	{ return LiveTimePacket<c_time>::getType() | NodeData::AUDIO; }
};

//Packet to pull MIDI data
struct MidiPullPacket : public TimePacket<double>
{
	MidiData						notes;
	std::vector<SampleModFunction>	mods;

	ChunkRange						cRange;	//Range corresponding to given time range

	//The offset between the current actual time and the note times (relative to beginning of the track)
	//	SO: offset is the actual time at the beginning of the track.
	//double							offset;

	MidiPullPacket();
	MidiPullPacket(TimeRange time_range);
	virtual ~MidiPullPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePacket<double>::getType() | NodeData::MIDI; }
};

//Packet to push MIDI data
struct MidiPushPacket : public LiveTimePacket<double>
{
	MidiEventQueue					events;
	std::vector<SampleModFunction>	mods;

	MidiPushPacket();
	MidiPushPacket(TimeRange data_range, const MidiEventQueue &midi_events);
	virtual ~MidiPushPacket() = default;

	virtual NodeDataType getType() override
	{ return LiveTimePacket<double>::getType() | NodeData::MIDI; }
};


//Packet to send info
struct InfoPacket : public PullPacket
{
	InfoPacket();
	virtual ~InfoPacket() = default;

	virtual NodeDataType getType() override
	{ return PullPacket::getType() | NodeData::INFO; }
};

//Packet to send audio info
struct AudioInfoPacket : public InfoPacket
{
	int bpm = 0,
		sampleRate = 0;

	AudioInfoPacket();
	AudioInfoPacket(int bpm_, int sample_rate);
	virtual ~AudioInfoPacket() = default;

	virtual NodeDataType getType() override
	{ return InfoPacket::getType() | NodeData::AUDIO_INFO; }
};

//Packet to send instrument sample function
struct InstrumentPacket : public InfoPacket
{
	NoteSampleFunction sample = nullptr;
	NoteSampleFunction sampleVel = nullptr;

	InstrumentPacket();
	InstrumentPacket(NoteSampleFunction sample_function, NoteSampleFunction sample_vel_function);
	virtual ~InstrumentPacket() = default;

	virtual NodeDataType getType() override
	{ return InfoPacket::getType() | NodeData::INSTRUMENT; }
};


#endif	//APOLLO_NODE_DATA_TYPES_H