#include "NodeConnection.h"

#include "Node.h"
#include "NodePackets.h"
#include "NodeGraph.h"

#include <iostream>


/////NODE CONNECTION/////
const double NodeConnection::ACTIVE_RESET_TIME = 0.15f;

Registry<NodeConnection> NodeConnection::reg;

//std::unordered_set<std::shared_ptr<NodeConnection>> NodeConnection::allNCs;

NodeConnection::NodeConnection(NCID id1, NCID id2)
	: id(reg.registerId(this))
{
	NodeConnector	*nc1 = NodeConnector::getNC(id1),
					*nc2 = NodeConnector::getNC(id2);

	if(nc1 && nc2 && NodeConnector::validConnection(id1, id2))
	{
		fromId = isOutput(nc1->ioType) ? id1 : (isOutput(nc2->ioType) ? id2 : -1);
		toId = isInput(nc1->ioType) ? id1 : (isInput(nc2->ioType) ? id2 : -1);

		fromNc = NodeConnector::getNC(fromId);
		toNc = NodeConnector::getNC(toId);

		//Connect given nodes
		if(fromNc)
		{
			fromNc->onConnect(toId, this);
			fromNode = fromNc->getNode();
		}
		if(toNc)
		{
			toNc->onConnect(fromId, this);
			toNode = toNc->getNode();
		}
	}
	//else invalid connection
}

NodeConnection::NodeConnection(NCID id)
	: id(reg.registerId(this))
{
	NodeConnector *nc = NodeConnector::getNC(id);

	if(nc)
	{
		fromId = isOutput(nc->ioType) ? id : -1;
		toId = isInput(nc->ioType) ? id : -1;
		
		fromNc = NodeConnector::getNC(fromId);
		toNc = NodeConnector::getNC(toId);

		fromNode = fromNc ? fromNc->getNode() : nullptr;
		toNode = toNc ? toNc->getNode() : nullptr;
	}
	//Else invalid id
}

/*
NodeConnection::NodeConnection(NodeConnector *from_nc, NodeConnector *to_nc)
	: fromNc(from_nc), toNc(to_nc)
{

}
*/

NodeConnection::~NodeConnection()
{
	//destroy();
	//allNCs.erase(this);

	if(fromIsConnected() && toIsConnected())
	{
		fromNc->onDisconnect(toId);
		toNc->onDisconnect(fromId);
	}

	reg.deregisterId(this);
}

//void NodeConnection::resetAllConnectionStates()
//{
//	for(auto nc : allNCs)
//		nc->resetConnectionStates();
//}

CID NodeConnection::getId() const
{
	return id;
}


bool NodeConnection::connectTo(NCID other_id)
{
	bool status = true;

	NodeConnector *other_nc = NodeConnector::getNC(other_id);

	if(fromIsConnected() && !toIsConnected() && other_nc && NodeConnector::validConnection(fromId, other_id))
	{
		//to --> other
		toId = other_id;
		toNc = other_nc;
		toNode = other_nc->getNode();

		fromNc->onConnect(toId, this);
		toNc->onConnect(fromId, this);
	}
	else if(toIsConnected() && !fromIsConnected() && other_nc && NodeConnector::validConnection(toId, other_id))
	{
		//from --> other
		fromId = other_id;
		fromNc = other_nc;
		fromNode = other_nc->getNode();
		
		fromNc->onConnect(toId, this);
		toNc->onConnect(fromId, this);
	}
	else
		status = false;

	return status;
}

bool NodeConnection::disconnectFrom(NCID nc_id)
{
	bool status = true;

	if(fromId == nc_id && toIsConnected())
	{
		fromNc->onDisconnect(toId);
		toNc->onDisconnect(fromId);

		fromId = -1;
		fromNc = nullptr;
		fromNode = nullptr;
	}
	else if(toId == nc_id && fromIsConnected())
	{
		fromNc->onDisconnect(toId);
		toNc->onDisconnect(fromId);
		
		toId = -1;
		toNc = nullptr;
		toNode = nullptr;
	}
	else
		status = false;

	return status;
}

