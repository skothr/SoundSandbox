#include "ContextTree.h"


/////CONTEXT TREE/////

ContextTree::ContextTree(const std::string &label_, voidCallback click_action)
	: label(label_), action(click_action)
{ }

void ContextTree::addChild(const ContextTree &child)
{
	children.push_back(child);
}