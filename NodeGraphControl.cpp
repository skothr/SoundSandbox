#include "NodeGraphControl.h"

#include "NodeGraph.h"
#include "PropertiesDisplay.h"
#include "NodeConnection.h"
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
{
	//for(auto c : connectionControls)
	//	if(c.second)
	//		delete c.second;
	connectionControls.clear();

	//for(auto nc : nodeControls)
	//	if(nc.second)
	//		delete nc.second;
	nodeControls.clear();
}

void NodeGraphControl::updateGraph()
{
	if(nodeGraph)
	{
		const std::unordered_map<NID, Node*>			g_nodes = nodeGraph->getNodes();
		const std::unordered_map<CID, NodeConnection*>	g_connections = nodeGraph->getConnections();

		//Find any Nodes that were deleted
		for(auto it = nodeControls.begin(); it != nodeControls.end(); )
		{
			if(g_nodes.count(it->first) == 0)
			{
				//Get rid of node
				removeNode(it->second.get());
				//delete it->first;
				it = nodeControls.erase(it);
			}
			else
				it++;
		}

		//Find any Connections that were deleted
		for(auto it = connectionControls.begin(); it != connectionControls.end(); )
		{
			if(g_connections.count(it->first) == 0)
			{
				//Get rid of connection
				removeChild(it->second.get(), false);
				it = connectionControls.erase(it);
			}
			else
				it++;
		}

		//Add new nodes
		for(const auto &n : g_nodes)
		{
			if(nodeControls.count(n.first) == 0)
			{
				NodeControl *nc = new NodeControl(this, n.second->graphPos, DEFAULT_STATE, n.second);
				auto &iter = nodeControls.emplace(n.first, std::unique_ptr<NodeControl>(nc));
				addNode(iter.first->second.get());
			}
		}
		
		//Add new connections
		for(const auto &c : g_connections)
		{
			if(connectionControls.count(c.first) == 0)
			{
				NodeConnectionControl *ncc = new NodeConnectionControl(this, DEFAULT_STATE, c.second);
				//c.second.
				addConnection(ncc);
			}
		}
	}
}

NodeConnectionControl* NodeGraphControl::addConnection(NodeConnectionControl *ncc)
{
	auto &iter = connectionControls.emplace(ncc->NC->getId(), std::unique_ptr<NodeConnectionControl>(ncc));
	//addChild(ncc, false);

	return iter.first->second.get();
}

void NodeGraphControl::removeConnection(NodeConnectionControl *ncc)
{
	removeChild(ncc, false);
	connectionControls.erase(ncc->NC->getId());
}


void NodeGraphControl::onSizeChanged(AVec d_size)
{
	setMinViewOffset(GRAPH_MIN);
	setMaxViewOffset(GRAPH_MAX + transform.absoluteToVirtualVec(size));
	NodeElementContainer::onSizeChanged(d_size);
}


void NodeGraphControl::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(movingConnection && direct)
		movingConnection->setHangingPos(transform.absoluteToVirtualPoint(m_pos));

	NodeElementContainer::onMouseMove(m_pos, d_pos, direct);
}

void NodeGraphControl::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	NodeElementContainer::onMouseUp(m_pos, b, direct);

	if(movingConnection)
	{
		std::cout << "NODE CONNECTING WAS INTERRUPTED.\n";
		
		nodeGraph->removeConnection(movingConnection->NC->getId());
		removeConnection(movingConnection);

		//delete movingConnection;
		movingConnection = nullptr;
		
		for(auto &nc : nodeControls)
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
		NodeControl *nc = dynamic_cast<NodeControl*>(e);
		propDisp->setActiveNode(nc ? nc->getNode() : nullptr);
	}
}

void NodeGraphControl::setPropDisplay(PropertiesDisplay *prop_disp)
{
	propDisp = prop_disp;
}

void NodeGraphControl::setGraph(NodeGraph *node_graph)
{
	//Completely different graph, so get rid of current controls
	//for(auto nc : nodeControls)
	//	if(nc.second)
	//		delete nc.second;
	nodeControls.clear();
	
	//for(auto c : connections)
	//	if(c.second)
	//		delete c.second;
	connectionControls.clear();

	//Load with new graph controls
	nodeGraph = node_graph;
	updateGraph();
}

