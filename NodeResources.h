#ifndef APOLLO_NODE_CONNECTOR_DESC_H
#define APOLLO_NODE_CONNECTOR_DESC_H

#include <unordered_map>
#include <memory>

#include "ExpandEnum.h"
#include "Registry.h"

class Node;
class NodeConnector;
class NodeConnection;

class PushPacket;
class PullPacket;
class FlushPacket;

////ID TYPES////
typedef RegId NID;	//Used as a unique identifier for each Node.
typedef RegId NCID;	//Used as a unique identifier for each NodeConnector
typedef RegId CID;	//Used as a unique identifier for each NodeConnection.

////POINTER TYPES////
typedef std::unique_ptr<Node> OwnedNode;
//typedef std::weak_ptr<Node> NodePtr;

typedef std::unique_ptr<NodeConnector> OwnedNodeConnector;
//typedef std::weak_ptr<NodeConnector> NodeConnectorPtr;

typedef std::unique_ptr<NodeConnection> OwnedNodeConnection;
//typedef std::weak_ptr<NodeConnection> NodeConnectionPtr;


////MAP TYPES////
typedef std::unordered_map<NID, Node*> NodeMap;			//A map of NIDs to nodes

//typedef std::unordered_map<NCID, NCOwnedPtr> NCMap;	//A map of NCIDs to connectors
//typedef std::unordered_map<NCID, COwnedPtr> CMap;		//A map of NCIDs to connections


////GLOBAL CLOCK/////
//HRes_Clock NODE_CLOCK;


////ENUMS/////
enum class NodeType
{
	INVALID = -1,

	NONE = 0,

	STATIC_AUDIO_BUFFER,
	STATIC_MIDI_BUFFER,
	DYNAMIC_AUDIO_BUFFER,
	DYNAMIC_MIDI_BUFFER,

	STATIC_MOD_BUFFER,
	DYNAMIC_MOD_BUFFER,

	//MOD_TRACK

	READ,	//Files
	WRITE,

	RENDER,
	SPEAKER,
	MICROPHONE,
	INSTRUMENT,
	MIDI_DEVICE,

	COUNT
};
typedef NodeType NType;
inline bool operator!(NType n1)
{
	return static_cast<int>(n1) <= 0;
}
inline int toIndex(NType nt)
{
	return static_cast<int>(nt);
}


enum class NodeDataType
{
	INVALID			= -1,
	NONE			= 0x00,

	PUSH			= 0x01,
	PULL			= 0x02,

	TIME			= 0x04,

	AUDIO			= 0x08,
	MIDI			= 0x10,

	INFO			= 0x20,

	FLUSH			= 0x40,

	INSTRUMENT		= 0x80,
	AUDIO_INFO		= 0x100
};
typedef NodeDataType NodeData;

EXPAND_ENUM_CLASS_OPERATORS(NodeDataType)
EXPAND_ENUM_CLASS_VALID(NodeDataType, NodeData::NONE)
EXPAND_ENUM_CLASS_PRINT(NodeDataType,
						({ NodeData::INVALID, NodeData::NONE, NodeData::PULL, NodeData::PUSH, NodeData::TIME, NodeData::AUDIO, NodeData::MIDI, NodeData::INFO, NodeData::INSTRUMENT, NodeData::AUDIO_INFO }),
						({ "INVALID", "NONE", "PULL", "PUSH", "TIME", "AUDIO", "MIDI", "INFO", "INSTRUMENT", "AUDIO_INFO" }) )


enum class TransferMethod
{
	INVALID = -1,
	COPY = 0,
	DIRECT,
	ADD
};


struct ChildNodeTree
{
	Node *node;
	std::vector<ChildNodeTree> children;

	ChildNodeTree(Node *n)
		: node(n)
	{ }
};


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


////DESCRIPTORS////
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

#endif	//APOLLO_NODE_CONNECTOR_DESC_H