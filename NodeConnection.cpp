#include "NodeConnection.h"

#include "Node.h"

#include <iostream>


/////NODE CONNECTION/////
const double NodeConnection::ACTIVE_RESET_TIME = 0.15f;

NodeConnection::NodeConnection(NCID from_id, NCID to_id)
	: fromId(from_id), toId(to_id)
{
	fromNc = NodeConnector::getNC(fromId);
	toNc = NodeConnector::getNC(toId);

	fromNode = fromNc ? fromNc->getNode() : nullptr;
	toNode = toNc ? toNc->getNode() : nullptr;
}

//NodeConnection::NodeConnection(NodeConnector *from_nc, NodeConnector *to_nc)
//	: fromNc(from_nc), toNc(to_nc)
//{
//
//}

NodeConnection::~NodeConnection()
{

}

bool NodeConnection::isConnected() const
{
	return (fromId >= 0 && toId >= 0);
}

NCDir NodeConnection::getActiveDir() const
{
	return activeDir;
}

void NodeConnection::setActive(NCDir direction)
{
	activeDir |= direction;
}

void NodeConnection::update(double dt)
{
	if(valid(activeDir & NCDir::FORWARD) && resetForwardActive.tick(dt))
	{
		activeDir &= ~NCDir::FORWARD;
		resetForwardActive.reset(ACTIVE_RESET_TIME);
	}
	if(valid(activeDir & NCDir::BACKWARD) && resetBackwardActive.tick(dt))
	{
		activeDir &= ~NCDir::BACKWARD;
		resetBackwardActive.reset(ACTIVE_RESET_TIME);
	}
}




/////NODE CONNECTOR/////
NCMap		NodeConnector::registeredConnectors;
NCID		NodeConnector::nextId = 0;

NodeConnector::NodeConnector(Node *n, NodeDataType data_type, IOType io_type, const std::string &nc_name, const std::string &nc_desc, int max_connections)
	: node(n), dataType(data_type), ioType(io_type),
		name(nc_name), description(nc_desc), maxConnections(max_connections),
		dirty(false)
{
	registerId();
}

NodeConnector::NodeConnector(const NodeConnector &other)
	: node(other.node), dataType(other.dataType), ioType(other.ioType),
		name(other.name), description(other.description), maxConnections(other.maxConnections),
		dirty(other.dirty)
{
	registerId();
}

NodeConnector::~NodeConnector()
{
	deregisterId();
}

bool NodeConnector::registerId()
{
	id = nextId;
	nextId++;
	registeredConnectors.emplace(id, this);

	return true;
}

bool NodeConnector::deregisterId()
{
	auto iter = registeredConnectors.find(id);

	if(iter != registeredConnectors.end())
	{
		registeredConnectors.erase(id);
		id = 0;
		return true;
	}
	else
	{
		std::cout << "ERROR: Node Connector (id = " << id << ") failed to be deregistered.\n";
		return false;
	}
}

Node* NodeConnector::getNode()
{
	return node;
}

NodeConnector* NodeConnector::getNC(NCID nc_id)
{
	return registeredConnectors[nc_id];
}

NCID NodeConnector::getId() const
{
	return id;
}

std::string NodeConnector::getName() const
{
	return name;
}

std::string NodeConnector::getDesc() const
{
	return description;
}

void NodeConnector::setName(const std::string &new_name)
{
	name = new_name;
}

void NodeConnector::setDesc(const std::string &new_desc)
{
	description = new_desc;
}

NodeDataType NodeConnector::getDataType() const
{
	return dataType;
}

int NodeConnector::numConnections() const
{
	return connections.size();
}

CMap NodeConnector::getConnections()
{
	return connections;
}

bool NodeConnector::isConnected(NCID other_id) const
{
	return (connections.find(other_id) != connections.end());
}

bool NodeConnector::connect(NCID other_id)
{
	if(!isConnected(other_id))
	{
		//TODO: determine which id is from and to
		NodeConnection *con = new NodeConnection(id, other_id);

		connections.emplace(other_id, con);
		getNC(other_id)->connections.emplace(id, con);

		return true;
	}
	else
	{
		std::cout << "ERROR: Tried to connect NodeConnectors that were already connected!\n";
		return false;
	}
}

