#ifndef APOLLO_STATIC_BUFFER_NODE
#define APOLLO_STATIC_BUFFER_NODE

#include "Node.h"
#include "BufferNode.h"
#include "Cursor.h"

template<typename TimeUnit, typename DataType>
class StaticBufferNode : public virtual BufferNode<TimeUnit, DataType>
{
protected:
	bool recording = false;

public:
	StaticBufferNode();
	virtual ~StaticBufferNode();

	//Recording means it's ready to record --> only actually records when cursor plays
	void setRecording(bool is_recording);	//Set to null to stop recording

	bool isRecording() const;

};

template<typename TimeUnit, typename DataType>
StaticBufferNode<TimeUnit, DataType>::StaticBufferNode()
	: Node(nullptr, NType::INVALID, "Null Static Buffer Node", "Unspecified"),
		BufferNode(0, DataType())
{ }

template<typename TimeUnit, typename DataType>
StaticBufferNode<TimeUnit, DataType>::~StaticBufferNode()
{ }

template<typename TimeUnit, typename DataType>
void StaticBufferNode<TimeUnit, DataType>::setRecording(bool is_recording)
{
	recording = is_recording;
}

template<typename TimeUnit, typename DataType>
bool StaticBufferNode<TimeUnit, DataType>::isRecording() const
{
	return recording;
}


#endif	//APOLLO_STATIC_BUFFER_NODE