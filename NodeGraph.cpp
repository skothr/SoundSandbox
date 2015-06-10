#include "NodeGraph.h"

#include "AUtility.h"

#include "Node.h"
#include "NodeBaseTypes.h"
#include "TrackNodes.h"
#include "InfoNodes.h"
#include "SpeakerNode.h"
#include "MicrophoneNode.h"
#include "InstrumentNode.h"
#include "MidiDeviceNode.h"
#include "RenderNode.h"
#include "ProjectTrackDisplay.h"
#include "NodeConnection.h"

#include "MidiBufferNode.h"
#include "AudioBufferNode.h"

#include "MidiData.h"
#include "ThreadedObject.h"

#include "Note.h"	//For testing

#define SPEAKER_BUFFER_SIZE 32//ceil(44100.0/AUDIO_CHUNK_SIZE)	//1 second of data
#define MIC_BUFFER_SIZE 16

int midi_port = -1;

/*
/////NODE TREE/////
//std::unordered_map<Node*, NodeTree*>	NodeTree::traversedNodes;
//bool										NodeTree::handled = false;

NodeTree::NodeTree(Node *start_node)
	: node(start_node)
{
	bool handling = !handled;

	if(handling)
	{
		traversedNodes.clear();
		handled = true;
	}

	buildTree();

	if(handling)
	{
		traversedNodes.clear();
		handled = false;
	}
}

NodeTree::~NodeTree()
{
	//TODO: Delete all NodeTrees in this tree
	
	//bool handling = !handled;

	//if(handling)
	//{
	//	traversedNodes.clear();
	//	handled = true;
	//}

	//traversedNodes[node] = this;

	//for(auto nt : forward)
	//{

	//}

	//if(handling)
	//{
	//	traversedNodes.clear();
	//	handled = false;
	//}
	
}

void NodeTree::buildTree()
{
	//Mark off this node
	traversedNodes[node] = this;

	std::vector<NodeConnector*> connectors = node->getConnectors();

	for(auto nc : connectors)
	{
		if(isInput(nc->ioType))
		{
			//INPUT (go backwards)
			for(unsigned int i = 0; i < nc->numConnections(); i++)
			{
				NodeConnector *c = nc->getConnection(i);
				auto found = traversedNodes.find(c->getNode());

				if(found != traversedNodes.end())
				{
					//Already traversed this node -- get existing NodeTree
					backward.push_back(found->second);
				}
				else
				{
					//Havent traversed this node yet -- make new NodeTree (RECURSES)
					NodeTree *new_nt = new NodeTree(c->getNode());
					backward.push_back(new_nt);
				}
			}
		}
		else
		{
			//OUTPUT (go forwards)
			for(unsigned int i = 0; i < nc->numConnections(); i++)
			{
				NodeConnector *c = nc->getConnection(i);
				auto found = traversedNodes.find(c->getNode());

				if(found != traversedNodes.end())
				{
					//Already traversed this node -- get existing NodeTree
					forward.push_back(found->second);
				}
				else
				{
					//Havent traversed this node yet -- make new NodeTree (RECURSES)
					NodeTree *new_nt = new NodeTree(c->getNode());
					forward.push_back(new_nt);
				}
			}
		}
	}
}

void NodeTree::print(int level)
{
	traversedNodes[node] = this;

	std::string indent = "";
	for(int i = 0; i < level; i++)
		indent += "\t";

	if(node)
	{
		std::cout << indent << node->getName() << "\n";

		std::cout << indent << "FORWARD\n";
		for(auto nt : forward)
		{
			if(traversedNodes.find(nt->node) == traversedNodes.end())
				nt->print(level + 1);
		}

		//std::cout << indent << "-----" << "\n";
		std::cout << indent << "BACKWARD\n";

		for(auto nt : backward)
		{
			if(traversedNodes.find(nt->node) == traversedNodes.end())
				nt->print(level - 1);
		}
	}
	else
	{
		std::cout << indent << "<NULL>\n";
	}
}
*/

