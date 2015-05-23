#include "NodeGraph.h"

#include "AUtility.h"

#include "NodeBaseTypes.h"
#include "TrackNodes.h"
#include "InfoNodes.h"
#include "IONodes.h"
#include "RenderNode.h"
#include "ProjectTrackDisplay.h"

#include "Vector.h"

#include "Note.h"	//For testing

#define SPEAKER_BUFFER_SIZE 32//ceil(44100.0/AUDIO_CHUNK_SIZE)	//1 second of data

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
{
	nodes.clear();
	nodeConnections.clear();
}

NodeGraph::NodeGraph(const std::unordered_map<Node*, Point2f> &g_nodes)
{
	nodes.reserve(g_nodes.size());
	nodes.insert(g_nodes.begin(), g_nodes.end());

	//Find node connections
	for(auto n : nodes)
	{
		std::vector<NodeConnector*> n_conn = n.first->getConnectors();
		for(auto nconn : n_conn)
		{
			//std::unordered_map<NCID, NCOwnedPtr> c = nconn->getConnections();
			CMap c = nconn->getConnections();

			for(auto nc : c)
			{
				//if(nodeConnections.count(nc.second) == 0)
				//{
					nodeConnections.emplace(nc.second);
				//}
			}
		}
	}
}

NodeGraph::~NodeGraph()
{
	reset();
}

void NodeGraph::addConnection(COwnedPtr nc)
{
	nodeConnections.emplace(nc);
}

void NodeGraph::removeConnection(COwnedPtr nc)
{
	nodeConnections.erase(nc);
}

void NodeGraph::addNode(Node *n, Point2f g_pos)
{
	nodes.emplace(n, g_pos);
	n->parentGraph = this;
	
	std::vector<NodeConnector*> n_conn = n->getConnectors();
	for(auto nc : n_conn)
	{
		//std::unordered_map<NCID, NCOwnedPtr> c = nc->getConnections();
		CMap c = nc->getConnections();

		for(auto nconn : c)
		{
			if(nodeConnections.count(nconn.second) == 0)
				nodeConnections.emplace(nconn.second);
		}
	}
}

/*
void NodeGraph::removeNode(unsigned int index)
{
	AU::safeDelete(nodes[index]);
	nodes.erase(nodes.begin() + index);
}
*/
void NodeGraph::removeNode(Node *n)
{
	/*
	for(unsigned int i = 0; i < nodes.size(); i++)
	{
		if(nodes[i] == n)
		{
			removeNode(i);
			break;
		}
	}
	*/

	for(auto nc : n->getConnectors())
	{
		for(auto nconn : nc->getConnections())
		{
			if(nodeConnections.count(nconn.second) > 0)
				nodeConnections.erase(nconn.second);
		}
	}
	
	nodes.erase(n);
	delete n;
}