NCID NodeConnection::getOppositeNc(NCID this_nc) const
{
	return this_nc == fromId ? toId : (this_nc == toId ? fromId : -1);
}

/*
bool NodeConnection::connectTo(NCID other_id)
{
	if((fromIsConnected() && toIsConnected()) || !NodeConnector::validConnection((fromIsConnected() ? fromId : toId), other_id))
		return false;

	bool status = true;


	if(fromIsConnected())
	{
		//Remove hanging connection
		fromNode->nodeLock.lockWait();
		fromNc->connections.erase(-1);
		fromNode->nodeLock.unlock();

		//to side needs to be connected to
		toId = other_id;
		toNc = NodeConnector::getNC(toId);
		toNode = toNc ? toNc->getNode() : nullptr;
	}
	else
	{
		//Remove hanging connection
		toNode->nodeLock.lockWait();
		toNc->connections.erase(-1);
		toNode->nodeLock.unlock();

		//from side needs to be connected to
		fromId = other_id;
		fromNc = NodeConnector::getNC(fromId);
		fromNode = fromNc ? fromNc->getNode() : nullptr;
	}

	//Update nodes
	//fromNc->onConnect(toId, this);
	//toNc->onConnect(fromId, this);

	return status;
}
*/
/*
bool NodeConnection::destroy()
{
	bool status = true;

	if(fromIsConnected())
	{
		Node *old_fromNode = fromNode;

		fromNode->nodeLock.lockWait();
		//fromNode->parentGraph->removeConnection(this);
		fromNc->onDisconnect(toId);

		fromId = -1;
		fromNc = nullptr;
		fromNode = nullptr;
		old_fromNode->nodeLock.unlock();
	}
	else if(toIsConnected())
	{
		toNode->nodeLock.lockWait();
		toNc->connections.erase(-1);
		toNode->nodeLock.unlock();
	}

	if(toIsConnected())
	{
		Node *old_toNode = toNode;

		toNode->nodeLock.lockWait();
		//toNode->parentGraph->removeConnection(this);
		toNc->onDisconnect(fromId);

		toId = -1;
		toNc = nullptr;
		toNode = nullptr;
		old_toNode->nodeLock.unlock();
	}
	else if(fromIsConnected())
	{
		fromNode->nodeLock.lockWait();
		fromNc->connections.erase(-1);
		fromNode->nodeLock.unlock();
	}

	return true;
}
*/
/*
bool NodeConnection::disconnectHalf(bool disconnect_from)
{
	//Only one hanging connection at a time
	if((toNc->connections.find(-1) != toNc->connections.end())
		|| (fromNc->connections.find(-1) != fromNc->connections.end()))
		return false;

	bool status = true;

	//Disconnect nodes
	fromNc->onDisconnect(toId);
	toNc->onDisconnect(fromId);

	//Keep one half still hanging on
	if(disconnect_from)
	{
		fromNode->nodeLock.lockWait();
		Node *old_fromNode = fromNode;

		fromId = -1;
		fromNc = nullptr;
		fromNode = nullptr;

		old_fromNode->nodeLock.unlock();

		//Add hanging connection
		toNode->nodeLock.lockWait();
		toNc->connections.emplace(-1, this);
		toNode->nodeLock.unlock();
	}
	else
	{
		toNode->nodeLock.lockWait();
		Node *old_toNode = toNode;

		toId = -1;
		toNc = nullptr;
		toNode = nullptr;
		
		old_toNode->nodeLock.unlock();
		
		//Add hanging connection
		fromNode->nodeLock.lockWait();
		fromNc->connections.emplace(-1, this);
		fromNode->nodeLock.unlock();
	}
	

	return status;
}
*/
/*
bool NodeConnection::disconnectHalf(NCID nc_id)
{
	return (nc_id == fromId || nc_id == toId) ? disconnectHalf(nc_id == fromId) : false;
}
*/
bool NodeConnection::isConnected() const
{
	return (toIsConnected() && fromIsConnected());
}

