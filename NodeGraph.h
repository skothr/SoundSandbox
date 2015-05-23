#ifndef APOLLO_NODE_GRAPH_H
#define APOLLO_NODE_GRAPH_H

//Include all Node resources
#include "Node.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>

class NodeConnector;
class ProjectTrackDisplay;

enum class NodeGraphPreset
{
	INVALID = -1,
	EMPTY = 0,
	SANDBOX,
	PROJECT,

	COUNT
};

inline unsigned int toIndex(NodeGraphPreset ngp)
{
	return static_cast<unsigned int>(ngp);
}

extern int midi_port;


//Represents a Node in a graph (includes position)
struct GraphNode
{
	Node		*node;
	Point2f		pos;

	GraphNode(Node *n, Point2f graph_pos)
		: node(n), pos(graph_pos)
	{ }
};

struct NodeTree
{
private:
	//static std::unordered_map<Node*, NodeTree*> traversedNodes;
	//static bool handled;	//Whether the static stuff is being handled (top node handles stuff)

	void buildTree();

public:
	Node *node = nullptr;

	std::vector<NodeTree*> forward;		//Forward connections
	std::vector<NodeTree*> backward;	//Backward connections

	//Builds a tree starting with the given node
	NodeTree(Node *start_node);
	~NodeTree();

	void print(int level = 0);
};


class NodeGraph
{
private:
	//static std::vector<Point2f> presetPositions[static_cast<unsigned int>(NodeGraphPreset::COUNT)];

	std::unordered_map<Node*, Point2f>		nodes;
	std::unordered_set<COwnedPtr>			nodeConnections;
	

public:
	NodeGraph();
	NodeGraph(const std::unordered_map<Node*, Point2f> &g_nodes);
	~NodeGraph();
	
	void addConnection(COwnedPtr nc);
	void removeConnection(COwnedPtr nc);

	void addNode(Node *n, Point2f g_pos);
	void removeNode(Node *n);

	const std::unordered_map<Node*, Point2f>* getNodes();
	//const std::vector<Point2f>& getInitialPos();
	//NodeTree* getNodeTree();

	unsigned int numNodes();

	void reset();
	void setPreset(NodeGraphPreset preset, int sample_rate);

	void update(double dt);

	friend class NodeConnector;
};


#endif	//APOLLO_NODE_GRAPH_H