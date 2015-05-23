#ifndef APOLLO_NODE_CONNECTION_H
#define APOLLO_NODE_CONNECTION_H

#include <unordered_map>
#include <memory>

#include "NodeDataTypes.h"
#include "ExpandEnum.h"

enum class NodeConnectionType
{
	INVALID = -1,

	DATA_INPUT = 0,
	DATA_OUTPUT,
	INFO_INPUT,
	INFO_OUTPUT,

	COUNT
};
typedef NodeConnectionType IOType;

inline unsigned int toIndex(IOType type)
{
	return static_cast<unsigned int>(type);
}

inline bool isOutput(IOType type)
{
	return type == IOType::DATA_OUTPUT || type == IOType::INFO_OUTPUT;
}

inline bool isInput(IOType type)
{
	return type == IOType::DATA_INPUT || type == IOType::INFO_INPUT;
}

inline IOType getOpposite(IOType type)
{
	switch(type)
	{
	case IOType::DATA_INPUT:
		return IOType::DATA_OUTPUT;
	case IOType::DATA_OUTPUT:
		return IOType::DATA_INPUT;
	case IOType::INFO_INPUT:
		return IOType::INFO_OUTPUT;
	case IOType::INFO_OUTPUT:
		return IOType::INFO_INPUT;
	default:
		return IOType::INVALID;
	}
}

enum class NodeConnectionDirection
{
	NONE = 0x00,

	FORWARD = 0x01,	//Data being pushed
	BACKWARD = 0x02	//Data being pulled
};
typedef NodeConnectionDirection NCDir;
EXPAND_ENUM_CLASS_OPERATORS(NCDir)
EXPAND_ENUM_CLASS_VALID(NCDir, NCDir::NONE)



class Node;
class NodeConnector;
class NodeConnection;

//typedef NodeConnector* NCOwnedPtr;
//typedef NodeConnection* COwnedPtr;

typedef int NCID;	//Used as a unique identifier for each NodeConnector
typedef std::unordered_map<NCID, NCOwnedPtr> NCMap;	//A map of NCIDs to connectors
typedef std::unordered_map<NCID, COwnedPtr> CMap;	//A map of NCIDs to connections

class NodeConnection
{

protected:
public:
	NCID						fromId		= -1,
								toId		= -1;

	NodeConnector				*fromNc		= nullptr,
								*toNc		= nullptr;
	
	Node						*fromNode	= nullptr,
								*toNode		= nullptr;

	NCDir						activeDir	= NCDir::NONE;
	
	static const double			ACTIVE_RESET_TIME;
	Timer						resetForwardActive,
								resetBackwardActive;

public:
	NodeConnection(NCID from_id, NCID to_id);
	//NodeConnection(NodeConnector *from_nc, NodeConnector *to_nc);
	virtual ~NodeConnection();
	
	bool isConnected() const;

	NCDir getActiveDir() const;
	void setActive(NCDir direction);

	void update(double dt);

};



struct NodeConnectorDesc
{
	NodeDataType		dataType = NodeData::INVALID;
	NodeConnectionType	ioType = IOType::DATA_INPUT;

	std::string name,
				desc;
	int maxConnections;

	NodeConnectorDesc(NodeDataType data_type, IOType io_type, const std::string &nc_name, const std::string &nc_desc, int max_connections)
		: dataType(data_type), ioType(io_type), name(nc_name), desc(nc_desc), maxConnections(max_connections)
	{ }
};

class NodeConnector
{
private:
	//STATIC IDs
	static NCMap	registeredConnectors;
	static NCID		nextId;

	bool registerId();
	bool deregisterId();

protected:
	NCID			id;

	NodeDataType	dataType;
	
	Node			*node;
	std::string		name,
					description;
	int				maxConnections;

	bool			dirty;

	CMap			connections;

public:
	//NodeConnector();
	NodeConnector(Node *n, NodeDataType data_type, IOType io_type, const std::string &nc_name, const std::string &nc_desc, int max_connections);
	NodeConnector(const NodeConnector &other);
	virtual ~NodeConnector();
	
	static NodeConnector* getNC(NCID nc_id);

	//NCDir			activeDir = NCDir::NONE;
	IOType			ioType = IOType::INVALID;

	Node* getNode();
	NCID getId() const;

	std::string getName() const;
	std::string getDesc() const;
	void setName(const std::string &new_name);
	void setDesc(const std::string &new_desc);

	NodeDataType getDataType() const;

	int numConnections() const;
	CMap getConnections();
	
	bool isConnected(NCID other_id) const;
	bool connect(NCID other_id);
	bool disconnect(NCID other_id);
	bool disconnectHalf(NCID other_id);	//Disconnects this half of the connection, leaves it hanging
	void disconnectAll();

	bool flushData(FlushPacket &info);
	//bool pullData(PullPacket &output, NCID other_id = -1);
	bool pullData(PullPacket &output);
	bool pushData(PushPacket &input);

	void update(double dt);

	friend class Node;
	friend class NodeConnection;
};