/////NODE GRAPH/////
/*
std::vector<Point2f> NodeGraph::presetPositions[static_cast<unsigned int>(NodeGraphPreset::COUNT)]
{	{	//EMPTY
	
	},

	{	//SANDBOX
		Point2f(-200.0f, -25.0f),	//MIDI_DEVICE
		Point2f(-50.0f, -25.0f),	//RENDER
		Point2f(-50.0f, -175.0f),	//INSTRUMENT
		Point2f(100.0f, -25.0f)		//SPEAKER
	},

	{	//PROJECT
		Point2f(-300.0f, -25.0f),	//MIDI_BUF
		Point2f(-137.5f, -175.0f),	//INSTRUMENT 1
		Point2f(-150.0f, -75.0f),	//RENDER
		Point2f(-25.0f, -75.0f),	//AUDIO_BUF
		Point2f(250.0f, -25.0f),	//SPEAKER
		Point2f(100.0f, -25.0f),	//TIME_MAP

		Point2f(-300.0f, 150.0f),	//MIDI_DEVICE
		Point2f(-137.5f, 25.0f),	//INSTRUMENT 2
		Point2f(-150.0f, 150.0f),	//RENDER 2
		//Point2f(-25.0f, 150.0f)		//AUDIO_BUF 2
	}
};
*/


NodeGraph::NodeGraph()
{ }

NodeGraph::NodeGraph(std::unordered_map<Node*, Point2f> &g_nodes, std::unordered_set<NodeConnection*> g_connections)
{
	nodes.reserve(g_nodes.size());
	connections.reserve(g_connections.size());

	//Emplace nodes
	for(auto &n : g_nodes)
		nodes.emplace(n.first->getId(), std::unique_ptr<Node>(n.first));

	//Emplace connections
	for(auto &c : g_connections)
		connections.emplace(c->getId(), std::unique_ptr<NodeConnection>(c));
}

NodeGraph::~NodeGraph()
{
	resetGraph();
}

/*
void NodeGraph::addConnection(COwnedPtr nc)
{
	nodeConnections.emplace(nc);
}

void NodeGraph::removeConnection(COwnedPtr nc)
{
	nodeConnections.erase(nc);
}
*/

void NodeGraph::addNode(Node *n, Point2f g_pos)
{
	auto &iter = nodes.emplace(n->getId(), std::unique_ptr<Node>(n));

	iter.first->second.get()->graphPos = g_pos;
	iter.first->second.get()->parentGraph = this;
}

bool NodeGraph::removeNode(NID id)
{
	Node *n = getNode(id);

	if(n)
	{
		//TODO
		//Remove all connections including this node
		//for(auto c : connections)
		//{
		//	if(c->from
		//}
		
		//Delete node
		nodes.erase(id);

		return true;
	}
	else
		return false;
}
/*
bool NodeGraph::connectNodes(NID n1, NID n2)
{
	std::unique_ptr<Node>	*node1 = getNode(n1),
							*node2 = getNode(n2);

	return (node1 && node2) ? connectNodes(*node1->get(), *node2->get()) : false;
}

bool NodeGraph::connectNodes(Node &n1, Node &n2)
{
	connections.emplace(new NodeConnection(
}
*/

NodeConnection* NodeGraph::makeNewConnection(NCID nc1, NCID nc2)
{
	NodeConnection *c = new NodeConnection(nc1, nc2);
	auto &iter = connections.emplace(c->getId(), std::unique_ptr<NodeConnection>(c));

	return iter.first->second.get();

	//if(nc.fromIsConnected())
	//	nc.fromNc->onConnect(nc.toId, *iter.first->second.get());
	//if(nc.toIsConnected())
	//	nc.toNc->onConnect(nc.fromId, *iter.first->second.get());
}

NodeConnection* NodeGraph::makeNewConnection(NCID nc)
{
	NodeConnection *c = new NodeConnection(nc);
	auto &iter = connections.emplace(c->getId(), std::unique_ptr<NodeConnection>(c));

	return iter.first->second.get();
}

