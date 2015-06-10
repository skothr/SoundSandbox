#ifndef APOLLO_NODE_PACKETS_H
#define APOLLO_NODE_PACKETS_H

#include "NodeResources.h"
#include "Range.h"
#include "Timing.h"
#include "Sampling.h"

#include "MIDI.h"
#include "MidiData.h"


//
//
//TODO:  Make another type of packet that can request multiple chunks!
//
//

class AudioVelChunk;
class Cursor;

////PACKETS////

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


//Packet to send a chunk of data
struct TimePullPacket : public PullPacket
{
	Cursor *pCursor = nullptr;	//Pointer to parent cursor
	//bool updateCursor = false;
	
	//const TimeRange		globalRange;		//Global range of the relevant chunk

	TimePullPacket(Cursor *p_cursor);//, bool update_cursor);
	//TimePullPacket(TimeRange global_range, Cursor *active_cursor);
	virtual ~TimePullPacket() = default;

	virtual NodeDataType getType() override
	{ return PullPacket::getType() | NodeData::TIME; }
};



//Packet to send data in real time (usually push)
struct TimePushPacket : public PushPacket
{
	//Cursor *activeCursor;

	TimePushPacket();
	//TimePushPacket(Cursor *active_cursor);
	virtual ~TimePushPacket() = default;

	virtual NodeDataType getType() override
	{ return PushPacket::getType() | NodeData::TIME; }
};


//Packet to pull audio data
struct AudioPullPacket : public TimePullPacket
{
	AudioVelChunk*		data;
	TransferMethod		method = TransferMethod::COPY;
	SampleMethod		sampleMethod = SampleMethod::POINT;

	AudioPullPacket(Cursor *p_cursor, AudioVelChunk *data_);
	AudioPullPacket(Cursor *p_cursor, AudioVelChunk *data_, TransferMethod t_method);
	virtual ~AudioPullPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePullPacket::getType() | NodeData::AUDIO; }
};

//Packet to push audio data
struct AudioPushPacket: public TimePushPacket
{
	AudioVelChunk*		data;
	//AudioVelChunk*		lastData;
	TransferMethod		method = TransferMethod::COPY;

	//Time			dataStart;

	AudioPushPacket(AudioVelChunk *data_);//, AudioVelChunk *last_data, Time data_start);
	AudioPushPacket(AudioVelChunk* data_, /*AudioVelChunk *last_data, Time data_start,*/ TransferMethod method_);
	virtual ~AudioPushPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePushPacket::getType() | NodeData::AUDIO; }
};

//Packet to pull MIDI data (in the form of notes intersecting this chunk)
struct MidiPullPacket : public TimePullPacket
{
	MidiSet							notes;
	std::vector<SampleModFunction>	mods;

	MidiPullPacket(Cursor *p_cursor);//, bool update_cursor);
	virtual ~MidiPullPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePullPacket::getType() | NodeData::MIDI; }
};

//Packet to push MIDI data (in the form of events)
struct MidiPushPacket : public TimePushPacket
{
	MidiEvent						event;
	std::vector<SampleModFunction>	mods;

	MidiPushPacket(MidiEvent e);
	//MidiPushPacket(MidiEvent e, const std::vector<SampleModFunction> &initial_mods);
	virtual ~MidiPushPacket() = default;

	virtual NodeDataType getType() override
	{ return TimePushPacket::getType() | NodeData::MIDI; }
};

/*
//Packet to push MIDI data one event at a time
struct MidiEventPushPacket : public PushPacket
{
	MidiEvent e;
	std::vector<SampleModFunction>	mods;
	
	MidiEventPushPacket();
	MidiEventPushPacket(const MidiEvent &e_);
	virtual ~MidiEventPushPacket() = default;

	virtual NodeDataType getType() override
	{ return PushPacket::getType() | NodeData::MIDI; }
};
*/

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

/*
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
	Range<T>	range;					//Requested (relative) range of data
	
	const TimeRange	globalTimeRange;	//Requested (global) range of data

	TimePacket();
	TimePacket(Range<T> r, TimeRange global_range);
	virtual ~TimePacket() = default;

	virtual NodeDataType getType() override
	{ return PullPacket::getType() | NodeData::TIME; }
};

template<typename T>
TimePacket<T>::TimePacket()
{ }

template<typename T>
TimePacket<T>::TimePacket(Range<T> r, TimeRange global_range)
	: range(r), globalTimeRange(global_range)
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
	AudioPullPacket(AudioVelChunk** data_, ChunkRange chunk_range, TimeRange global_range, TransferMethod method_, bool update_data);
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
	MidiPullPacket(TimeRange time_range, TimeRange global_range);
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

//Packet to push MIDI data one event at a time
struct MidiEventPushPacket : public PushPacket
{
	MidiEvent e;
	std::vector<SampleModFunction>	mods;
	
	MidiEventPushPacket();
	MidiEventPushPacket(const MidiEvent &e_);
	virtual ~MidiEventPushPacket() = default;

	virtual NodeDataType getType() override
	{ return PushPacket::getType() | NodeData::MIDI; }
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
*/

#endif	//APOLLO_NODE_PACKETS_H