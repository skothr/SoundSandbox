#ifndef APOLLO_DYNAMIC_BUFFER_NODE_H
#define APOLLO_DYNAMIC_BUFFER_NODE_H

#include "BufferNode.h"

template<typename TimeUnit, typename DataType>
class DynamicBufferNode : public virtual BufferNode<TimeUnit, DataType>
{
protected:

public:
	DynamicBufferNode();
	virtual ~DynamicBufferNode();

	virtual void update(const Time &dt) override;
};

template<typename TimeUnit, typename DataType>
DynamicBufferNode<TimeUnit, DataType>::DynamicBufferNode()
	: Node(nullptr, NType::INVALID, "Null Dynamic Buffer Node", "Unspecified"),
		BufferNode(0, DataType())
{ }


template<typename TimeUnit, typename DataType>
DynamicBufferNode<TimeUnit, DataType>::~DynamicBufferNode()
{ }


template<typename TimeUnit, typename DataType>
void DynamicBufferNode<TimeUnit, DataType>::update(const Time &dt)
{

}

/*
template<typename TimeUnit, typename DataType>
DynamicBufferNode::
*/


#endif	//APOLLO_DYNAMIC_BUFFER_NODE_H