bool NodeConnector::disconnect(NCID other_id)
{
	if(isConnected(other_id))
	{
		connections.erase(other_id);
		getNC(other_id)->connections.erase(id);

		return true;
	}
	else
	{
		std::cout << "ERROR: Tried to disconnect NodeConnectors that weren't connected!\n";
		return false;
	}
}

bool NodeConnector::disconnectHalf(NCID other_id)
{
	if(isConnected(other_id))
	{
		NodeConnection *con = connections[other_id];

		if(con->isConnected())
		{
			if(con->fromId == id)	//from
			{
				con->fromId = -1;
				con->fromNc = nullptr;
				con->fromNode = nullptr;
			}
			else	//to
			{
				con->toId = -1;
				con->toNc = nullptr;
				con->toNode = nullptr;
			}
		}
		else
			connections.erase(other_id);
		
		return true;
	}
	else
	{
		return false;
	}
}

void NodeConnector::disconnectAll()
{
	for(auto c : connections)
	{
		getNC(c.first)->connections.erase(id);
		delete c.second;
	}

	connections.clear();
}


bool NodeConnector::flushData(FlushPacket &info)
{
	bool flushed = false;

	if(isInput(ioType))	//Only input nodes can flush
	{
		node->nodeLock.lockWait();
		//Flush each connection
		for(auto c : connections)
			flushed |= getNC(c.first)->node->flushData(info);
		node->nodeLock.unlock();
	}

	return flushed;
}

bool NodeConnector::pullData(PullPacket &output)//, NCID other_id)
{
	bool pulled = false;

	if(isInput(ioType))	//Only input nodes can pull
	{
		//if(other_id < 0)
		//{

		node->nodeLock.lockWait();
		//Pull data from all connections
		for(auto c : connections)
		{
			if(getNC(c.first)->node->pullData(output, c.first))
			{
				pulled = true;
				c.second->setActive(NCDir::BACKWARD);
			}
		}
		node->nodeLock.unlock();

		//}
		//else
		//{
		//	//Pull data from specified connection
		//	pulled = getNC(other_id)->node->pullData(output, other_id);
		//}
	}

	return pulled;
}

bool NodeConnector::pushData(PushPacket &input)
{
	bool pushed = false;

	if(isOutput(ioType))	//Only output nodes can push
	{
		node->nodeLock.lockWait();
		//Push data to each connection
		for(auto c : connections)
		{
			if(getNC(c.first)->node->pushData(input, c.first))
			{
				pushed = true;
				c.second->setActive(NCDir::FORWARD);
			}
		}
		node->nodeLock.unlock();
	}

	return pushed;
}


void NodeConnector::update(double dt)
{
	node->nodeLock.lockWait();

	for(auto c : connections)
		c.second->update(dt);

	node->nodeLock.unlock();
}


