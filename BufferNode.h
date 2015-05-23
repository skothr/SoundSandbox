#ifndef APOLLO_BUFFER_NODE
#define APOLLO_BUFFER_NODE

#include "Node.h"

//A Node that functions as a buffer --> Static unless recording onto it
//TODO: Decide on name (RecordNode? RecordBufferNode?) and rename file
class RecordBufferNode : public Node
{
protected:

public:
	RecordBufferNode();
	virtual ~RecordBufferNode();
};




#endif	//APOLLO_BUFFER_NODE