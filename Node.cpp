#include "Node.h"

#include "NodeConnection.h"
//#include <GL/glew.h>
//#include "Screen.h"

//#include "Keyboard.h"
//#include "AUtility.h"
//#include <sstream>
//#include <iomanip>

#include "NodeGraph.h"


/////NODE/////
NodeMap		Node::registeredNodes;
NID			Node::nextId = 0;

Node::Node(NType node_type, std::string node_name, std::string node_desc)
	: type(node_type), name(node_name), description(node_desc)
{
	registerId();
}

Node::Node(const NDesc &n_desc)
	: name(n_desc.nameStr), /*type(n_desc.typeStr),*/ description(n_desc.descStr), nodeLock(n_desc.typeStr)
{
	/*
	id = desc->id;
	//Add id to static list, and update nextId
	regIds.emplace(id, this);
	nextId = max(id + 1, nextId);

	//Update connectors
	connectors.clear();
	connectors.reserve(desc->connectors.size());
	connectorList.reserve(desc->connectors.size());

	for(unsigned int i = 0; i < connectors.size(); i++)
	{
		NCID nc_id = desc->connectors[i];
		NodeConnector *nc = NodeConnector::getNodeConnector(nc_id);

		connectors.emplace(nc_id, nc);
		connectorList.push_back(nc);
	}
	*/
}

Node::Node(const Node &other)
	: type(other.type), name(other.name), description(other.description)
{
	registerId();
}

Node::~Node()
{
	deregisterId();
}


bool Node::registerId()
{
	id = nextId;
	nextId++;
	registeredNodes.emplace(id, this);

	return true;
}

bool Node::deregisterId()
{
	auto iter = registeredNodes.find(id);

	if(iter != registeredNodes.end())
	{
		registeredNodes.erase(id);
		id = 0;
		return true;
	}
	else
	{
		std::cout << "ERROR: Node (id = " << id << ") failed to be deregistered.\n";
		return false;
	}
}

bool Node::validId(NID id)
{
	return (id >= 0 && registeredNodes.find(id) != registeredNodes.end());
}


std::vector<NCID> Node::init(const std::vector<NodeConnectorDesc> &nc_desc)
{
	connectors.clear();
	connectors.reserve(nc_desc.size());
	
	inputConnectors.clear();
	outputConnectors.clear();

	std::vector<NCID> ids;
	ids.reserve(nc_desc.size());

	for(auto ncd : nc_desc)
	{
		NodeConnector *nc = new NodeConnector(this, ncd.dataType, ncd.ioType, ncd.name, ncd.desc, ncd.maxConnections);
		ids.push_back(nc->id);
		connectors.emplace(nc->id, nc);
		
		if(isInput(ncd.ioType))
			inputConnectors.emplace(nc->id, nc);
		else
			outputConnectors.emplace(nc->id, nc);
	}

	return ids;
}

void Node::init(const std::vector<NCID> &nc_ids)
{
	connectors.clear();
	connectors.reserve(nc_ids.size());

	for(auto ncid : nc_ids)
	{
		NodeConnector *nc = NodeConnector::getNC(ncid);
		connectors.emplace(ncid, nc);
	}
}


void Node::getChildNodes(const std::vector<NodeType> &types, std::vector<ChildNodeTree> &results)
{
	for(auto nc : inputConnectors)
	{
		for(auto c : nc.second->connections)
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


NID Node::getId() const
{
	return id;
}

std::string Node::getName() const
{
	return name;
}

std::string Node::getDesc() const
{
	return description;
}

void Node::setName(const std::string &new_name)
{
	name = new_name;
}

void Node::setDesc(const std::string &new_desc)
{
	description = new_desc;
}

bool Node::isLocked() const
{
	return nodeLock.isLocked();
}

bool Node::isDirty() const
{
	return dirty;
}


bool Node::connect(NCID this_nc, NCID other_nc)
{
	bool success = connectors[this_nc]->connect(other_nc);
	onConnect(this_nc, other_nc);
	NodeConnector::getNC(other_nc)->getNode()->onConnect(other_nc, this_nc);

	return success;
}

bool Node::disconnect(NCID this_nc, NCID other_nc)
{
	bool success = connectors[this_nc]->disconnect(other_nc);
	onDisconnect(this_nc, other_nc);
	NodeConnector::getNC(other_nc)->getNode()->onDisconnect(other_nc, this_nc);

	return success;
}

void Node::disconnectAll(NCID this_nc)
{
	//TODO: Implement this
	/*
	NodeConnector *nc = connectors[this_nc];

	std::vector<NCID> other_ncs;
	other_ncs.reserve(nc->numConnections());

	nc->disconnectAll();

	for(auto onc : other_ncs)
	*/
}

std::vector<NCOwnedPtr> Node::getConnectors()
{
	std::vector<NCOwnedPtr> ncs;
	ncs.reserve(connectors.size());
	//ncs.insert(ncs.end(), connectors.begin(), connectors.end());
	for(auto nc : connectors)
		ncs.push_back(nc.second);

	return ncs;
}

bool Node::flushData(FlushPacket &info)
{
	bool flushed = false;


	return flushed;
}


NType Node::getType() const
{
	return type;
}

void Node::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new NDesc()));
	NDesc *desc = dynamic_cast<NDesc*>(objDesc);
	
	desc->id = id;
	desc->type = getType();

	desc->nameStr = name;
	desc->typeStr = "";
	desc->descStr = description;

	desc->dirty = dirty;

	desc->connectors.clear();
	desc->connectors.reserve(connectors.size());

	//for(unsigned int i = 0; i < connectorList.size(); i++)
	//	desc->connectors.push_back(connectorList[i]->getId());
	for(auto nc : connectors)
		desc->connectors.push_back(nc.first);
}


