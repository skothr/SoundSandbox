#ifndef APOLLO_CONTEXT_TREE_H
#define APOLLO_CONTEXT_TREE_H

#include "Callbacks.h"
#include <string>
#include <vector>

struct ContextTree
{
	std::string		label = "";
	voidCallback	action = nullptr;

	std::vector<ContextTree> children = {};

	ContextTree() = default;
	ContextTree(const std::string &label_, voidCallback click_action);

	void addChild(const ContextTree &child);
};


#endif	//APOLLO_CONTEXT_TREE_H