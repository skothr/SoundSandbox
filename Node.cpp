#include "Node.h"

#include "NodePackets.h"
#include "NodeConnection.h"
//#include <GL/glew.h>
//#include "Screen.h"

//#include "Keyboard.h"
//#include "AUtility.h"
//#include <sstream>
//#include <iomanip>

#include "NodeGraph.h"

/////NODE/////
//Cursor Node::globalCursor(DEFAULT_SAMPLE_RATE, AUDIO_CHUNK_SIZE, 1);

Registry<Node>		Node::reg;
TimeRange			Node::globalRange(0.0, 0.0);

Node::Node()
	: parentGraph(nullptr), id(-1),
		type(NType::INVALID), name(""), description(""),
		nodeLock("INVALID")
{ }

Node::Node(NodeGraph *parent_graph, NType node_type, std::string node_name, std::string node_desc)
	: parentGraph(parent_graph), id(reg.registerId(this)),
		type(node_type), name(node_name), description(node_desc),
		nodeLock("Node" + std::to_string(id))
{ }

/*
Node::Node(NodeGraph *parent_graph, const NDesc &n_desc)
	: parentGraph(parent_graph), id(reg.registerId(this)),
		type(n_desc.type), name(n_desc.nameStr), description(n_desc.descStr),
		nodeLock(n_desc.typeStr)
{ }
*/

Node::Node(NodeGraph *parent_graph, const Node &other)
	: parentGraph(parent_graph), id(reg.registerId(this)),
		type(other.type), name(other.name), description(other.description),
		nodeLock("Node" + std::to_string(id))
{ }

Node::Node(const Node &other)
	: parentGraph(other.parentGraph), id(reg.registerId(this)),
		type(other.type), name(other.name), description(other.description),
		nodeLock("Node" + std::to_string(id))
{ }

Node::~Node()
{
	//Delete connectors
	//for(auto nc : connectors)
	//	nc.second.reset();
	connectors.clear();

	//Deregister this node
	reg.deregisterId(id);
	id = -1;
}

void Node::updateGlobalRange(Time g_step)
{
	//globalRange.end = HRes_Clock::getGlobalTime();
	//globalRange.start = globalRange.end - g_step;
	globalRange.start = globalRange.end;
	globalRange.end = HRes_Clock::getGlobalTime();
}

std::vector<NCID> Node::init(const std::vector<NodeConnectorDesc> &nc_desc)
{
	std::vector<NCID> ids;

	if(!initialized)
	{
		connectors.reserve(nc_desc.size());
		ids.reserve(nc_desc.size());

		for(auto ncd : nc_desc)
		{
			NodeConnector *nc = new NodeConnector(this, ncd.dataType, ncd.ioType, ncd.name, ncd.desc, ncd.maxConnections);
			
			ids.push_back(nc->id);
			auto &iter = connectors.emplace(nc->id, std::unique_ptr<NodeConnector>(nc));

			nc = iter.first->second.get();

			std::unordered_map<NCID, NodeConnector&> *dirConnectors = &(isInput(ncd.ioType) ? inputConnectors : outputConnectors);
			dirConnectors->emplace(nc->id, *nc);
		}
	}

	return ids;
}


NID Node::getId() const
{
	return id;
}

bool Node::isLocked() const
{
	return nodeLock.isLocked();
}

bool Node::isDirty() const
{
	return dirty;
}

/*
bool Node::connect(NCID this_nc, NCID other_nc)
{
	bool success = connectors[this_nc]->makeConnection(other_nc);

	if(success)
		parentGraph->addConnection(connectors[this_nc]->connections[other_nc]);

	return success;
}

bool Node::disconnect(NCID this_nc, NCID other_nc)
{
	bool success = connectors[this_nc]->destroyConnection(other_nc);

	if(success)
		parentGraph->removeConnection(connectors[this_nc]->connections[other_nc]);

	return success;
}

void Node::disconnectAll(NCID this_nc)
{
	connectors[this_nc]->destroyAllConnections();
}
*/

/*
std::vector<NCOwnedPtr> Node::getConnectors()
{
	std::vector<NCOwnedPtr> ncs;
	ncs.reserve(connectors.size());
	//ncs.insert(ncs.end(), connectors.begin(), connectors.end());
	for(auto nc : connectors)
		ncs.push_back(nc.second);

	return ncs;
}
*/

void Node::getChildNodes(const std::vector<NodeType> &types, std::vector<ChildNodeTree> &results)
{
	for(auto nc : inputConnectors)
	{
		for(auto c : nc.second.connections)
		{
			NodeConnector *ncc = NodeConnector::getNC(c.first);
			Node *n = ncc->getNode();
			if(std::find(types.begin(), types.end(), n->getType()) != types.end())
			{
				results.push_back(ChildNodeTree(n));
				n->getChildNodes(types, results[results.size() - 1].children);
			}
			else
			{
				n->getChildNodes(types, results);
			}
		}
	}
}

/*
bool Node::flushData(FlushPacket &info)
{
	bool flushed = false;

	nodeLock.lockWait();

	for(auto nc : inputConnectors)
	{
		flushed |= nc.second->flushData(info);
	}

	nodeLock.unlock();

	return flushed;
}
*/

NType Node::getType() const
{
	return type;
}


void Node::update(const Time &dt)
{
	nodeLock.lockWait();

	for(auto &nc : connectors)
		nc.second->update(dt);

	nodeLock.unlock();

	onUpdate(dt);
}
