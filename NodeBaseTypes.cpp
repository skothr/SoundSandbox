#include "NodeBaseTypes.h"

#include "Samplers.h"
#include "AUtility.h"


/////TRACK NODE/////
//(Template, defined in header)


/////MOD NODE/////

ModNode::ModNode()
	: Node(NType::INVALID, "Null Mod Node", "Unspecified")
{

}



/////READ NODE/////

ReadNode::ReadNode()
	: Node(NType::INVALID, "Null Read Node", "Unspecified")
{

}

/*
bool ReadNode::canPush()
{
	return false;
}
*/

/////WRITE NODE/////

WriteNode::WriteNode()
	: Node(NType::INVALID, "Null Write Node", "Unspecified")
{

}

/*
bool WriteNode::canPull()
{
	return false;
}


bool WriteNode::canFlush()
{
	return false;
}
*/