#ifndef APOLLO_BUFFER_NODE
#define APOLLO_BUFFER_NODE

#include <vector>

#include "Node.h"
#include "AudioData.h"
#include "Cursor.h"

//A Node that functions as a buffer --> can be static(can record) or dynamic(automatically updates when data is requested)
template<typename TimeUnit, typename DataType>
class BufferNode : public virtual Node
{
	/*
protected:
	//Extra members(private) for STATIC buffer (dynamic = false)
	template<bool D = false>
	struct ExtraPrivateMembers
	{
		Cursor *pCursor = nullptr;
	};
	//Extra members(private) for DYNAMIC buffer (dynamic = true)
	template<>
	struct ExtraPrivateMembers<true>
	{

	};

public:
	//Extra members(public) for STATIC buffer (dynamic = false)
	template<bool D = false>
	struct ExtraPublicMembers
	{

	};
	//Extra members(public) for DYNAMIC buffer (dynamic = true)
	template<>
	struct ExtraPublicMembers<true>
	{

	};
	*/

protected:
	//DEFAULT BUFFER//
	DataType				data;
	TimeUnit				length;

	//A cursor, to tell where in the buffer is currently active.
	Cursor					cursor;
	
	std::vector<ChildNodeTree> childNodes;

	virtual void onLengthChanged(TimeUnit d_length)	{ }




	//STATIC BUFFER//



	//DYNAMIC BUFFER//


public:
	//DEFAULT BUFFER//
	BufferNode(TimeUnit initial_length, const DataType &initial_data);
	virtual ~BufferNode();

	void setLength(TimeUnit new_length);
	TimeUnit getLength() const;

	const DataType* getData() const;

	virtual Cursor* getCursor() override;

	//STATIC BUFFER//
	//template<bool D = dynamic, class = typename std::enable_if<!D>::type>
	//void setRecording(Cursor *p_record_cursor);	//Set to null to stop recording
	
	//template<bool D = dynamic, class = typename std::enable_if<!D>::type>
	//bool isRecording() const;


	//DYNAMIC BUFFER//
	//template<bool D = dynamic, class = typename std::enable_if<!D>::type>
	//void placeholder();

};

template<typename TimeUnit, typename DataType>
BufferNode<TimeUnit, DataType>::BufferNode(TimeUnit initial_length, const DataType &initial_data)
	: Node(nullptr, NType::INVALID, "Null Mod Node", "Unspecified"),
		length(initial_length), data(initial_data), cursor(DEFAULT_SAMPLE_RATE, AUDIO_CHUNK_SIZE, 1)
{ }

template<typename TimeUnit, typename DataType>
BufferNode<TimeUnit, DataType>::~BufferNode()
{
	//data.clear();
}

template<typename TimeUnit, typename DataType>
void BufferNode<TimeUnit, DataType>::setLength(TimeUnit new_length)
{
	TimeUnit d_length = new_length - length;
	length = new_length;

	onLengthChanged(d_length);
}

template<typename TimeUnit, typename DataType>
TimeUnit BufferNode<TimeUnit, DataType>::getLength() const
{
	return length;
}

template<typename TimeUnit, typename DataType>
const DataType* BufferNode<TimeUnit, DataType>::getData() const
{
	return &data;
}

template<typename TimeUnit, typename DataType>
Cursor* BufferNode<TimeUnit, DataType>::getCursor()
{
	return &cursor;
}

/*
template<typename TimeUnit, typename DataType, true>
class BufferNode : public virtual Node
{

};
*/

#endif	//APOLLO_BUFFER_NODE