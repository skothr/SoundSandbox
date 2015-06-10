#ifndef APOLLO_NODE_GRAPH_H
#define APOLLO_NODE_GRAPH_H

//Include all Node resources
#include "Vector.h"
#include "NodeResources.h"
#include "Timing.h"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

class Node;
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

	//Map --> Node id to node UNIQUE ptr
	std::unordered_map<NID, std::unique_ptr<Node>>				nodes;
	//Set of connections between nodes
	std::unordered_map<CID, std::unique_ptr<NodeConnection>>	connections;
	
	Node* getNode(NID id);
	
public:
	NodeGraph();
	NodeGraph(std::unordered_map<Node*, Point2f> &g_nodes, std::unordered_set<NodeConnection*> g_connections);
	~NodeGraph();

	void addNode(Node *n, Point2f g_pos);
	bool removeNode(NID id);
	
	//Creates a new connection between the specified nodes
	//bool connectNodes(NID n1, NID n2);
	//bool connectNodes(Node& n1, Node& n2);

	//Creates a new connection between the specified nodeconnectors. Returns pointer to created connection.
	NodeConnection* makeNewConnection(NCID n1, NCID n2);
	NodeConnection* makeNewConnection(NCID nc);
	bool removeConnection(CID id);

	const std::unordered_map<NID, Node*> getNodes();
	const std::unordered_map<CID, NodeConnection*> getConnections();
	
	unsigned int getNumNodes() const;
	unsigned int getNumConnections() const;

	//NodeTree* getNodeTree();

	void resetGraph();
	void setPreset(NodeGraphPreset preset, int sample_rate);

	void update(const Time &dt);
	void resetConnectionStates();

	//Stops all threads that are running on nodes' devices
	void stopAllDevices();

	friend class NodeConnector;
};


#endif	//APOLLO_NODE_GRAPH_H