/*
/////NODE CONNECTION/////

NodeConnection::NodeConnection(NCID from_id, NCID to_id)
	: fromId(from_id), toId(to_id),
		fromNc(from_id > 0 ? NodeConnector::getNodeConnector(from_id) : nullptr),
		toNc(to_id > 0 ? NodeConnector::getNodeConnector(to_id) : nullptr)
{
	fromNode = fromNc ? fromNc->getNode() : nullptr;
	toNode = toNc ? toNc->getNode() : nullptr;
}

bool NodeConnection::isConnected() const
{
	return (fromNode && fromNc && toNode && toNc);
}







/////NODE CONNECTOR/////
NCList NodeConnector::regIds;
NCID NodeConnector::nextId = 1;	//Starts at 1; 0 is always invalid.

const double NodeConnector::ACTIVE_RESET_TIME = 0.15f;

NodeConnector::NodeConnector()
	: NodeConnector(nullptr, NodeConnectorDesc())
{ }

NodeConnector::NodeConnector(Node *n, NodeConnectorDesc &c_desc)
	: dataType(c_desc.dataType), ioType(c_desc.ioType), name(c_desc.name), desc(c_desc.desc),
		node(n), maxConnections(c_desc.maxConnections)
{
	if(node)
	{
		//Register
		if(statusSucceeded(registerId()) && maxConnections > 0)
			connections.reserve(maxConnections);
	}
	else
		std::cout << "ERROR: NodeConnector initialized with null node -- weird things may happen!\n";
}

NodeConnector::NodeConnector(const NCDesc &nc_desc)
	: dataType(nc_desc.dataType), ioType(nc_desc.ioType), name(nc_desc.name), desc(nc_desc.desc),
		node(nullptr), maxConnections(nc_desc.maxConnections),
		id(nc_desc.id)
{
	//Add id to static list, and update nextId
	regIds.emplace(id, this);
	nextId = max(id + 1, nextId);
	
	nId = nc_desc.nodeId;
	cIds.reserve(nc_desc.connections.size());
	cIds.insert(cIds.end(), nc_desc.connections.begin(), nc_desc.connections.end());
}

NodeConnector::~NodeConnector()
{
	if(validConnector())
		deregisterId();
}

NodeConnector* NodeConnector::getNodeConnector(NCID nc_id)
{
	return regIds[nc_id];
}

AStatus NodeConnector::registerId()
{
	AStatus status;

	//Make sure connector is valid
	//TODO: Make sure this is all that needs to be checked
	if(dataType != NodeData::INVALID && maxConnections != 0)
	{
		id = nextId;
		regIds.emplace(id, this);
		nextId++;
	}
	else
		status.setError(AS::ErrorType::GENERAL, "Attempted to register an invalid connector.");

	return status;
}


AStatus NodeConnector::deregisterId()
{
	AStatus status;
	NCList::const_iterator id_iter = regIds.find(id);

	if(id_iter == regIds.end())
		status.setWarning(AS::WType::NOT_REGISTERED, "NodeConnector ID is not registered.");
	else
		regIds.erase(id_iter);

	id = 0;

	return status;
}

//
//void NodeConnector::getPointers()
//{
//	node = Node::getNode(nId);
//
//	//Update connections
//	connections.clear();
//	connections.resize(cIds.size());
//	connections.reserve(maxConnections);
//
//	for(unsigned int i = 0; i < connections.size(); i++)
//		connections[i] = NodeConnector::getNodeConnector(cIds[i]);
//}


NCID NodeConnector::getId()
{
	return id;
}

AStatus NodeConnector::connectorsCompatible(const NodeConnector *nc1, const NodeConnector *nc2)
{
	AStatus status;
	std::stringstream ss(status.msg);

	if(!nc1 || !nc2)
	{
		status.setError(AS::ErrorType::GENERAL, "One of the given connectors is NULL.");
	}
	else if(!nc1->validConnector())
	{
		status = AS::A_ERROR;
		ss << "NodeConnector " << nc1->name << " is invalid.";
	}
	else if(!nc2->validConnector())
	{
		status = AS::A_ERROR;
		ss << "NodeConnector " << nc2->name << " is invalid.";
	}
	else if(nc1->dataType != nc2->dataType)
	{
		status = AS::A_ERROR;
		ss << "Data type " << nc1->dataType << " incompatible with " << nc2->dataType << ".";
	}
	else if(nc1->numOpenConnections() == 0)
	{
		status = AS::A_ERROR;
		ss << "NodeConnector " << nc1->name << " has no open connections.";
	}
	else if(nc2->numOpenConnections() == 0)
	{
		status = AS::A_ERROR;
		ss << "NodeConnector " << nc2->name << " has no open connections.";
	}
	else if(nc1->ioType == nc2->ioType)
	{
		status = AS::A_ERROR;
		ss << "Both NodeConnectors same I/O-type (" << nc1->name << ": " << toIndex(nc1->ioType) << ", " << nc2->name << ": " << toIndex(nc2->ioType) << ").";
	}
	else if(nc1->node == nc2->node)
	{
		status = AS::A_ERROR;
		ss << "Both NodeConnectors belog to the same Node (" << nc1->node->getName() << ").";
	}
	//else no error

	return status;
}

bool NodeConnector::validConnector() const
{
	return (id != 0);
}

AStatus NodeConnector::connect(NCID nc_id)
{
	NodeConnector *nc = getNodeConnector(nc_id);
	AStatus status = connectorsCompatible(this, nc);

	if(statusGood(status))
	{
		NCID	output_id = isOutput(nc->ioType) ? nc_id : id,
				input_id = isInput(nc->ioType) ? nc_id : id;

		NCOwnedPtr	ncon(new NodeConnection(output_id, input_id));
		
		connectorLock.lockWait();
		connections.emplace(nc_id, ncon);
		connectorLock.unlock();

		nc->connectorLock.lockWait();
		nc->connections.emplace(id, ncon);
		nc->connectorLock.unlock();
		
		node->parentGraph->addConnection(ncon);
		//nc->getNode()->parentGraph->addConnection(ncon);

		node->onConnect(this, nc);
		nc->getNode()->onConnect(nc, this);
	}

	return status;
}

AStatus NodeConnector::connect(NodeConnector *nc)
{
	return connect(nc ? nc->getId() : 0);
}


NCOwnedPtr NodeConnector::startConnect()
{
	bool output = (ioType == NCType::DATA_OUTPUT || ioType == NCType::INFO_OUTPUT);
	NCOwnedPtr ncon(new NodeConnection(output ? id : 0, output ? 0 : id));
	
	connectorLock.lockWait();
	connections.emplace(0, ncon);
	connectorLock.unlock();

	node->parentGraph->addConnection(ncon);

	return ncon;
}

void NodeConnector::finishConnect(NCID other_id)
{
	NCOwnedPtr ncp = connections[0];

	if(ncp->fromNode)
	{
		ncp->toId = other_id;
		ncp->toNc = getNodeConnector(other_id);
		ncp->toNode = ncp->toNc->getNode();
	}
	else
	{
		ncp->fromId = other_id;
		ncp->fromNc = getNodeConnector(other_id);
		ncp->fromNode = ncp->fromNc->getNode();
	}

	connections.erase(0);
	connections.emplace(other_id, ncp);
}


AStatus NodeConnector::disconnect(NCID nc_id)
{
	AStatus status;
	
	NodeConnector *nc = getNodeConnector(nc_id);

	node->parentGraph->removeConnection(connections[nc_id]);
	//nc->getNode()->parentGraph->removeConnection(nc->connections[id]);

	connectorLock.lockWait();
	//connections[nc_id].reset();
	connections.erase(nc_id);
	connectorLock.unlock();

	nc->connectorLock.lockWait();
	//nc->connections[id].reset();
	nc->connections.erase(id);
	nc->connectorLock.unlock();
	
	node->onDisconnect(this, nc);
	nc->getNode()->onDisconnect(nc, this);

	return status;
}

AStatus NodeConnector::disconnect(NodeConnector *nc)
{
	return disconnect(nc->getId());
}

//
//AStatus NodeConnector::disconnect(CIndex index)
//{
//	NodeConnector *nc = getConnection(index);
//	return (nc ? disconnect(nc) : AStatus(AS::A_ERROR, "Failed to disconnect (index out of bounds)."));
//}


void NodeConnector::disconnectAll()
{
	connectorLock.lockWait();

	//Disconnect nodes
	for(auto c : connections)
	{
		NodeConnector *nc = getNodeConnector(c.first);
		node->parentGraph->removeConnection(c.second);
		//nc->getNode()->parentGraph->removeConnection(nc->connections[id]);

		//c.second.reset();

		nc->connectorLock.lockWait();
		//nc->connections[id].reset();
		nc->connections.erase(id);
		nc->connectorLock.unlock();
		
		node->onDisconnect(this, nc);
		nc->getNode()->onDisconnect(nc, this);

	}
	connections.clear();

	connectorLock.unlock();

}

Node* NodeConnector::getNode()
{
	return node;
}

//
//Node* NodeConnector::getConnectedNode(CIndex index)
//{
//	NodeConnector *c_nc = getConnection(index);
//	return (c_nc ? c_nc->getNode() : nullptr);
//}


NCOwnedPtr NodeConnector::getConnection(NCID nc_id)
{
	connectorLock.lockWait();

	NCOwnedPtr a_ptr = connections[nc_id];

	connectorLock.unlock();

	return a_ptr;
}

std::unordered_map<NCID, NCOwnedPtr> NodeConnector::getConnections()
{
	std::unordered_map<NCID, NCOwnedPtr> ptrs;

	connectorLock.lockWait();
	
	ptrs.reserve(connections.size());
	for(auto c : connections)
		ptrs.emplace(c.first, c.second);

	connectorLock.unlock();

	return ptrs;
}

//
//NodeConnector::CIndex NodeConnector::getIndex(NodeConnector *nc)
//{
//	connectorLock.lockWait();
//
//	CIndex index = std::find(connections.begin(), connections.end(), nc) - connections.begin();
//	CIndex ind = (index < connections.size() ? index : -1);
//	
//	connectorLock.unlock();
//
//	return ind;
//}
//
//
//NodeConnector::CIndex NodeConnector::getIndex(NCID nc_id)
//{
//	auto found = regIds.find(nc_id);
//	return (found == regIds.end()) ? -1 : getIndex(found->second);
//}

unsigned int NodeConnector::numConnections() const
{
	return connections.size();
}
int NodeConnector::numOpenConnections() const
{
	return (maxConnections < 0) ? (-1) : (maxConnections - numConnections());
}
bool NodeConnector::hasConnections() const
{
	return numConnections() > 0;
}

bool NodeConnector::isActive()
{
	return valid(activeDir);
}

NDir NodeConnector::activeDirection()
{
	return activeDir;
}

void NodeConnector::setActive(NDir active_dir, NCID nc_id)
{
	activeDir |= active_dir;

	NodeConnector *nc = getNodeConnector(nc_id);
	if(nc)
		nc->activeDir |= active_dir;
}

bool NodeConnector::canPull(NCID nc_id)
{
	NodeConnector *nc = getNodeConnector(nc_id);
	return (nc && isInput(ioType) && nc->getNode()->canPull());
}

bool NodeConnector::canPush(NCID nc_id)
{
	NodeConnector *nc = getNodeConnector(nc_id);
	return (nc && isOutput(ioType) && nc->getNode()->canPush());
}

bool NodeConnector::checkType(NodeData type) const
{
	return (type & dataType) == dataType;
}

void NodeConnector::update(double dt)
{
	connectorLock.lockWait();

	if(valid(activeDir & NDir::FORWARD) && resetForwardActive.tick(dt))
	{
		activeDir &= ~NDir::FORWARD;
		resetForwardActive.reset(ACTIVE_RESET_TIME);
	}
	if(valid(activeDir & NDir::BACKWARD) && resetBackwardActive.tick(dt))
	{
		activeDir &= ~NDir::BACKWARD;
		resetBackwardActive.reset(ACTIVE_RESET_TIME);
	}
	
	connectorLock.unlock();
}

bool NodeConnector::hasTimeline()
{
	bool has_timeline = node->hasCursor();
	if(has_timeline) return true;

	connectorLock.lockWait();
	for(auto nconn : connections)
	{
		NodeConnector *nc = getNodeConnector(nconn.first);
		if(has_timeline |= (nc->ioType == NCType::DATA_OUTPUT && nc->hasTimeline()))
			break;
	}
	connectorLock.unlock();

	return has_timeline;
}

bool NodeConnector::timelineIsPlaying()
{
	bool playing = node->isPlaying();
	if(playing) return true;
	
	connectorLock.lockWait();
	for(auto nconn : connections)
	{
		NodeConnector *nc = getNodeConnector(nconn.first);
		if(playing |= (nc->ioType == NCType::DATA_OUTPUT && nc->timelineIsPlaying()))
			break;
	}
	connectorLock.unlock();

	return playing;
}

bool NodeConnector::isDirty() const
{
	return dirty;
}

AStatus NodeConnector::setDirty(ChunkRange r)
{
	AStatus status;

	if(isOutput(ioType))
	{
		
		if(Keyboard::keyDown(Keys::K_1))
		{
			std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " setting dirty --> (" << r.start << ", " << r.end << ")\n";
		}

		dirty = true;
		
		//Propogate dirty range
		connectorLock.lockWait();
		for(auto nconn : connections)
		{
			NodeConnector *nc = getNodeConnector(nconn.first);
			status = nc->getNode()->setDirty(r);
			if(!statusGood(status))
				break;
		}
		connectorLock.unlock();
	}
	//else
	//	status.setWarning("Only outputs can be made dirty.");

	return status;
}

AStatus NodeConnector::flushData(FlushPacket &info)
{
	AStatus status;
	
	//Only call flush on input nodes (flush request should only propogate backwards)
	if(isInput(ioType))
	{
		//
		//std::vector<NodeConnector*> flush_connections;
		//flush_connections.reserve(numConnections());

		////Copy connectors to temp vector
		//connectorLock.lockWait();
		//flush_connections.insert(flush_connections.end(), connections.begin(), connections.end());
		//connectorLock.unlock();

		////Flush each node
		//for(auto nc : flush_connections)
		//{
		//	//Flush connected node (which should propogate the flush if necessary)
		//	if(nc->getNode())
		//		status = nc->getNode()->flush(curr_t);
		//	if(!statusGood(status))
		//		break;
		//}
		

		connectorLock.lockWait();
		for(auto nconn : connections)
		{
			NodeConnector *nc = getNodeConnector(nconn.first);
			Node *n = nc->getNode();
			if(n && n->canFlush())
			{
				if(Keyboard::keyDown(Keys::K_1))
					std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " flushing --> (" << info.flushTimeRange.start << ", " << info.flushTimeRange.end << ") : (" << info.flushChunkRange.start << ", " << info.flushChunkRange.end << ")\n";

				status = nc->getNode()->flush(info);
				if(!statusGood(status))
					break;
			}
		}
		connectorLock.unlock();
	}
	else
		status.setWarning(AS::WType::GENERAL, "Only input nodes can be flushed.");

	return status;
}

AStatus NodeConnector::pullData(PullPacket &output, NCID nc_id)
{
	AStatus status;

	if(valid(output.getType() & dataType))
	{
		NodeConnector *nc = getNodeConnector(nc_id);
		Node *n;

		if(nc && (n = nc->getNode()) && n->canPull())
		{
			if(Keyboard::keyDown(Keys::K_1))
			{
				MidiPullPacket *mpp = dynamic_cast<MidiPullPacket*>(&output);
				AudioPullPacket *app = dynamic_cast<AudioPullPacket*>(&output);
				
				if(mpp)
					std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pulling --> (" << mpp->range.start << ", " << mpp->range.end << ")\n";
				else if(app)
					std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pulling --> (" << app->range.start << ", " << app->range.end << ")\n";
				else
					std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pulling -->  " << "UNKNOWN\n";
			}

			setActive(NDir::BACKWARD, nc_id);
			status = n->pullData(output, nc->getId());
		}
		else
			status.setWarning(AS::WType::NO_ACTION_TAKEN, "No data to pull.");
	}
	else
		status.setError(AS::ErrorType::GENERAL, "Incorrect packet type received");

	return status;
}

AStatus NodeConnector::pushData(PushPacket &input)
{
	AStatus status;
	
	if(valid(input.getType() & dataType))
	{
		
		//std::vector<NodeConnector*> push_connections;
		//push_connections.reserve(numConnections());

		//connectorLock.lockWait();
		//for(CIndex index = 0; index < (CIndex)numConnections(); index++)
		//{
		//	NodeConnector *nc = getConnection(index);
		//	Node *n;

		//	if(nc && (n = nc->getNode()) && n->canPush())
		//	{
		//		setActive(NDir::FORWARD, index);
		//		push_connections.push_back(nc);
		//	}
		//}
		//connectorLock.unlock();

		//for(auto nc : push_connections)
		//{
		//	status = nc->getNode()->pushData(input, nc->getId(), getId());
		//	if(!statusGood(status))
		//		break;
		//}
		
		connectorLock.lockWait();
		for(auto nconn : connections)
		{
			NodeConnector *nc = getNodeConnector(nconn.first);
			Node *n;
			if(nc && (n = nc->getNode()) && n->canPush())
			{
				
				if(Keyboard::keyDown(Keys::K_1))
				{
						MidiPushPacket *mpp = dynamic_cast<MidiPushPacket*>(&input);
						AudioPushPacket *app = dynamic_cast<AudioPushPacket*>(&input);
				
						if(mpp)
							std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pushing --> (" << mpp->range.start << ", " << mpp->range.end << ")\n";
						else if(app)
							std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pushing --> (" << app->range.start << ", " << app->range.end << ")\n";
						else
							std::cout << std::setw(20) << getNode()->getName() << std::setw(25) << " pushing -->  " << "UNKNOWN\n";
				}

				setActive(NDir::FORWARD, nconn.first);
				status = n->pushData(input, nc->getId(), getId());
				if(!statusGood(status))
					break;
			}
		}
		connectorLock.unlock();
	}
	else
		status.setError(AS::ErrorType::GENERAL, "Incorrect packet type received.");

	return status;
}

void NodeConnector::updateDesc()
{
	objDesc = (objDesc ? objDesc : (ObjDesc*)(new NCDesc()));
	NCDesc *nc_desc = dynamic_cast<NCDesc*>(objDesc);
	
	nc_desc->id = id;
	nc_desc->nodeId = node->getId();

	nc_desc->dataType = dataType;
	nc_desc->ioType = ioType;

	nc_desc->maxConnections = maxConnections;

	nc_desc->name = name;
	nc_desc->desc = desc;


	nc_desc->connections.clear();
	nc_desc->connections.reserve(connections.size());

	for(auto nconn : connections)
		nc_desc->connections.push_back(nconn.second);
}

*/