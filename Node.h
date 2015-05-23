#ifndef APOLLO_NODE_H
#define APOLLO_NODE_H

#include <string>
#include <vector>

//#include "Timing.h"
#include "NodeDataTypes.h"
#include "AStatus.h"
//#include "Vector.h"
//#include "ExpandEnum.h"
#include "OwnedMutex.h"
#include "Saveable.h"

#include "NodeConnection.h"

#include <unordered_map>
#include <fstream>
#include <memory>



enum class NodeType
{
	INVALID = -1,

	NONE = 0,
	AUDIO_TRACK,
	MIDI_TRACK,
	AUDIO_MOD_TRACK,
	MIDI_MOD_TRACK,
	RENDER,
	SPEAKER,
	TIME_MAP,
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


class Node;
class NodeConnector;
class NodeConnection;

class NodeGraph;


typedef NodeConnector* NCOwnedPtr;
typedef NodeConnection* COwnedPtr;

typedef int NID;	//Used as a unique identifier for each Node.
typedef std::unordered_map<NID, Node*> NodeMap;			//A map of NIDs to nodes
typedef std::unordered_map<NCID, NCOwnedPtr> NCMap;		//A map of NCIDs to connectors


struct ChildNodeTree
{
	Node *node;
	std::vector<ChildNodeTree> children;

	ChildNodeTree(Node *n)
		: node(n)
	{ }
};

//typedef unsigned int NCI;		//Used as an index to access each of a Node's connectors.

//Node base class -- defines basic Node functionality.
class Node : public Saveable
{
private:
	//STATIC -- IDs

	static NodeMap registeredNodes;
	static NID		nextId;
	
	bool registerId();
	bool deregisterId();

protected:
	NID							id;				//This Node's id
	NType						type;			//Type of Node

	std::string					name,			//This Node's name
								description;	//A description of this Node

	bool						dirty;			//Whether this Node is dirty (needs to be updated)

	//std::vector<NodeConnector*> connectors;	//A list of this Node's connectors
	NCMap						connectors,
								inputConnectors,	//Only input connectors
								outputConnectors;	//Only output connectors

	OwnedMutex					nodeLock;		//A mutex for when this node is accessed by mutliple threads at once

	
	std::vector<NCID> init(const std::vector<NodeConnectorDesc> &node_connectors);
	void init(const std::vector<NCID> &nc_ids);

	void getChildNodes(const std::vector<NodeType> &types, std::vector<ChildNodeTree> &results);
	
	virtual void onConnect(NCID this_nc, NCID other_nc)		{ }
	virtual void onDisconnect(NCID this_nc, NCID other_nc)	{ }
	
	//virtual void initNode()				{ };
	virtual void onUpdate(double dt)	{ }		//What to be done when the Node is updated 
	virtual void updateDesc() override;

public:
	Node(NType node_type, std::string node_name, std::string node_desc);
	Node(const NDesc &n_desc);
	Node(const Node &other);
	virtual ~Node();

	static bool validId(NID id);

	NodeGraph *parentGraph;

	NID getId() const;
	
	std::string getName() const;
	std::string getDesc() const;
	void setName(const std::string &new_name);
	void setDesc(const std::string &new_desc);

	bool isLocked() const;
	bool isDirty() const;

	bool connect(NCID this_nc, NCID other_nc);
	bool disconnect(NCID this_nc, NCID other_nc);
	void disconnectAll(NCID this_nc);

	std::vector<NCOwnedPtr> getConnectors();
	
	//These functions define functionality of data communication with this Node.
	virtual bool flushData(FlushPacket &info);
	virtual bool pullData(PullPacket &output, NCID this_id) = 0;
	virtual bool pushData(PushPacket &input, NCID this_id) = 0;

	virtual void update(double dt);

	NType getType() const;

