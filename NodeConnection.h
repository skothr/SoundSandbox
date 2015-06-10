#ifndef APOLLO_NODE_CONNECTION_H
#define APOLLO_NODE_CONNECTION_H

#include <unordered_map>
#include <memory>

#include "Timing.h"
#include "Registry.h"
#include "NodeResources.h"

#include <unordered_set>

class Node;

class NodeConnection
{
private:
	static Registry<NodeConnection> reg;

protected:
	CID							id			= -1;

	NCID						fromId		= -1,
								toId		= -1;

	NodeConnector				*fromNc		= nullptr,
								*toNc		= nullptr;
	
	Node						*fromNode	= nullptr,
								*toNode		= nullptr;

	NCDir						activeDir	= NCDir::NONE,
								lastActiveDir = NCDir::NONE;
	
	bool						pushing = false,		//Whether this connection is trying to pull/push.
								pulling = false,
								lastPushing = false,	//Used as new pushing/pulling values are being set (equal to last state)
								lastPulling = false;

public:
	//TODO: Only NodeGraph (friend class) can construct a connection.
	NodeConnection(NCID id1, NCID id2);		//Create connection between given nc
	NodeConnection(NCID id1);				//Create hanging connection off the given nc

	//NodeConnection(NodeConnector *from_nc, NodeConnector *to_nc);
	virtual ~NodeConnection();
	
	static const double	ACTIVE_RESET_TIME;

	//static void resetAllConnectionStates();

	CID getId() const;

	bool connectTo(NCID other_id);
	bool disconnectFrom(NCID nc_id);

	//Returns the opposite NodeConnector to the one that's given (if both exist)
	NCID getOppositeNc(NCID this_nc) const;
	
	//bool connectTo(NCID other_id);
	//bool disconnectHalf(bool disconnect_from);	//disconnect_from is whether to disconnect the from side (versus to side.)
	//bool disconnectHalf(NCID nc_id);

	//bool destroy();

	bool isConnected() const;
	bool toIsConnected() const;
	bool fromIsConnected() const;

	bool isHanging() const;

	NCDir getActiveDir() const;
	void setActive(NCDir direction);

	void setPushing(bool is_pushing);
	void setPulling(bool is_pulling);

	bool isPushing() const;
	bool isPulling() const;


	void update(const Time &dt);
	
	//Resets activeDir and pushing/pulling variables to clear before calculating again.
	void resetConnectionStates();
	
	//friend class NodeConnector;
	friend class NodeConnectionControl;
	friend class NodeGraph;
	friend class NodeGraphControl;
};

class NodeConnector
{
private:
	static Registry<NodeConnector> reg;

protected:
	NCID			id;

	NodeDataType	dataType;
	
	Node			*node;
	int				maxConnections;

	bool			dirty;

	std::unordered_map<NCID, NodeConnection*>		connections;

public:
	//NodeConnector();
	NodeConnector(Node *n, NodeDataType data_type, IOType io_type, const std::string &nc_name, const std::string &nc_desc, int max_connections);
	NodeConnector(const NodeConnector &other);
	virtual ~NodeConnector();
	
	IOType			ioType			= IOType::INVALID;
	std::string		name			= "",
					description		= "";

	static NodeConnector* getNC(NCID nc_id);
	static bool validConnection(NCID nc_id1, NCID nc_id2);

	Node* getNode();
	NCID getId() const;

	NodeDataType getDataType() const;

	int numConnections() const;
	//CMap getConnections();

	//Fully connects/disconnects this NC and the one with the given id
	//bool makeConnection(NCID other_id);
	//bool destroyConnection(NCID other_id);
	
	//Updates this NC's variables to connect/disconnect to the NC with the given id
	void onConnect(NCID other_id, NodeConnection *c);
	void onDisconnect(NCID other_id);

	//Fully disconnects all connections from this NC
	void destroyAllConnections();

	//Returns whether this NC is connected to the NC with the given id
	bool isConnected(NCID other_id) const;

	//bool flushData(FlushPacket &info);
	bool pullData(PullPacket &output);
	bool pushData(PushPacket &input);

	void update(const Time &dt);

	//Resets pushing/pulling variables to clear before calculating again
	void resetConnectionStates();

	friend class Node;
	friend class NodeConnection;

	//TEMP?
	friend class MidiDeviceNode;
};

#endif	//APOLLO_NODE_CONNECTION_H