const std::unordered_map<Node*, Point2f>* NodeGraph::getNodes()
{
	return &nodes;
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

unsigned int NodeGraph::numNodes()
{
	return nodes.size();
}


void NodeGraph::reset()
{
	for(auto &n : nodes)
		//AU::safeDelete(n.first);
		delete n.first;
	nodes.clear();

	nodeConnections.clear();

}

void NodeGraph::update(double dt)
{
	for(auto n : nodes)
		n.first->update(dt);
	//for(unsigned int i = 0; i < nodes.size(); i++)
	//	nodes[i]->update(dt);
}

void NodeGraph::setPreset(NodeGraphPreset preset, int sample_rate)
{
	AStatus status;

	MidiTrackNode	*mainMidiBuf;
	InstrumentNode	*instrument1,
					*instrument2;
	RenderNode		*mainRender;
	AudioTrackNode *mainAudioBuf,
					*audioBuf2;
	SpeakerNode		*mainSpeaker;

	MidiDeviceNode	*mainDevice;

	//TimeMapNode		*timeMap;

	RenderNode		*render2;
	
	MidiData notes;	//For testing

	
	reset();
	//initialPos = &presetPositions[toIndex(preset)];


	switch(preset)
	{
	//Empty graph (do nothing)
	case NodeGraphPreset::EMPTY:
		break;

	//Initial NodeGraph for a new sandbox
	case NodeGraphPreset::SANDBOX:
		//Preset Nodes
		instrument1 = new InstrumentNode();
		mainRender = new RenderNode(sample_rate);
		mainSpeaker = new SpeakerNode(0, sample_rate, SPEAKER_BUFFER_SIZE, nullptr);
		mainDevice = new MidiDeviceNode(midi_port);	//TODO: Choose port with gui
		
		//Add nodes to list
		addNode(mainDevice, Point2f(-200.0f, -25.0f));
		addNode(mainRender, Point2f(-50.0f, -25.0f));
		addNode(instrument1, Point2f(-50.0f, -175.0f));
		addNode(mainSpeaker, Point2f(100.0f, -25.0f));

		//Preset Connections
		//Node::connect(mainDevice->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		//Node::connect(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		//Node::connect(mainRender->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);

		mainDevice->connect(mainDevice->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		instrument1->connect(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		mainRender->connect(mainRender->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		
		break;

	//Initial NodeGraph for a new project
	case NodeGraphPreset::PROJECT:
		//Preset Nodes
		mainMidiBuf = new MidiTrackNode();
		instrument1 = new InstrumentNode();
		mainRender = new RenderNode(sample_rate);
		mainAudioBuf = new AudioTrackNode(sample_rate, 1000);
		mainSpeaker = new SpeakerNode(0, sample_rate, SPEAKER_BUFFER_SIZE, nullptr);

		//timeMap = new TimeMapNode(sample_rate);

		mainDevice = new MidiDeviceNode(midi_port);
		instrument2 = new InstrumentNode();
		render2 = new RenderNode(sample_rate);
		//audioBuf2 = new AudioTrackNode(sample_rate);
		
		//Add nodes to list
		addNode(mainMidiBuf, Point2f(-300.0f, -25.0f));
		addNode(instrument1, Point2f(-137.5f, -175.0f));
		addNode(mainRender, Point2f(-150.0f, -75.0f));
		addNode(mainAudioBuf, Point2f(-25.0f, -75.0f));
		addNode(mainSpeaker, Point2f(250.0f, -25.0f));

		//addNode(timeMap, Point2f(100.0f, -25.0f));

		addNode(mainDevice, Point2f(-300.0f, 150.0f));
		addNode(instrument2, Point2f(-137.5f, 25.0f));
		addNode(render2, Point2f(-150.0f, 150.0f));
		//addNode(audioBuf2, Point2f(-25.0f, 150.0f));


		//Preset Connections
		mainMidiBuf->connect(mainMidiBuf->OUTPUTS.MIDI_ID, mainRender->INPUTS.MIDI_ID);
		instrument1->connect(instrument1->OUTPUTS.INSTRUMENT_ID, mainRender->INPUTS.INSTRUMENT_ID);
		mainRender->connect(mainRender->OUTPUTS.AUDIO_ID, mainAudioBuf->INPUTS.AUDIO_ID);
		mainAudioBuf->connect(mainAudioBuf->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		//timeMap->connect(timeMap->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);
		
		mainDevice->connect(mainDevice->OUTPUTS.MIDI_ID, render2->INPUTS.MIDI_ID);
		instrument2->connect(instrument2->OUTPUTS.INSTRUMENT_ID, render2->INPUTS.INSTRUMENT_ID);
		render2->connect(render2->OUTPUTS.AUDIO_ID, mainSpeaker->INPUTS.AUDIO_ID);


		////ADD TEST NOTES (Legend of Zelda Main Theme)////
		//legendOfZeldaTheme1.interpret(notes, 0.0, 30);
		legendOfZeldaTheme2.interpret(notes, 0.0, 30, 100);
		//notes.addNote(MidiNote(-1, 0, 0.0, 100.0));	//100-second long rest (empty)
		mainMidiBuf->setBuffer(notes);
		mainAudioBuf->setLength((c_time)(mainMidiBuf->getData()->getSpan().length()*(float)sample_rate/(float)AUDIO_CHUNK_SIZE));

		break;

	default:	//NODEGRAPH_PRESET_INVALID
		//Error
		break;
	}
}