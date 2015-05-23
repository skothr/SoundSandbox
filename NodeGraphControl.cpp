#include "NodeGraphControl.h"

#include "NodeGraph.h"
#include "PropertiesDisplay.h"
#include "NodeControl.h"
#include "NodeConnectionControl.h"
#include "Node.h"

#include <algorithm>

/////NODE GRAPH CONTROL/////
const GuiPropFlags		NodeGraphControl::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;

const APoint			NodeGraphControl::GRAPH_MIN(-10000.0f, -10000.0f),
						NodeGraphControl::GRAPH_MAX(10000.0f, 10000.0f);
const AVec				NodeGraphControl::UNIT_SIZE(50.0f, 50.0f),
						NodeGraphControl::ZOOM_STEP(0.01f, 0.01f);

NodeGraphControl::NodeGraphControl(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, NodeGraph *node_graph)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		NodeElementContainer(s_flags, GRAPH_MIN, GRAPH_MAX, UNIT_SIZE, ZOOM_STEP),
		nodeGraph(node_graph)
{
	updateGraph();
}

NodeGraphControl::~NodeGraphControl()
{ }

void NodeGraphControl::updateGraph()
{
	if(nodeGraph)
	{
		const std::unordered_map<Node*, Point2f> *g_nodes = nodeGraph->getNodes();
		
		std::unordered_set<Node*> removed;

		for(auto n : nodeControls)
		{
			if(g_nodes->count(n.first) == 0)
			{
				//Get rid of node
				removed.emplace(n.first);

				//Get rid of any related connections
				std::unordered_set<NodeConnectionControl*> removed_c;
				std::vector<NodeConnector*> n_connectors = n.first->getConnectors();

				for(auto nc : n_connectors)
				{
					CMap n_connections = nc->getConnections();
					for(auto ncc : n_connections)
					{
						for(auto c : connections)
						{
							if(ncc.second == c->NC)
								removed_c.emplace(c);
						}
					}
				}
				
				for(auto nc : removed_c)
				{
					nodeGraph->removeConnection(nc->NC);
					removeChild(nc, false);
					connections.erase(nc);
					delete nc;
				}
			}
		}

		for(auto n : removed)
		{
			NodeControl *nc = nodeControls[n];

			removeNode(nc);
			nodeControls.erase(n);
			//delete nc;
		}

		//Add new nodes
		std::unordered_map<Node*, NodeControl*> added;
		for(auto n : *g_nodes)
		{
			if(nodeControls.count(n.first) == 0)
			{
				//Set up node
				NodeControl *nc = new NodeControl(this, n.second, DEFAULT_STATE, n.first);
				added.emplace(n.first, nc);
			}
		}
		
		//nodeControls.insert(added.begin(), added.end());
		for(auto nc : added)
		{
			addNode(nc.second);
			nodeControls.emplace(nc.first, nc.second);
		}
		
		for(auto n : *g_nodes)
		{
			for(auto con : n.first->getConnectors())
			{
				for(auto ncon : con->getConnections())
				{
					bool add = true;
					for(auto c : connections)
					{
						add = c->NC != ncon.second;
						if(!add)
							break;
					}

					if(add)
						connections.emplace(new NodeConnectionControl(this, DEFAULT_STATE, ncon.second));

				}
			}
		}

	}
}

void NodeGraphControl::onSizeChanged(AVec d_size)
{
	setMinViewOffset(GRAPH_MIN);
	setMaxViewOffset(GRAPH_MAX + transform.absoluteToVirtualVec(size));
	NodeElementContainer::onSizeChanged(d_size);
}


void NodeGraphControl::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(movingConnection)
		movingConnection->setHangingPos(transform.absoluteToVirtualPoint(m_pos));

	NodeElementContainer::onMouseMove(m_pos, d_pos, direct);
}