bool NodeGraph::removeConnection(CID id)
{
	auto iter = connections.find(id);

	if(iter != connections.end())
	{
		connections.erase(iter);
		return true;
	}
	else
		return false;
}


Node* NodeGraph::getNode(NID id)
{
	auto iter = nodes.find(id);

	return (iter != nodes.end()) ? iter->second.get() : nullptr;
}

const std::unordered_map<NID, Node*> NodeGraph::getNodes()
{
	std::unordered_map<NID, Node*> g_nodes;
	g_nodes.reserve(nodes.size());

	for(auto &n : nodes)
		g_nodes.emplace(n.first, n.second.get());

	return g_nodes;
}

const std::unordered_map<CID, NodeConnection*> NodeGraph::getConnections()
{
	std::unordered_map<CID, NodeConnection*> g_conn;
	g_conn.reserve(nodes.size());

	for(const auto &c : connections)
		g_conn.emplace(c.first, c.second.get());

	return g_conn;
}


/*
const std::vector<Point2f>& NodeGraph::getInitialPos()
{
	return *initialPos;
}
*/

/*
//TODO: Clean this awful shit
NodeTree* NodeGraph::getNodeTree()
{
	Node *start_node = nullptr;
	
	//Find the first node that doesn't have input connectors (leaf node)
	for(auto n : nodes)
	{
		bool	has_input = false,
				has_output = false;

		std::vector<NodeConnector*> connectors = n->getConnectors();
		for(auto c : connectors)
		{
			if((has_input |= isInput(c->ioType)) && (has_output |= isOutput(c->ioType)))
				break;
		}

		if(!has_input)
		{
			start_node = n;
			break;
		}
	}

	return new NodeTree(start_node);
}
*/

unsigned int NodeGraph::getNumNodes() const
{
	return nodes.size();
}

unsigned int NodeGraph::getNumConnections() const
{
	return connections.size();
}


void NodeGraph::resetGraph()
{
	//Stop all currently running devices/threads
	stopAllDevices();

	//Delete all connections
	connections.clear();
	
	//Now delete all nodes
	nodes.clear();
}

void NodeGraph::update(const Time &dt)
{
	for(auto &n : nodes)
		n.second->update(dt);
}

void NodeGraph::resetConnectionStates()
{
	for(auto &c : connections)
		c.second->resetConnectionStates();
}

