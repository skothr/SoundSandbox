#include "NodeBaseTypes.h"

#include "Samplers.h"
#include "AUtility.h"

#include "NodePackets.h"

#include "Cursor.h"


/////TRACK NODE/////
//(Template, defined in header)


/////MOD NODE/////

ModNode::ModNode()
	: Node(nullptr, NType::INVALID, "Null Mod Node", "Unspecified")
{

}



/////INPUT NODE/////

InputNode::InputNode()
	: Node(nullptr, NType::INVALID, "Null Read Node", "Unspecified")
{

}

/*
bool InputNode::canPush()
{
	return false;
}
*/

/////OUTPUT NODE/////

OutputNode::OutputNode()
	: Node(nullptr, NType::INVALID, "Null Write Node", "Unspecified")
{

}

/*
bool OutputNode::canPull()
{
	return false;
}


bool OutputNode::canFlush()
{
	return false;
}
*/