void Node::update(double dt)
{
	for(auto nc : connectors)
		nc.second->update(dt);

	onUpdate(dt);
}


/*


/////NODE/////
NodeList	Node::regIds;
NodeId		Node::nextId = 1;

Node::Node(std::string node_type, std::string node_desc)
	: nodeName(node_type), nodeType(node_type), nodeDesc(node_desc), nodeLock(node_type)
{
	AStatus status;

	if(!statusGood(status = registerId()))
		std::cout << "FAILED TO REGISTER NODE:\n"  << status << "\n";
}

Node::Node(const NDesc &n_desc)
	: nodeName(n_desc.nameStr), nodeType(n_desc.typeStr), nodeDesc(n_desc.descStr), nodeLock(n_desc.typeStr)
{
	id = desc->id;
	//Add id to static list, and update nextId
	regIds.emplace(id, this);
	nextId = max(id + 1, nextId);

	//Update connectors
	connectors.clear();
	connectors.reserve(desc->connectors.size());
	connectorList.reserve(desc->connectors.size());

	for(unsigned int i = 0; i < connectors.size(); i++)
	{
		NCID nc_id = desc->connectors[i];
		NodeConnector *nc = NodeConnector::getNodeConnector(nc_id);

		connectors.emplace(nc_id, nc);
		connectorList.push_back(nc);
	}
}

Node::~Node()
{
	nodeLock.lockWait();

	for(auto c : connectors)
		AU::safeDelete(c.second);
	connectors.clear();
	
	AStatus status;
	if(!statusGood(status = deregisterId()))
		std::cout << "FAILED TO DEREGISTER NODE:\n" << status << "\n";
	
	nodeLock.unlock();
}


AStatus Node::registerId()
{
	AStatus status;
	
	id = nextId;
	regIds.emplace(id, this);
	nextId++;

	return status;
}

AStatus Node::deregisterId()
{
	AStatus status;
	
	auto id_iter = regIds.find(id);

	if(id_iter != regIds.end())
		regIds.erase(id_iter);
	else
		status.setWarning(AS::WType::NOT_REGISTERED, "NodeConnector ID is not registered.");

	id = 0;

	return status;
}

NodeId Node::getId()
{
	return id;
}

std::vector<NCID> Node::init(std::vector<NodeConnectorDesc> &node_connectors)
{
	AStatus status;

	connectors.clear();
	connectors.reserve(node_connectors.size());
	connectorList.reserve(node_connectors.size());

	std::vector<NCID> ids(node_connectors.size());

	//Define node connections
	for(unsigned int i = 0; i < node_connectors.size(); i++)
	{
		NodeConnector *nc = new NodeConnector(this, node_connectors[i]);
		ids[i] = nc->getId();
		connectors.emplace(ids[i], nc);
		connectorList.push_back(nc);
	}

	return ids;
}

void Node::init(const std::vector<NCID> &nc_ids)
{
	connectors.clear();
	connectors.reserve(nc_ids.size());
	connectorList.reserve(nc_ids.size());
	
	//Define node connections
	for(unsigned int i = 0; i < nc_ids.size(); i++)
	{
		NodeConnector *nc = NodeConnector::getNodeConnector(nc_ids[i]);
		connectors.emplace(nc_ids[i], nc);
		connectorList.push_back(nc);
	}
}

Node* Node::getNode(NodeId n_id)
{
	return regIds[n_id];
}


AStatus Node::connect(NCID nc1, NCID nc2)
{
	return (nc1 ? NodeConnector::getNodeConnector(nc1)->connect(nc2) : NodeConnector::getNodeConnector(nc2)->connect(nc1));
}


//Public functions
void Node::setName(std::string name)
{
	nodeName = name;
}

void Node::setDescription(std::string desc)
{
	nodeDesc = desc;
}

std::string Node::getName()
{
	return nodeName;
}

std::string Node::getDescription()
{
	return nodeDesc;
}

unsigned int Node::numConnectors() const
{
	return connectors.size();
}

const std::vector<NodeConnector*>& Node::getConnectors()
{
	return connectorList;
}


NodeConnector* Node::getConnector(NCID id)
{
	NCList::iterator iter = connectors.find(id);
	NodeConnector *nc = (iter == connectors.end() ? nullptr : iter->second);
	
	return nc;
}


Node* Node::findClosestNodeType(NodeType type, NodeDataType allowed_types, int &distance, NCType io1, NCType io2, int depth, int max_depth)
{
	//std::cout << "SEARCHING... " << depth << "\n";
	if(max_depth >= 0)
	{
		if(depth >= max_depth)
		{
			if(getType() == type)
			{
				distance = depth;
				return this;
			}
			else
				return nullptr;
		}
		else if(distance >= 0 && depth > distance)
			return nullptr;
	}
	else if(getType() == type)
	{
		distance = depth;
		return this;
	}

	std::vector<NodeConnector*> con = getConnectors();

	//Loop through connectors
	for(auto nc : con)
	{
		if((nc->ioType == io1 || nc->ioType == io2) && valid(nc->dataType & allowed_types))
		{
			std::unordered_map<NCID, NCOwnedPtr> a_conn = nc->getConnections();

			//Loop through connector's connections
			for(auto nconn : a_conn)
			{
				NodeConnector *nc = NodeConnector::getNodeConnector(nconn.first);

				//Recurse
				Node *n = nc->getNode()->findClosestNodeType(type, allowed_types, distance, io1, io2, depth + 1, max_depth);

				if(n)	//Found a node
					return n;
			}
		}
	}

	return nullptr;
}

Node* Node::getClosestNode(NodeType type, NodeDataType allowed_data_types, int &distance, bool backward, int max_depth)
{
	distance = -1;
	
	NCType	io1 = backward ? NCType::DATA_INPUT : NCType::DATA_OUTPUT,
			io2 = backward ? NCType::INFO_INPUT : NCType::INFO_OUTPUT;
	
	Node *n = findClosestNodeType(type, allowed_data_types, distance, io1, io2, 0, max_depth);

	return n;
}


bool Node::hasCursor() const
{
	return false;
}

bool Node::isPlaying()
{
	return hasCursor();
}

AStatus Node::propogateFlush(FlushPacket &info)
{
	AStatus status;

	//Flush all connectors (only inputs will actually flush)
	for(auto c : getConnectors())
	{
		status = c->flushData(info);
		if(!statusGood(status))
			break;
	}

	return status;
}

AStatus Node::flush(FlushPacket &info)
{
	return propogateFlush(info);
}

bool Node::isDirty() const
{
	return dirty;
}

AStatus Node::setDirty(ChunkRange r)
{
	AStatus status;

	dirty = true;
	
	//Set all connectors dirty (only outputs will be set)
	for(auto c : getConnectors())
	{
		status = c->setDirty(r);
		if(!statusGood(status))
			break;
	}

	return status;
}

AStatus Node::clean(ChunkRange r)
{
	return AS::A_SUCCESS;
}

void Node::update(double dt)
{
	std::vector<NodeConnector*> con = getConnectors();

	for(auto c : con)
		c->update(dt);
	
	onUpdate(dt);
}

void Node::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new NDesc()));
	NDesc *desc = dynamic_cast<NDesc*>(objDesc);
	
	desc->id = id;
	desc->type = getType();

	desc->nameStr = nodeName;
	desc->typeStr = nodeType;
	desc->descStr = nodeDesc;

	desc->dirty = dirty;

	desc->connectors.clear();
	desc->connectors.reserve(connectorList.size());

	for(unsigned int i = 0; i < connectorList.size(); i++)
		desc->connectors.push_back(connectorList[i]->getId());
}

*/