void NodeGraphControl::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	NodeElementContainer::onMouseUp(m_pos, b, direct);

	if(direct && movingConnection)
	{
		std::cout << "NODE CONNECTING WAS INTERRUPTED.\n";
		
		connections.erase(movingConnection);
		removeChild(movingConnection, false);
		delete movingConnection;
		movingConnection = nullptr;
		
		for(auto nc : nodeControls)
			nc.second->setHighlight(IOType::INVALID, NodeDataType::INVALID);
	}
}

void NodeGraphControl::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	if(movingConnection)
		movingConnection->setHangingPos(m_pos);

	NodeElementContainer::onDrag(m_pos, d_pos, direct);

}

void NodeGraphControl::onActiveNodeChanged(NodeElement *e)
{
	if(propDisp)
	{
		if(e)
		{
			NodeControl *nc = dynamic_cast<NodeControl*>(e);
			if(nc)
				propDisp->setActiveNode(nc->getNode());
		}
		else
			propDisp->setActiveNode(nullptr);
	}
}

void NodeGraphControl::setPropDisplay(PropertiesDisplay *prop_disp)
{
	propDisp = prop_disp;
}

void NodeGraphControl::setGraph(NodeGraph *node_graph)
{
	nodeGraph = node_graph;
	updateGraph();
}


void NodeGraphControl::startConnect(NodeControl *nc, NCID nc_id)
{
	if(!movingConnection)
	{
		NodeConnector *nc = NodeConnector::getNC(nc_id);
		IOType io_type = nc->ioType;
		bool from_set = (io_type == IOType::DATA_OUTPUT || io_type == IOType::INFO_OUTPUT);

		movingConnection = new NodeConnectionControl(this, DEFAULT_STATE, COwnedPtr(new NodeConnection(from_set ? nc_id : -1, from_set ? -1 : nc_id)));
		
		connections.emplace(movingConnection);
		updateGraph();

		IOType			desired_io = getOpposite(nc->ioType);
		NodeDataType	desired_dt = nc->getDataType();

		for(auto nc : nodeControls)
			nc.second->setHighlight(desired_io, desired_dt);
	}
}

void NodeGraphControl::startConnect(NodeConnectorControl *ncc)
{
	if(!movingConnection)
	{
		IOType io_type = ncc->getIoType();
		NCID nc_id = ncc->getId();
		bool from_set = (io_type == IOType::DATA_OUTPUT || io_type == IOType::INFO_OUTPUT);

		movingConnection = new NodeConnectionControl(this, DEFAULT_STATE, COwnedPtr(new NodeConnection(from_set ? nc_id : -1, from_set ? -1 : nc_id)));
		
		connections.emplace(movingConnection);
	
		updateGraph();
		
		IOType			desired_io = getOpposite(io_type);
		NodeDataType	desired_dt = ncc->getDataType();

		for(auto nc : nodeControls)
			nc.second->setHighlight(desired_io, desired_dt);
	}
}


void NodeGraphControl::finishConnect(NodeControl *nc, NCID nc_id)
{
	if(movingConnection)
	{
		NCID old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId);

		removeChild(movingConnection, false);
		nodeGraph->removeConnection(movingConnection->NC);
		delete movingConnection;
		movingConnection = nullptr;

		NodeConnector::getNC(old_ncid)->connect(nc_id);
		updateGraph();
		
		for(auto nc : nodeControls)
			nc.second->setHighlight(IOType::INVALID, NodeDataType::INVALID);
	}
}

void NodeGraphControl::finishConnect(NodeConnectorControl *ncc)
{
	if(movingConnection)
	{
		//NCType io_type = ncc->getIoType();
		//NCID	old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId),
		NCID		nc_id = ncc->getId(),
					old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId);

		removeChild(movingConnection, false);
		nodeGraph->removeConnection(movingConnection->NC);
		delete movingConnection;
		movingConnection = nullptr;

		NodeConnector::getNC(old_ncid)->connect(nc_id);
		updateGraph();

		for(auto nc : nodeControls)
			nc.second->setHighlight(IOType::INVALID, NodeDataType::INVALID);
	}
}


NodeControl* NodeGraphControl::getNodeControl(Node *node)
{
	return nodeControls[node];
}