void NodeGraph::setPreset(NodeGraphPreset preset, int sample_rate)
{
	AStatus status;

	StaticMidiBufferNode	*mainMidiBuf;
	InstrumentNode			*instrument1,
							*instrument2;
	RenderNode				*mainRender;
	DynamicAudioBufferNode	*mainAudioBuf,
							*audioBuf2;
	SpeakerNode				*mainSpeaker;
	MicrophoneNode			*mainMic;

	MidiDeviceNode			*mainDevice;

	RenderNode				*render2;

	std::unique_ptr<Node>					mainMidiBuf_ptr,
											instrument1_ptr,
											instrument2_ptr,
											mainRender_ptr,
											mainAudioBuf_ptr,
											audioBuf2_ptr,
											mainSpeaker_ptr,
											mainMic_ptr,
											mainDevice_ptr,
											render2_ptr;

	MidiData notes;	//For testing

	
	resetGraph();
	//initialPos = &presetPositions[toIndex(preset)];


	switch(preset)
	{
	//Empty graph (do nothing)
	case NodeGraphPreset::EMPTY:
		break;

	//Initial NodeGraph for a new sandbox
	case NodeGraphPreset::SANDBOX:

		//Preset Nodes
		instrument1 = new InstrumentNode(this);
		mainRender = new RenderNode(this, sample_rate);
		mainSpeaker = new SpeakerNode(this, 0, sample_rate, SPEAKER_BUFFER_SIZE, nullptr);
		mainMic = new MicrophoneNode(this, 4, sample_rate, MIC_BUFFER_SIZE);
		mainDevice = new MidiDeviceNode(this, midi_port);	//TODO: Choose port with gui
		
		//Add nodes to list
		addNode(mainDevice, Point2f(-200.0f, -25.0f));
		addNode(mainRender, Point2f(-50.0f, -25.0f));
		addNode(instrument1, Point2f(-50.0f, -175.0f));
		addNode(mainSpeaker, Point2f(100.0f, -25.0f));
		addNode(mainMic,	Point2f(-100.0f, 75.0f));

		//Preset Connections
		//Node::connect(mainDevice->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		//Node::connect(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		//Node::connect(mainRender->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);

		makeNewConnection(mainDevice->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		makeNewConnection(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		makeNewConnection(mainRender->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		
		break;

	//Initial NodeGraph for a new project
	case NodeGraphPreset::PROJECT:
		//Preset Nodes
		mainMidiBuf = new StaticMidiBufferNode(this);
		instrument1 = new InstrumentNode(this);
		mainRender = new RenderNode(this, sample_rate);
		mainAudioBuf = new DynamicAudioBufferNode(this, sample_rate, 1000);
		mainSpeaker = new SpeakerNode(this, 0, sample_rate, SPEAKER_BUFFER_SIZE, nullptr);
		mainMic = new MicrophoneNode(this, 3, sample_rate, MIC_BUFFER_SIZE);

		//timeMap = new TimeMapNode(sample_rate);

		mainDevice = new MidiDeviceNode(this, midi_port);
		instrument2 = new InstrumentNode(this);
		render2 = new RenderNode(this, sample_rate);
		//audioBuf2 = new AudioTrackNode(sample_rate);
		
		//Add nodes to list
		addNode(mainMidiBuf, Point2f(-300.0f, -25.0f));
		addNode(instrument1, Point2f(-137.5f, -175.0f));
		addNode(mainRender, Point2f(-150.0f, -75.0f));
		addNode(mainAudioBuf, Point2f(-25.0f, -75.0f));
		addNode(mainSpeaker, Point2f(250.0f, -25.0f));
		addNode(mainMic,	Point2f(-100.0f, 75.0f));

		//addNode(timeMap, Point2f(100.0f, -25.0f));

		addNode(mainDevice, Point2f(-300.0f, 150.0f));
		addNode(instrument2, Point2f(-137.5f, 25.0f));
		addNode(render2, Point2f(-150.0f, 150.0f));
		//addNode(audioBuf2, Point2f(-25.0f, 150.0f));


		//Preset Connections
		makeNewConnection(mainMidiBuf->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		makeNewConnection(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		makeNewConnection(mainRender->OUTPUTS.AUDIO_ID, mainAudioBuf->INPUTS.AUDIO_ID);
		makeNewConnection(mainAudioBuf->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		//timeMap->connect(timeMap->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		
		makeNewConnection(mainDevice->OUTPUTS.MIDI_ID, render2->INPUTS.MIDI_ID);
		makeNewConnection(instrument2->OUTPUTS.INSTRUMENT_ID, render2->INPUTS.INSTRUMENT_ID);
		makeNewConnection(render2->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);


		////ADD TEST NOTES (Legend of Zelda Main Theme)////
		//legendOfZeldaTheme1.interpret(notes, 0.0, 30);
		//legendOfZeldaTheme2.interpret(notes, 0.0, 30, 100);
		//notes.addNote(MidiNote(-1, 0, 0.0, 100.0));	//100-second long rest (empty)
		//mainMidiBuf->setBuffer(notes);

		mainMidiBuf->setLength(360.0);
		mainAudioBuf->setLength((c_time)(mainMidiBuf->getLength()*(Time)((double)sample_rate/(double)AUDIO_CHUNK_SIZE)));

		break;

	default:	//NODEGRAPH_PRESET_INVALID
		//Error
		break;
	}
}

void NodeGraph::stopAllDevices()
{
	for(auto &n : nodes)
	{
		ThreadedObject *t = dynamic_cast<ThreadedObject*>(n.second.get());
		if(t)
			t->shutDownThread();
	}
}