bool NodeConnection::toIsConnected() const
{
	return (toId >= 0);
}

bool NodeConnection::fromIsConnected() const
{
	return (fromId >= 0);
}

bool NodeConnection::isHanging() const
{
	return !toIsConnected() || !fromIsConnected();
}

NCDir NodeConnection::getActiveDir() const
{
	return activeDir | lastActiveDir;
}

void NodeConnection::setActive(NCDir direction)
{
	activeDir |= direction;
}

void NodeConnection::setPushing(bool is_pushing)
{
	pushing = is_pushing;
}

void NodeConnection::setPulling(bool is_pulling)
{
	pulling = is_pulling;
}

bool NodeConnection::isPushing() const
{
	return pushing || lastPushing;
}

bool NodeConnection::isPulling() const
{
	return pulling || lastPulling;
}

void NodeConnection::update(const Time &dt)
{
	/*
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
	*/
}

void NodeConnection::resetConnectionStates()
{
	lastActiveDir = activeDir;
	lastPushing = pushing;
	lastPulling = pulling;

	activeDir = NCDir::NONE;
	pushing = false;
	pulling = false;
}



/////NODE CONNECTOR/////
Registry<NodeConnector>	NodeConnector::reg;

NodeConnector::NodeConnector(Node *n, NodeDataType data_type, IOType io_type, const std::string &nc_name, const std::string &nc_desc, int max_connections)
	: id(reg.registerId(this)), node(n), dataType(data_type), ioType(io_type),
		name(nc_name), description(nc_desc), maxConnections(max_connections),
		dirty(false)
{ }

NodeConnector::NodeConnector(const NodeConnector &other)
	: id(reg.registerId(this)), node(other.node), dataType(other.dataType), ioType(other.ioType),
		name(other.name), description(other.description), maxConnections(other.maxConnections),
		dirty(other.dirty)
{ }

NodeConnector::~NodeConnector()
{
	destroyAllConnections();

	reg.deregisterId(id);
	id = -1;
}

bool NodeConnector::validConnection(NCID nc_id1, NCID nc_id2)
{
	NodeConnector	*nc1 = getNC(nc_id1),
					*nc2 = getNC(nc_id2);

	return (nc1 && nc2												&&	//Check that they arent null
			!nc1->isConnected(nc_id2) && !nc2->isConnected(nc_id1)	&&	//Check that theyre not connected
			nc1->ioType == getOpposite(nc2->ioType));//					&&	//Check that one is output and one is input
			//nc1->getDataType() == nc2->getDataType());					//Check that they are the same data type
}

Node* NodeConnector::getNode()
{
	return node;
}

NodeConnector* NodeConnector::getNC(NCID nc_id)
{
	return reg.getObject(nc_id);
}

NCID NodeConnector::getId() const
{
	return id;
}

NodeDataType NodeConnector::getDataType() const
{
	return dataType;
}

int NodeConnector::numConnections() const
{
	return connections.size();
}
/*
CMap NodeConnector::getConnections()
{
	return connections;
}
*/

/*
bool NodeConnector::makeConnection(NCID other_id)
{
	bool status = true;

	if(validConnection(id, other_id))
	{
		bool from = (isOutput(ioType));
		NodeConnection *con = from ? new NodeConnection(id, other_id) : new NodeConnection(other_id, id);
	}
	else
		status = false;

	return status;
}

bool NodeConnector::destroyConnection(NCID other_id)
{
	return connections[other_id]->destroy();
}
*/