/*
void NodeGraphControl::startDisconnect(NodeControl *nc, NCID nc_id)
{
	if(!movingConnection)
	{
		NodeConnector *nc = NodeConnector::getNC(nc_id);
	}
}

void NodeGraphControl::startDisconnect(NodeConnectorControl *ncc)
{


}
*/

void NodeGraphControl::startConnect(NCID nc_id)
{
	if(!movingConnection)
	{
		NodeConnector *c = NodeConnector::getNC(nc_id);
		IOType io_type = c->ioType;
		bool from_set = (io_type == IOType::DATA_OUTPUT || io_type == IOType::INFO_OUTPUT);


		//NodeConnection *new_ncc = from_set ? new NodeConnection(nc_id, -1) : new NodeConnection(-1, nc_id);
		NodeConnection *new_ncc = nodeGraph->makeNewConnection(nc_id);
		movingConnection = addConnection(new NodeConnectionControl(this, DEFAULT_STATE, new_ncc));
		
		//addConnection(movingConnection);
		//updateGraph();

		IOType			desired_io = getOpposite(c->ioType);
		NodeDataType	desired_dt = c->getDataType();

		for(auto &nc : nodeControls)
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
		
		//NodeConnection *new_ncc = from_set ? new NodeConnection(nc_id, -1) : new NodeConnection(-1, nc_id);
		NodeConnection *new_ncc = nodeGraph->makeNewConnection(nc_id);
		movingConnection = addConnection(new NodeConnectionControl(this, DEFAULT_STATE, new_ncc));

		//addConnection(movingConnection);
		//updateGraph();
		
		IOType			desired_io = getOpposite(io_type);
		NodeDataType	desired_dt = ncc->getDataType();

		for(auto &nc : nodeControls)
			nc.second->setHighlight(desired_io, desired_dt);
	}
}


void NodeGraphControl::finishConnect(NCID nc_id)
{
	if(movingConnection)
	{
		if(movingConnection->NC->connectTo(nc_id))
		{
			//Success
			std::cout << "SUCCESSFULLY CONNECTED NODES!\n";
			movingConnection->setHangingNode(nc_id);
		}
		else
		{
			//Invalid (Delete connection)
			std::cout << "INVALID NODE CONNECTION!\n";
			nodeGraph->removeConnection(movingConnection->NC->getId());
			removeConnection(movingConnection);
			
			//delete movingConnection->NC;
			//delete movingConnection;
		}

		//NCID old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId);

		//removeChild(movingConnection, false);
		//nodeGraph->removeConnection(movingConnection->NC);
		//delete movingConnection;
		movingConnection = nullptr;

		//NodeConnector::getNC(old_ncid)->connect(nc_id);
		//updateGraph();
		
		for(auto &n : nodeControls)
			n.second->setHighlight(IOType::INVALID, NodeDataType::INVALID);
	}
}

void NodeGraphControl::finishConnect(NodeConnectorControl *ncc)
{
	if(movingConnection)
	{
		if(movingConnection->NC->connectTo(ncc->getId()))
		{
			//Success
			std::cout << "SUCCESSFULLY CONNECTED NODES!\n";
			movingConnection->setHangingNode(ncc->getId());
		}
		else
		{
			//Invalid (Delete connection)
			std::cout << "INVALID NODE CONNECTION!\n";
			nodeGraph->removeConnection(movingConnection->NC->getId());
			removeConnection(movingConnection);

			//delete movingConnection->NC;
			//delete movingConnection;
		}

		//NCType io_type = ncc->getIoType();
		//NCID	old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId),
		//NCID		nc_id = ncc->getId(),
		//			old_ncid = max(movingConnection->NC->fromId, movingConnection->NC->toId);

		//removeChild(movingConnection, false);
		//nodeGraph->removeConnection(movingConnection->NC);
		//delete movingConnection;
		movingConnection = nullptr;

		//NodeConnector::getNC(old_ncid)->connect(nc_id);
		//updateGraph();

		for(auto &nc : nodeControls)
			nc.second->setHighlight(IOType::INVALID, NodeDataType::INVALID);
	}
}

NodeConnectionControl* NodeGraphControl::getMovingConnection()
{
	return movingConnection;
}

NodeControl& NodeGraphControl::getNodeControl(NID id)
{
	return *nodeControls[id].get();
}

