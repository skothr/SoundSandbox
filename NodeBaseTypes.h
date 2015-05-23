#ifndef APOLLO_NODE_TYPES_H
#define APOLLO_NODE_TYPES_H

#include "Node.h"

#include <vector>

#include "Timing.h"
#include "Cursor.h"
#include "Audio.h"
#include "MIDI.h"
#include "AStatus.h"

#include "Interpolation.h"

class Waveform;

/////TRACK NODE STUFF/////
struct TrackPoint
{
	s_time time;
	double value;
	InterpolationType interp;	//Interpolation for after this point
	std::vector<double> data;

	TrackPoint(s_time t, double val, InterpolationType interp_, std::vector<double> data_ = {})
		: time(t), value(val), interp(interp_), data(data_)
	{ }
};

//Represents a Node that spans through time --> time units defined by T
template<typename T>
class TrackNode : public virtual Node
{
protected:
	T			length,						//Accessible track length
				maxLength;					//Actual track length

	Cursor		*pRecordCursor = nullptr;	//Points to cursor that represents where the track is recording to in its data
	//double		recordStart = -1.0;		//Holds the time that the track started recording


	//max_length_changed --> how much maxLength changed (if at all)
	virtual void onLengthChanged(T max_length_changed);

	virtual void onRecordChanged(Cursor *p_old_cursor, bool recording);

public:
	TrackNode();
	TrackNode(T initial_length);
	virtual ~TrackNode() = default;

	void setLength(T new_length);
	T getLength() const;
	T getMaxLength() const;

	void setRecording(Cursor *record_cursor);
	bool isRecording() const;

	//Clears a range of data.
	//	If compress is true, the data within the range should be deleted, and the track's length
	//	shortened around it accordingly.
	virtual void clearRange(Range<T> r, bool compress) = 0;

	//virtual AStatus pullData(NodePacket &output); // = 0;
	//virtual AStatus pushData(NodePacket &input, NCID nc_id); // = 0;

	//virtual NodeType getType() override { return NODE_TRACK; }
};

template<typename T>
TrackNode<T>::TrackNode()
	: TrackNode((T)0)
{ }

template<typename T>
TrackNode<T>::TrackNode(T initial_length)
	: Node(NType::INVALID, "Null Track Node", "Unspecified"), length(initial_length), maxLength(initial_length)
{ }

template<typename T>
void TrackNode<T>::onLengthChanged(T max_length_changed)
{ }

template<typename T>
void TrackNode<T>::onRecordChanged(Cursor *p_old_cursor, bool recording)
{ }

template<typename T>
void TrackNode<T>::setLength(T new_length)
{
	length = new_length;

	//Grow maxLength if length is now bigger
	T diff = (length - maxLength)*(length > maxLength);
	maxLength += diff;
	
	onLengthChanged(diff);
}

template<typename T>
T TrackNode<T>::getLength() const
{
	return length;
}

template<typename T>
T TrackNode<T>::getMaxLength() const
{
	return maxLength;
}

template<typename T>
void TrackNode<T>::setRecording(Cursor *record_cursor)
{
	nodeLock.lockWait();

	Cursor *old = pRecordCursor;
	pRecordCursor = record_cursor;
	onRecordChanged(old, static_cast<bool>(record_cursor));
	//recordStart = (record_cursor ? AUDIO_CLOCK.getExactTime() : -1.0);

	nodeLock.unlock();
}

template<typename T>
bool TrackNode<T>::isRecording() const
{
	return static_cast<bool>(pRecordCursor);
}



//Represents a Node that modifies aspects of the information that passes through it.
class ModNode : public virtual Node
{
protected:

public:
	ModNode();
	virtual ~ModNode() = default;
	
	//virtual AStatus pullData(NodePacket &output); // = 0;
	//virtual AStatus pushData(NodePacket &input, NCID nc_id); // = 0;
	
	//virtual NodeType getType() override { return NODE_MOD; }
};

//Represents a Node that can only be read from
class ReadNode : public virtual Node
{
protected:

public:
	ReadNode();
	virtual ~ReadNode() = default;
	
	//virtual AStatus pullData(NodePacket &output);	// = 0;
	
	//virtual bool canPush() override;

	virtual bool pushData(PushPacket &input, NCID this_id) override
	{ return false; }
};

//Represents a Node that can only be written to
class WriteNode : public virtual Node
{
protected:

public:
	WriteNode();
	virtual ~WriteNode() = default;
	
	//virtual bool canPull() override;
	//virtual bool canFlush() override;

	virtual bool pullData(PullPacket &output, NCID this_id) override
	{ return false; }
	
	//virtual AStatus pushData(NodePacket &input, NCID nc_id);	// = 0;
};


#endif	//APOLLO_NODE_TYPES_H