void NodeConnector::onConnect(NCID other_id, NodeConnection *c)
{
	node->nodeLock.lockWait();

	connections.emplace(other_id, c);
	node->onConnect(id, other_id);
	
	node->nodeLock.unlock();

	/*
	if(!isConnected(other_id))
	{
		node->nodeLock.lockWait();
		//TODO: determine which id is from and to
		NodeConnection *con = new NodeConnection(id, other_id);

		connections.emplace(other_id, con);
		getNC(other_id)->connections.emplace(id, con);

		node->onConnect(id, other_id);
		getNC(other_id)->node->onConnect(other_id, id);
		
		node->nodeLock.unlock();
		return true;
	}
	else
	{
		std::cout << "ERROR: Tried to connect NodeConnectors that were already connected!\n";
		return false;
	}
	*/
}

void NodeConnector::onDisconnect(NCID other_id)
{
	node->nodeLock.lockWait();
	connections.erase(other_id);
	node->nodeLock.unlock();

	node->onDisconnect(id, other_id);

	/*
	if(isConnected(other_id))
	{
		node->nodeLock.lockWait();
		NodeConnector *other_nc = getNC(other_id);
		NodeConnection *c = connections[other_id];

		connections.erase(other_id);
		other_nc->connections.erase(id);
		
		node->onDisconnect(id, other_id);
		other_nc->node->onDisconnect(other_id, id);

		//delete c;
		node->nodeLock.unlock();

		return true;
	}
	else
	{
		std::cout << "ERROR: Tried to disconnect NodeConnectors that weren't connected!\n";
		return false;
	}
	*/
}

void NodeConnector::destroyAllConnections()
{
	node->nodeLock.lockWait();
	
	std::vector<NodeConnection*> conn;
	conn.reserve(connections.size());
	/*
	for(auto c : connections)
	{
		conn.push_back(c.second);
		getNC(c.first)->connections.erase(id);
	}
	*/

	for(auto c : conn)
		delete c;	//Destroys itself in the desctructor

	node->nodeLock.unlock();
}

/*
void NodeConnector::disconnectAll()
{
	node->nodeLock.lockWait();
	for(auto c : connections)
	{
		getNC(c.first)->connections.erase(id);
		
		node->onDisconnect(id, c.first);
		getNC(c.first)->node->onDisconnect(c.first, id);
		
		delete c.second;
	}

	connections.clear();
	node->nodeLock.unlock();
}
*/

bool NodeConnector::isConnected(NCID other_id) const
{
	return (connections.find(other_id) != connections.end());
}


//bool NodeConnector::flushData(FlushPacket &info)
//{
//	bool flushed = false;
//
//	if(isInput(ioType))	//Only input nodes can flush
//	{
//		node->nodeLock.lockWait();
//		//Flush each connection
//		for(auto c : connections)
//			flushed |= getNC(c.first)->node->flushData(info);
//		node->nodeLock.unlock();
//	}
//
//	return flushed;
//}

bool NodeConnector::pullData(PullPacket &output)//, NCID other_id)
{
	bool pulled = false;

	if(isInput(ioType))	//Only input nodes can pull
	{
		node->nodeLock.lockWait();
		//Pull data from all connections
		for(auto c : connections)
		{
			NCID other_nc = c.second->getOppositeNc(id);
			NodeConnector *nc = getNC(other_nc);
			if(nc)
			{
				c.second->setPulling(true);
				if(nc->node->pullData(output, other_nc, id))
				{
					pulled = true;
					c.second->setActive(NCDir::BACKWARD);
				}
			}
		}
		node->nodeLock.unlock();
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
			NCID other_nc = c.second->getOppositeNc(id);
			NodeConnector *nc = getNC(other_nc);
			if(nc)
			{
				c.second->setPushing(true);
				if(nc->node->pushData(input, other_nc, id))
				{
					pushed = true;
					c.second->setActive(NCDir::FORWARD);
				}
			}
		}
		node->nodeLock.unlock();
	}

	return pushed;
}


void NodeConnector::update(const Time &dt)
{
	node->nodeLock.lockWait();

	for(auto c : connections)
		c.second->update(dt);

	node->nodeLock.unlock();
}

void NodeConnector::resetConnectionStates()
{
	//pushing = false;
	//pulling = false;
}