/*


//Represents a unique id for each NodeConnector
typedef unsigned int NCID;
//Used as a list of node connectors
typedef std::unordered_map<NCID, NodeConnector*> NCList;


class Node;

//Node Connection -- holds data about the connection between two nodes
//	- FROM ==> TO

struct NodeConnection
{
	NCID			fromId = 0,
					toId = 0;

	NodeConnector	*fromNc = nullptr,
					*toNc = nullptr;

	Node			*fromNode = nullptr,
					*toNode = nullptr;

	NodeConnection(NCID from_id, NCID to_id);
	~NodeConnection() = default;

	bool isConnected() const;
};

typedef std::shared_ptr<NodeConnection> NCOwnedPtr;
typedef std::weak_ptr<NodeConnection> NCAccessPtr;




//Node Connector Descriptor -- describes a Node Connector; to be used for Node Connector creation.
struct NodeConnectorDesc
{
	NodeDataType		dataType = NodeData::INVALID;
	NodeConnectionType	ioType = NCType::DATA_INPUT;

	std::string			name = "",
						desc = "";

	int					maxConnections = 1;

	//
	//NodeConnectorDesc()
	//	: dataType(NODE_DATA_INVALID), ioType(NCType::INPUT), name(""), desc(""), maxConnections(0)
	//{ }
	
	NodeConnectorDesc() { }

	NodeConnectorDesc(NodeDataType data_type, NodeConnectionType io, std::string name_, std::string desc_, int max_connections)
		: dataType(data_type), ioType(io), name(name_), desc(desc_), maxConnections(max_connections)
	{ }
};

//Node Connector -- defines a slot for a Node to be connected to another Node to acheive some function.
class NodeConnector : public Saveable
{
private:
	static NCList regIds;
	static NCID nextId;

	AStatus registerId();
	AStatus deregisterId();

	NCID						id = 0;	//ncId == 0 means invalid connector

	static const double			ACTIVE_RESET_TIME;
	Timer						resetForwardActive,
								resetBackwardActive;
	
	static AStatus connectorsCompatible(const NodeConnector *nc1, const NodeConnector *nc2);

	NodeId						nId = 0;
	std::vector<NCID>			cIds;

public:
	std::string					name = "",
								desc = "";
	
	const NodeDataType			dataType = NodeData::INVALID;
	const NodeConnectionType	ioType = NCType::DATA_INPUT;

	Node						*node = nullptr;

	NDir						activeDir = NDir::NONE;	//In which direction the node connector is currently active

	NCID getId();

	//Used for connection indices
	typedef int CIndex;

protected:
	int							maxConnections = 0;	//maxConnections < 0 means it can have infinite connections.
													//maxConnections == 0 means this connector is invalid.
	
	bool						dirty = true;

	std::unordered_map<NCID, NCOwnedPtr> connections;
	//std::vector<NodeConnection*> 

	bool validConnector() const;

public:
	NodeConnector();
	NodeConnector(Node *n, NodeConnectorDesc &c_desc);
	NodeConnector(const NCDesc &nc_desc);
	~NodeConnector();
	
	static NodeConnector* getNodeConnector(NCID nc_id);
	
	OwnedMutex connectorLock;	//Mutex for thread safety on this NodeConnector's data objects

	//Connects this Node Connector to another. Checks if data types match.
	//  Only one call to this function necessary per connector pair.
	AStatus connect(NCID nc_id);
	AStatus connect(NodeConnector *nc);

	NCOwnedPtr startConnect();
	void finishConnect(NCID other_id);
	
	//Disconnects one of this node's connections (specified by id)
	//Only one call to this function necessary per connector pair.
	AStatus disconnect(NCID nc_id);
	AStatus disconnect(NodeConnector *nc);
	//AStatus disconnect(CIndex index);

	//Disconnects this Node Connector from everything it was connected to.
	void disconnectAll();
	
	//void getPointers();	//Gets actual pointers of nodes/connectors from stored ids

	//Returns the node that owns this connector.
	Node* getNode();
	//Returns the node connected at the specified index.
	//Node* getConnectedNode(CIndex index);

	//Returns the connection at the given index, or nullptr if it was out of bounds.
	NCOwnedPtr getConnection(NCID nc_id);		//THREAD SAFE
	//std::vector<NodeConnector*>& getConnections();
	std::unordered_map<NCID, NCOwnedPtr> getConnections();
	//Returns the index that the given connection is at, or -1 if there is no connection to that nc.
	//CIndex getIndex(NodeConnector *nc);				//THREAD SAFE
	//CIndex getIndex(NCID nc_id);					//THREAD SAFE

	//Returns the number of connections that are connected.
	unsigned int numConnections() const;						//THREAD SAFE
	//Returns the number of connections that are unconnected.
	//	Returns -1 if unlimited.
	int numOpenConnections() const;								//THREAD SAFE
	bool hasConnections() const;								//THREAD SAFE

	bool isActive();
	NDir activeDirection();
	void setActive(NDir active_dir, NCID nc_id);	//index should be index of other node (both should be active)

	bool canPull(NCID nc_id);
	bool canPush(NCID nc_id);
	
	//Returns whether the given data type is compatible with this connector.
	bool checkType(NodeData type) const;

	//Updates timer Ticking down whether this connection is active.
	void update(double dt);

	//Returns whether this path has a node with a cursor (currently only TimeMapNode)
	// -- If theres no cursor, a speaker can't pull data from it since it's based in real time (not mapped time)
	bool hasTimeline();
	bool timelineIsPlaying();
	
	bool isDirty() const;
	AStatus setDirty(ChunkRange r);
	//AStatus clean(TimeRange r);

	//Requests that this node and all preceding ones flush any accumulating data.
	AStatus flushData(FlushPacket &info);

	//Pulls data from the connected Node (if connected to one)
	AStatus pullData(PullPacket &output, NCID nc_id);
	//Pushes data to all active connections.
	AStatus pushData(PushPacket &input);
	
protected:
	virtual void updateDesc() override;
};
*/


#endif	//APOLLO_NODE_CONNECTION_H