	friend class NodeConnector;
};



/*
struct NDesc;
struct NCDesc;

enum NodeFlags
{
	//Flags
	NF_AUDIO = 0x01,
	NF_MIDI = 0x02,
	NF_TRACK = 0x04,
	NF_INFO = 0x08,
	NF_MOD = 0x10,
	NF_DEVICE = 0x20,
};

enum class NodeType
{
	INVALID = -1,

	NONE = 0,
	AUDIO_TRACK,
	MIDI_TRACK,
	AUDIO_MOD_TRACK,
	MIDI_MOD_TRACK,
	RENDER,
	SPEAKER,
	TIME_MAP,
	INSTRUMENT,
	MIDI_DEVICE,

	COUNT
	
	//
	//NONE = 0x00,

	////Node Types

	//AUDIO_TRACK = NF_AUDIO | NF_TRACK,
	//MIDI_TRACK = NF_MIDI | NF_TRACK,
	//MOD_TRACK = NF_MOD | NF_TRACK,
	//AUDIO_MOD_TRACK = NF_AUDIO | MOD_TRACK,
	//MIDI_MOD_TRACK = NF_MIDI | MOD_TRACK,

	//MIDI_DEVICE = NF_MIDI | NF_DEVICE,
	//SPEAKER = NF_AUDIO | NF_DEVICE,

	//RENDER = 0x40,
	//TIME_MAP = 0x80,
	//INSTRUMENT = 0x100 | NF_INFO,

	////Make sure to update this if any values are added/removed!!
	//COUNT = 11
	
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

enum class NodeDirection
{
	NONE = 0x00,

	FORWARD = 0x01,	//Data being pushed
	BACKWARD = 0x02	//Data being pulled
};
typedef NodeDirection NDir;

EXPAND_ENUM_CLASS_OPERATORS(NDir)
EXPAND_ENUM_CLASS_VALID(NDir, NDir::NONE)
//EXPAND_ENUM_CLASS_PRINT(NDir,	{NDir::NONE, NDir::FORWARD, NDir::BACKWARD},
//								{"NONE", "FORWARD", "BACKWARD"})


class Node;
class NodeConnector;
class NodeGraph;


//Represents a unique id for each Node
typedef unsigned int NodeId;
//Used as a list of Nodes
typedef std::unordered_map<NCID, Node*> NodeList;


struct NDesc;
struct NCDesc;

//Node Base Class
class Node : public Saveable
{
	friend struct NDesc;
	friend class NodeGraph;
	friend class NodeConnector;

private:
	static NodeList		regIds;
	static NodeId		nextId;

	NodeId				id = 0;	//id == 0 means invalid node
	NDesc				*desc = nullptr;
	
	AStatus registerId();
	AStatus deregisterId();

protected:
	//Connectors
	NCList						connectors;
	std::vector<NodeConnector*> connectorList;
	
	NodeGraph	*parentGraph = nullptr;

	//Name/Description
	std::string nodeName = "",
				nodeType = "",
				nodeDesc = "";

	bool		dirty = true;
	
	//Should be overriden in child classes to call init()
	virtual void initNode() = 0;

	std::vector<NCID> init(std::vector<NodeConnectorDesc> &node_connectors);
	void init(const std::vector<NCID> &nc_ids);
	
	//Helper function for getClosestNode() that performs the recursion.
	Node* findClosestNodeType(NodeType type, NodeDataType allowed_types,int &distance, NCType io1, NCType io2, int depth, int max_depth);

	AStatus propogateFlush(FlushPacket &info);

	virtual void onUpdate(double dt) { }

public:
	Node(std::string node_type, std::string node_desc);
	Node(const NDesc &n_desc);
	virtual ~Node();
	
	NodeId getId();
	static Node* getNode(NodeId n_id);

	static AStatus connect(NCID nc1, NCID nc2);
	
	//Mutex for thread safety
	OwnedMutex	nodeLock;		//For accessing/writing data concurrently

	//Name/Descrition
	void setName(std::string name);
	void setDescription(std::string desc);
	std::string getName();
	std::string getDescription();

	//Connections
	std::vector<NodeConnector*> getInputConnectors();
	std::vector<NodeConnector*> getOutputConnectors();

	unsigned int numConnectors() const;
	const std::vector<NodeConnector*>& getConnectors();
	NodeConnector* getConnector(NCID id);

	virtual void onConnect(NodeConnector *this_nc, NodeConnector *other_nc) { }
	virtual void onDisconnect(NodeConnector *this_nc, NodeConnector *other_nc) { }

	//Finds the closest node to this one with the given parameters.
	Node* getClosestNode(NodeType type, NodeDataType allowed_data_types, int &distance, bool backward = true, int max_depth = -1);
	
	bool isLocked() const;

	//Data propagation//
	virtual bool canPull() = 0;
	virtual bool canPush() = 0;
	virtual bool canFlush() = 0;
	
	//TODO: Make separate PLAYABLE Node class
	virtual bool hasCursor() const;
	virtual bool isPlaying();

	//Should flush any accumulating data when called (most likely by a speaker node)
	virtual AStatus flush(FlushPacket &info);

	//Function to pull updated data from previous Nodes.
	//If output is null, pointer to cached data should be passed (faster). Otherwise, the data should be copied into output (return value should be output).
	virtual AStatus pullData(PullPacket &output, NCID this_id) = 0;
	virtual AStatus pushData(PushPacket &input, NCID this_id, NCID other_id) = 0;

	bool isDirty() const;

	virtual AStatus setDirty(ChunkRange r);
	virtual AStatus clean(ChunkRange r);

	void update(double dt);
	
	virtual NodeType getType() = 0;
	
protected:
	virtual void updateDesc() override;
};
*/

#endif	//APOLLO_NODE_H