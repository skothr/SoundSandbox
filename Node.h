#ifndef APOLLO_NODE_H
#define APOLLO_NODE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <memory>

#include "Registry.h"
#include "OwnedMutex.h"
//#include "Saveable.h"

//TODO: Remove this!!
#include "Cursor.h"
////

#include "NodeResources.h"
#include "Screen.h"


class Node;
class NodeConnector;
class NodeConnection;

class PushPacket;
class PullPacket;
class FlushPacket;

class NodeGraph;

//Node base class -- defines basic Node functionality.
class Node
{
private:
	static Registry<Node>		reg;

	bool						initialized = false;

protected:

	//Holds current range of global times for the current chunk
	static TimeRange			globalRange;	

	static void updateGlobalRange(Time g_step);

	NodeGraph					*parentGraph = nullptr;

	NID							id = -1;					//This Node's id
	NType						type;						//Type of Node

	bool						dirty			= true;		//Whether this Node is dirty (needs to be updated)

	bool						pushing			= false,	//Whether this node is trying to push/pull the next node.
								pulling			= false;

	std::unordered_map<NCID, std::unique_ptr<NodeConnector>>	connectors;	
	std::unordered_map<NCID, NodeConnector&>					inputConnectors,
																outputConnectors;

	//NCMap						connectors,
	//							inputConnectors,			//Only input connectors
	//							outputConnectors;			//Only output connectors

	OwnedMutex					nodeLock;					//A mutex for thread safety

	
	std::vector<NCID> init(const std::vector<NodeConnectorDesc> &node_connectors);
	
	virtual void onConnect(NCID this_nc, NCID other_nc)		{ }		//Executed when the Node is connected
	virtual void onDisconnect(NCID this_nc, NCID other_nc)	{ }		//Executed when the Node is disconnected
	
	virtual void onUpdate(const Time &dt)						{ }		//Executed when the Node is updated
	//virtual void updateDesc() override;

	Node();

	//TODO: getCursor() function that looks ahead and finds the cursor attached to this path (if there is one)

public:
	Node(NodeGraph *parent_graph, NType node_type, std::string node_name, std::string node_desc);
	//Node(NodeGraph *parent_graph, const NDesc &n_desc);
	Node(NodeGraph *parent_graph, const Node &other);
	Node(const Node &other);
	virtual ~Node();

	Point2f graphPos = Point2f(0.0f, 0.0f);
	
	std::string	name			= "",		//This Node's name
				description		= "";		//A description of this Node

	NID getId() const;

	bool isLocked() const;
	bool isDirty() const;

	//bool connect(NCID this_nc, NCID other_nc);
	//bool disconnect(NCID this_nc, NCID other_nc);
	//void disconnectAll(NCID this_nc);

	//std::vector<NCOwnedPtr> getConnectors();
	
	void getChildNodes(const std::vector<NodeType> &types, std::vector<ChildNodeTree> &results);

	virtual Cursor* getCursor() { return nullptr; }
	
	//These functions define functionality of data communication with this Node.
	//virtual bool flushData(FlushPacket &info);
	virtual bool pullData(PullPacket &output, NCID this_id, NCID other_id) = 0;
	virtual bool pushData(PushPacket &input, NCID this_id, NCID other_id) = 0;

	virtual void update(const Time &dt);

	NType getType() const;

	friend class NodeConnector;
	friend class NodeConnection;
	friend class NodeGraph;

	friend class NodeControl;

	//TEMP (for cursor)
	friend class SandboxWindow;
	friend class NodeConnection;
};

#endif	//APOLLO_NODE_H