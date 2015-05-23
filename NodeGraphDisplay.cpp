#include "NodeGraphDisplay.h"

#include "AUtility.h"

#include "NodeGraph.h"

#include "PropertiesDisplay.h"
#include "Timing.h"
#include "Interpolation.h"

#include "RenderNode.h"
#include "TimeMapNode.h"
#include "TrackNodes.h"
#include "IONodes.h"

#include "Label.h"

#include <cfloat>


/////NODE GRAPH DISPLAY/////
const AVec			NodeGraphDisplay::BASE_GRAPH_STEP	= Vec2f(50.0f, 50.0f),
					NodeGraphDisplay::ZOOM_STEP			= AVec(0.01f, 0.01f);
const GuiPropFlags	NodeGraphDisplay::PROP_FLAGS		= PFlags::ABSORB_SCROLL | PFlags::HARD_BACK;

NodeGraphDisplay::NodeGraphDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, NodeGraph *node_graph)
	: GuiElement(parent_, a_pos, a_size, GuiProps(s_flags, PROP_FLAGS)),
		NodeElementContainer(s_flags, APoint(-10000.0f, -10000.0f), APoint(10000.0f, 10000.0f), BASE_GRAPH_STEP, ZOOM_STEP),
		nodeGraph(node_graph)
{
	initGraph();

	setAllBgStateColors(Color(0.10f, 0.10f, 0.10f, 1.0f));

	setMinZoom(AVec(0.3f, 0.3f));
	setMaxZoom(AVec(3.0f, 3.0f));
	setMinViewOffset(APoint(-FLT_MAX, -FLT_MAX));
	setMaxViewOffset(APoint(FLT_MAX, FLT_MAX));
	
	setViewOffset(-size*(1.0f/2.0f));
	setZoomPivot(size*(1.0f/2.0f));
}

NodeGraphDisplay::~NodeGraphDisplay()
{
	//Delete node controls
	for(auto nc : nodeControls)
		AU::safeDelete(nc);
	nodeControls.clear();

	//Delete connections
	for(auto c : connections)
		AU::safeDelete(c);
	connections.clear();
}


void NodeGraphDisplay::reset()
{
	//Delete node controls
	for(auto nc : nodeControls)
		AU::safeDelete(nc);
	nodeControls.clear();
	selectedControls.clear();

	//Delete connections
	for(auto c : connections)
		AU::safeDelete(c);
	connections.clear();

	//Clear other variables
	draggedNode = nullptr;
	draggedConnector = 0;
	connectionIndex = -1;

	//removeAllBody();
	clearChildren();

	//Re-init graph
	initGraph();
}



void NodeGraphDisplay::initGraph()
{
	if(nodeGraph)
	{
		/*
		std::vector<Node*> nodes = nodeGraph->getNodes();
		
		if(nodes.size > 0)
		{
			Vec2f	step(100.0f, 100.0f);
			Point2f max_pos(step.x,step.y),
					min_pos(0.0f, 0.0f);

			std::unordered_set<Node*> visited_nodes;
		
			initNodeControl(nodes[0], Point2f(0.0f, 0.0f), step, visited_nodes, max_pos, min_pos);
		}
		//else no nodes
		*/


		//Add node controls
		const std::vector<Node*> &nodes = nodeGraph->getNodes();
		const std::vector<Point2f> &initial_pos = nodeGraph->getInitialPos();

		nodeControls.reserve(nodes.size());
		
		//Create NodeControls
		for(unsigned int i = 0; i < nodes.size(); i++)
		{
			NodeControl *nc = new NodeControl(this, nodes[i], initial_pos[i]);
			nodeControls.push_back(nc);
			addBody(nc);
		}

		//Create NodeConnectionControls
		for(unsigned int i = 0; i < nodeControls.size(); i++)
		{
			//Connections
			for(auto from : nodes[i]->getConnectors())
			{
				//Only make output connections (to avoid duplicates)
				if(from->ioType == NCType::DATA_OUTPUT || from->ioType == NCType::INFO_OUTPUT)
				{
					for(NodeConnector::CIndex i = 0; i < from->numConnections(); i++)
						addConnection(from->getId(), from->getConnection(i)->getId());
				}
			}
		}
	}
}


/*
void NodeGraphDisplay::initNodeControl(Node *n, Point2f pos, const Vec2f &step, std::unordered_set<Node*> &visited_nodes, Point2f &max_pos, Point2f &min_pos)
{
	auto n_iter = visited_nodes.find(n);

	if(n_iter == visited_nodes.end())
	{
		//Create new NodeControl
		NodeControl *nc = new NodeControl(this, n, pos);
		nodeControls.push_back(nc);
		addBody(nc);

		//TODO: recurse

	}
	//else node has already been traversed
}
*/

void NodeGraphDisplay::addNode(Node *n, Point2f graph_pos)
{
	NodeControl *nc = new NodeControl(this, n, graph_pos);
	nodeControls.push_back(nc);
	addBody(nc);
}

void NodeGraphDisplay::addConnection(NodeControl *from_nc, NodeControl *to_nc, NCID from_id, NCID to_id)
{
	NodeConnectionControl *ncc = new NodeConnectionControl(this, from_nc, to_nc, from_id, to_id);
	connections.push_back(ncc);
	addBody(ncc);
}

void NodeGraphDisplay::addConnection(NCID from_id, NCID to_id)
{
	NodeConnector	*from_nc = NodeConnector::getNodeConnector(from_id),
					*to_nc = NodeConnector::getNodeConnector(to_id);

	addConnection(getNodeControl(from_nc->getNode()), getNodeControl(to_nc->getNode()), from_id, to_id);
}

void NodeGraphDisplay::removeConnection(int index)
{
	NodeConnectionControl *connection = connections[index];

	removeBody(connection);
	delete connection;
	connections.erase(connections.begin() + index);
}

void NodeGraphDisplay::clearSelected()
{
	for(auto c : selectedControls)
		c->selected = false;
	selectedControls.clear();

	setActiveNode(nullptr);
}

void NodeGraphDisplay::selectNode(NodeControl *nc, bool set_active)
{
	if(!nc->selected)
	{
		nc->selected = true;
		selectedControls.push_back(nc);
	}

	if(set_active)
		setActiveNode(nc);
}

void NodeGraphDisplay::deselectNode(NodeControl *nc)
{
	if(nc->selected)
	{
		for(unsigned int i = 0; i < selectedControls.size(); i++)
		{
			if(selectedControls[i] == nc)
			{
				selectedControls.erase(selectedControls.begin() + i);
				break;
			}
		}
		nc->selected = false;

		if(propDisp && propDisp->getActiveNode() == nc->getNode())
			setActiveNode(chooseActiveNode());
	}
}

void NodeGraphDisplay::selectNodeToggle(NodeControl *nc)
{
	if(nc->selected)
		deselectNode(nc);
	else
		selectNode(nc, (selectedControls.size() > 0));
}

bool NodeGraphDisplay::stickySelect()
{
	return Keyboard::keyDown(Keys::K_LSHIFT) || Keyboard::keyDown(Keys::K_RSHIFT);
}

bool NodeGraphDisplay::toggleStickySelect()
{
	return Keyboard::keyDown(Keys::K_LCTRL) || Keyboard::keyDown(Keys::K_RCTRL);
}



NodeControl* NodeGraphDisplay::chooseActiveNode()
{
	if(selectedControls.size() > 0)
		return selectedControls[selectedControls.size() - 1];
	else
		return nullptr;
}


void NodeGraphDisplay::setActiveNode(NodeControl *nc)
{
	//Set focus
	for(auto c : nodeControls)
		c->setFocus(c == nc);

	//Set property display active node
	if(propDisp)
	{
		if(nc)
			propDisp->setActiveNode(nc->getNode());
		else
			propDisp->setActiveNode(nullptr);
	}
}



int NodeGraphDisplay::getNodeIndex(Node *node_ptr)
{
	std::vector<Node*> nodes = nodeGraph->getNodes();
	for(unsigned int i = 0; i < nodes.size(); i++)
	{
		if(node_ptr == nodes[i])
			return (int)i;
	}

	return -1;
}

bool NodeGraphDisplay::nodeIsVisible(unsigned int index)
{
	return (index < nodeControls.size()) && nodeControls[index]->isVisible();
}

bool NodeGraphDisplay::nodeIsVisible(Node *node_ptr)
{
	int index = getNodeIndex(node_ptr);
	return (index >= 0) && nodeIsVisible(index);
}


void NodeGraphDisplay::setPropDisp(PropertiesDisplay *prop_disp)
{
	propDisp = prop_disp;
}

PropertiesDisplay* NodeGraphDisplay::getPropDisp()
{
	return propDisp;
}


NodeControl* NodeGraphDisplay::getNodeControl(unsigned int index)
{
	return (index < nodeControls.size()) ? nodeControls[index] : nullptr;
}

NodeControl* NodeGraphDisplay::getNodeControl(Node *node_ptr)
{
	int index = getNodeIndex(node_ptr);
	return (index < nodeControls.size()) ? nodeControls[index] : nullptr;
}

void NodeGraphDisplay::startConnectingNodes(NodeControl *from_node, NCID *connector)
{
	if(!draggedNode)
	{
		std::cout << "STARTING CONNECTION...\n";

		draggedNode = from_node;

		NCID nc_id;
		NodeConnectionType type;

		if(connector && *connector)
		{
			draggedConnector = *connector;
			nc_id = draggedConnector;
			type = NodeConnector::getNodeConnector(draggedConnector)->ioType;
		}
		else
		{
			draggedConnector = 0;
			nc_id = from_node->getDefaultConnector()->getId();
			type = NodeConnector::getNodeConnector(nc_id)->ioType;
		}

		if(type == NodeConnectionType::DATA_INPUT || type == NodeConnectionType::INFO_INPUT)
			addConnection(nullptr, from_node, 0, nc_id);
		else
			addConnection(from_node, nullptr, nc_id, 0);

		connectionIndex = connections.size() - 1;
	}
}

void NodeGraphDisplay::finishConnectingNodes(NodeControl *to_node, NCID *connector)
{
	if(!draggedNode) return;

	std::cout << "TRYING TO COMPLETE CONNECTION...\n";

	Node *n_from, *n_to;
	NCID id_from, id_to;

	if(draggedConnector)
	{
		switch(NodeConnector::getNodeConnector(draggedConnector)->ioType)
		{
		case NCType::DATA_OUTPUT:
		case NCType::INFO_OUTPUT:
			n_from = draggedNode->getNode();
			n_to = to_node->getNode();

			id_from = draggedConnector;
			id_to = (connector ? *connector : 0);
			break;
		case NCType::DATA_INPUT:
		case NCType::INFO_INPUT:
				n_from = to_node->getNode();
				n_to = draggedNode->getNode();
			
				id_from = (connector ? *connector : 0);
				id_to = draggedConnector;
				break;
		default:
			return;
		}
	}
	else
	{
		n_from = draggedNode->getNode();
		n_to = to_node->getNode();
		
		id_from = draggedConnector;
		id_to = (connector ? *connector : 0);
	}
	
	std::vector<NodeConnector*> c_from = n_from->getConnectors(),
								c_to = n_to->getConnectors();
	
	NodeConnector	*nc_from = nullptr,
					*nc_to = nullptr;

	if(id_from)
	{
		nc_from = NodeConnector::getNodeConnector(id_from);

		if(id_to)
		{
			NodeConnector *nc = NodeConnector::getNodeConnector(id_to);
			if(statusGood(nc_from->connect(nc)))
				nc_to = nc;
		}
		else
		{
			for(unsigned int i = 0; i < c_to.size(); i++)
			{
				if(statusGood(nc_from->connect(c_to[i])))
				{
					nc_to = c_to[i];
					break;
				}
			}
		}
	}
	else
	{
		if(id_to)
		{
			nc_to = NodeConnector::getNodeConnector(id_to);
			for(unsigned int i = 0; i < c_from.size(); i++)
			{
				if(statusGood(c_from[i]->connect(nc_to)))
				{
					nc_from = c_from[i];
					break;
				}
			}
		}
		else
		{
			//Find best fit for connection between these connectors
			for(unsigned int i1 = 0; !nc_from && i1 < c_from.size(); i1++)
			{
				for(unsigned int i2 = 0; i2 < c_to.size(); i2++)
				{
					if(statusGood(c_from[i1]->connect(c_to[i2])))
					{
						nc_from = c_from[i1];
						nc_to = c_to[i2];
						break;
					}
				}
			}
		}
	}

	if(nc_from && nc_to)
	{
		std::cout << "Connected Node \"" << n_from->getName() << "\" to Node \"" << n_to->getName() << "\".\n";
		std::cout << "\t(Data type = " << nc_from->dataType << ")\n";
		
		clearSelected();
		
		if(!connections[connectionIndex]->hasFrom())
		{
			NodeControl *from_node = getNodeControl(n_from);
			connections[connectionIndex]->setOtherNode(from_node, nc_from);
			selectNode(from_node, true);
		}
		else
		{
			connections[connectionIndex]->setOtherNode(to_node, nc_to);
			selectNode(to_node, true);
		}
	}
	else
	{
		std::cout << "ERROR --> Could not connect Node \"" << n_from->getName() << "\" to Node \"" << n_to->getName() << "\".\n";
		removeConnection(connectionIndex);
		//clearSelected();
	}

	draggedNode = nullptr;
	draggedConnector = 0;
	connectionIndex = -1;
}

void NodeGraphDisplay::restartConnectingNodes(NodeConnectionControl *connection, NodeControl *connected_node, NCID connector)
{
	if(!draggedNode)
	{
		std::cout << "RESTARTING CONNECTION...\n";

		draggedNode = connected_node;
		draggedConnector = connector;
		for(unsigned int i = 0; i < connections.size(); i++)
		{
			if(connections[i] == connection)
			{
				connectionIndex = i;
				break;
			}
		}
	}
}

/*
void NodeGraphDisplay::moveSelectedNodes(AVec d_pos)
{
	for(auto c : selectedControls)
		c->move(d_pos);
}
*/

void NodeGraphDisplay::onControlClick(NodeControl *nc)
{
	if(stickySelect())
		selectNode(nc, true);
	else if(toggleStickySelect())
		selectNodeToggle(nc);
	else
	{
		clearSelected();
		selectNode(nc, true);
	}
}

void NodeGraphDisplay::onControlDrag(NodeControl *nc, APoint m_pos)
{
	if(draggedNode)
	{
		APoint c_pos = m_pos;

		if(draggedNode != nc)
		{
			if(draggedConnector)
				c_pos = nc->getConnectorPoint(getOpposite(NodeConnector::getNodeConnector(draggedConnector)->ioType));
			else
				c_pos = nc->getConnectorPoint(getOpposite(draggedNode->getDefaultConnector()->ioType));
		}
		
		connections[connectionIndex]->setHangingPos(c_pos);
	}
}

void NodeGraphDisplay::onSizeChanged(AVec d_size)
{
	moveViewOffset(absoluteToVirtualVec(d_size*(-1.0f/2.0f)));
	setZoomPivot(size*(1.0f/2.0f));
}

void NodeGraphDisplay::onScroll(APoint m_pos, Vec2f d_scroll)
{
	RVec a_zoom = getZoom();
	AVec mult = AVec(1.0f, 1.0f) - ZOOM_STEP*(d_scroll.x + d_scroll.y);
	setZoom(RVec(a_zoom.x*mult.x, a_zoom.y*mult.y));
	//moveZoom(-ZOOM_STEP*(d_scroll.x + d_scroll.y));
}

void NodeGraphDisplay::onMouseDown(APoint m_pos, MouseButton b)
{
	if(valid(b & NODEGRAPH_SELECT_NODES_BTN))
	{
		if(!stickySelect() && !toggleStickySelect())
			clearSelected();

		selectionPivot = m_pos;
		selectionRect.pos = selectionPivot;
		selectionRect.size = AVec(0, 0);
		//selectionRect.space = TSpace::O_VIRTUAL;
	}
}

void NodeGraphDisplay::onMouseUp(APoint m_pos, MouseButton b)
{
	if(pointInside(m_pos))
	{
		if(draggedNode)// && valid(b & NODEGRAPH_CONNECT_NODES_BTN))
		{
			std::cout << "NODE CONNECTING WAS INTERRUPTED!\n";	//Except not really...

			draggedNode = nullptr;
			draggedConnector = 0;

			removeConnection(connectionIndex);
			connectionIndex = -1;
		}
		else if(selectionRect.space != TSpace::INVALID && valid(b & NODEGRAPH_SELECT_NODES_BTN))
		{
			/*
			if(selectedControls.size() > 0)
			{
				//Choose which selected control is in focus
				//selectedControls[selectedControls.size() - 1]->setFocus(true);
				if(propDisp)
					propDisp->setActiveNode(chooseActiveNode());
			}
			else if(propDisp)
			{
				//No selected controls
				propDisp->setActiveNode(nullptr);
			}
			*/
		
			setActiveNode(chooseActiveNode());

			selectionRect.space = TSpace::INVALID;
		}
	}
}

void NodeGraphDisplay::onMouseMove(APoint m_pos, AVec d_pos)
{
	if(draggedNode)
	{
		connections[connectionIndex]->setHangingPos(m_pos);
	}
}

void NodeGraphDisplay::onDrag(APoint m_pos, AVec d_pos)
{
	if(valid(Mouse::getButtonStates() & NODEGRAPH_MOVE_OFFSET_BTN))
	{
		moveViewOffset(-absoluteToVirtualVec(d_pos));
	}
	else if(selectionRect.space != TSpace::INVALID)
	{
		//Adjust selectionRect
		if(selectionPivot.x > m_pos.x)
		{
			selectionRect.pos.x = m_pos.x;
			selectionRect.size.x = selectionPivot.x - m_pos.x;
		}
		else
		{
			selectionRect.pos.x = selectionPivot.x;
			selectionRect.size.x = m_pos.x - selectionPivot.x;
		}

		if(selectionPivot.y > m_pos.y)
		{
			selectionRect.pos.y = m_pos.y;
			selectionRect.size.y = selectionPivot.y - m_pos.y;
		}
		else
		{
			selectionRect.pos.y = selectionPivot.y;
			selectionRect.size.y = m_pos.y - selectionPivot.y;
		}

		//Clamp selection rect within absolute bounds
		Rect bounds = Rect(APoint(0, 0), size);
		selectionRect = Rect::findIntersection(selectionRect, absoluteToVirtualRect(bounds));
		
		if(!stickySelect())
			clearSelected();

		for(auto c : nodeControls)
		{
			if((c->selected && stickySelect()) || Rect::intersects(c->getRect(), selectionRect))
				selectNode(c, false);
		}
	}
}

bool NodeGraphDisplay::respondToClick(APoint m_pos, MouseButton b)
{
	return valid(b & (NODEGRAPH_CONNECT_NODES_NODE_BTN | NODEGRAPH_CONNECT_NODES_CONNECTOR_BTN
						| NODEGRAPH_SELECT_NODES_BTN | NODEGRAPH_MOVE_OFFSET_BTN));
}


void NodeGraphDisplay::drawGraph(GlInterface &gl)
{
	AVec	graph_step = virtualToAbsoluteVec(BASE_GRAPH_STEP),
			inv_graph_step = AVec(1.0f/graph_step.x, 1.0f/graph_step.y),
			dist_to_origin = virtualToAbsolutePoint(APoint(0, 0)),
			dist_after_origin = size - dist_to_origin;
	
	APoint	first_offset = APoint(fmod(dist_to_origin.x, graph_step.x), fmod(dist_to_origin.y, graph_step.y)),
			last_offset = APoint(fmod(dist_after_origin.x, graph_step.x), fmod(dist_after_origin.y, graph_step.y));
	Point2i lines_before_origin = Point2i(floor(dist_to_origin.x*inv_graph_step.x), floor(dist_to_origin.y*inv_graph_step.y)),
			lines_after_origin = Point2i(floor(dist_after_origin.x*inv_graph_step.x), floor(dist_after_origin.y*inv_graph_step.y)),
			num_lines = lines_before_origin + Vec2i(1, 1) + lines_after_origin;

	std::vector<TVertex> points;
	points.reserve(2*(num_lines.x + num_lines.y));

	//Add vertical lines
	int i = 0;
	for(float x = first_offset.x; x < size.x; x += graph_step.x, i++)
	{
		Color c = (i != lines_before_origin.x ? Color(0.0f, 0.0f, 0.0f, 1.0f) : Color(1.0f, 0.0f, 0.0f, 1.0f));
		points.push_back(TVertex(APoint(x, 0.0f), c));
		points.push_back(TVertex(APoint(x, size.y), c));
	}

	//Add horizontal lines
	i = 0;
	for(float y = first_offset.y; y < size.y; y += graph_step.y, i++)
	{
		Color c = (i != lines_before_origin.y ? Color(0.0f, 0.0f, 0.0f, 1.0f) : Color(1.0f, 0.0f, 0.0f, 1.0f));
		points.push_back(TVertex(APoint(0.0f, y), c));
		points.push_back(TVertex(APoint(size.x, y), c));
	}

	//Draw lines
	gl.drawShape(GL_LINES, points);
}

void NodeGraphDisplay::update(double dt)
{
	ParentElement::update(dt);

	for(auto c : connections)
		c->update(dt);
}

void NodeGraphDisplay::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		//Draw background
		drawBackground(gl);
		//Draw graph behind nodes
		drawGraph(gl);

		//Draw nodes
		drawChildren(gl);

		transformGl(gl);

		//Draw selection rect
		if(selectionRect.space != TSpace::INVALID)
		{
			std::vector<TVertex> points;
			points.reserve(5);

			Color col(0.0f, 1.0f, 1.0f, 1.0f);

			APoint	p1 = selectionRect.pos,
					p2 = p1 + selectionRect.size;

			points.push_back(TVertex(p1, col));
			points.push_back(TVertex(APoint(p1.x, p2.y), col));
			points.push_back(TVertex(p2, col));
			points.push_back(TVertex(APoint(p2.x, p1.y), col));
			points.push_back(TVertex(p1, col));

			//Draw rect
			gl.drawShape(GL_LINE_STRIP, points);
		}

		untransformGl(gl);
		restoreViewport(gl);
	}
}



/////NODE CONTROL/////
const GuiPropFlags NodeControl::PROP_FLAGS = PFlags::HARD_BACK;
const GuiStateFlags NodeControl::STATE_FLAGS = DEFAULT_STATE;

const AVec	NodeControl::BRANCH_SIZE = AVec(100.0f, 50.0f),
			NodeControl::LEAF_SIZE = AVec(75.0f, 50.0f),
			NodeControl::DEFAULT_SIZE = NodeControl::BRANCH_SIZE;

NodeControl::NodeControl(NodeGraphDisplay *parent_, APoint graph_pos, Node *node_)
	: GuiElement(parent_, graph_pos, AVec(), GuiProps(STATE_FLAGS, PROP_FLAGS)),
		MovableElement(STATE_FLAGS),
		ParentElement(),
		ngd_parent(dynamic_cast<NodeGraphDisplay*>(parent_)), node(node_)
{
	//Initialize connectors
	for(unsigned int i = 0; i < toIndex(NCType::COUNT); i++)
		connectorControls[i] = nullptr;

	initControl();
}

NodeControl::~NodeControl()
{
	AU::safeDelete(nodeName);
}


void NodeControl::initControl()
{
	setSize(getNodeSize());

	if(node)
	{
		std::vector<GuiElement*> children;
		//Label (node name)
		nodeName = new Label(this, APoint(), DEFAULT_STATE_FLOAT, node->getName(), 12);
		nodeName->centerAround(size*(1.0f/2.0f));
		children.push_back(nodeName);

		bool hasConnector[static_cast<unsigned int>(NCType::COUNT)] {false};

		std::vector<NodeConnector*> c = node->getConnectors();

		for(auto con : c)
			hasConnector[toIndex(con->ioType)] = true;

		//Create connectors
		for(unsigned int i = 0; i < toIndex(NCType::COUNT); i++)
		{
			if(hasConnector[i])
			{
				connectorControls[i] = new NodeConnectorControl(this, static_cast<NCType>(i));
				//children.push_back(connectorControls[i]);
			}
		}
		
		//init(&children, nullptr);
	}
	else
	{
		std::cout << "ERRORRRR --> NodeControl's node is null!\n";
	}
	
	//TODO: Separate this for each node type
	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(0.05f, 0.05f, 0.05f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(0.05f, 0.05f, 0.05f, 1.0f), CS::DRAGGING);
}

NodeGraphDisplay* NodeControl::getNgdParent()
{
	return ngd_parent;
}

Node* NodeControl::getNode()
{
	return node;
}

Vec2f NodeControl::getNodeSize()
{
	if(node)
	{
		switch(node->getType())
		{
		case NType::AUDIO_TRACK:
		case NType::MIDI_TRACK:
		case NType::AUDIO_MOD_TRACK:
		case NType::MIDI_MOD_TRACK:
			return BRANCH_SIZE;
		case NType::RENDER:
		case NType::TIME_MAP:
			return BRANCH_SIZE;
		case NType::SPEAKER:
		case NType::INSTRUMENT:
			return LEAF_SIZE;
		}
	}

	return DEFAULT_SIZE;
}

APoint NodeControl::getConnectorPoint(NodeConnectionType io_type)
{
	AVec half_size = size*(1.0f/2.0f);
	switch(io_type)
	{
	case NCType::DATA_INPUT:
		return pos + AVec(0.0f, half_size.y);
	case NCType::DATA_OUTPUT:
		return pos + AVec(size.x, half_size.y);

	case NCType::INFO_INPUT:
		return pos + AVec(half_size.x, 0.0f);
	case NCType::INFO_OUTPUT:
		return pos + AVec(half_size.x, size.y);

	default:
		return APoint(0, 0);
	}
}

NodeConnector* NodeControl::getDefaultConnector()
{
	if(node)
	{
		NCID id = 0;
		switch(node->getType())
		{
		case NType::AUDIO_TRACK:
			id = dynamic_cast<AudioTrackNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::MIDI_TRACK:
			id = dynamic_cast<MidiTrackNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::AUDIO_MOD_TRACK:
			id = dynamic_cast<AudioModTrackNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::MIDI_MOD_TRACK:
			id = dynamic_cast<MidiModTrackNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::RENDER:
			id = dynamic_cast<RenderNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::TIME_MAP:
			id = dynamic_cast<TimeMapNode*>(node)->OUTPUTS.AUDIO_ID;
			break;
		case NType::MIDI_DEVICE:
			id = dynamic_cast<MidiDeviceNode*>(node)->OUTPUTS.MIDI_ID;
			break;
		case NType::SPEAKER:
			id = dynamic_cast<SpeakerNode*>(node)->INPUTS.AUDIO_ID;
			break;
		case NType::INSTRUMENT:
			id = dynamic_cast<InstrumentNode*>(node)->OUTPUTS.INSTRUMENT_ID;
			break;
		}
		return NodeConnector::getNodeConnector(id);
	}
	else
		return nullptr;
}

void NodeControl::onMouseDown(APoint m_pos, MouseButton b)
{
	if(valid(b & NODEGRAPH_CONNECT_NODES_NODE_BTN))
	{
		ngd_parent->startConnectingNodes(this);
	}
	
	if(respondToClick(m_pos, b))
	{
		ngd_parent->onControlClick(this);
	}
}

void NodeControl::onMouseUp(APoint m_pos, MouseButton b)
{
	if(pointInside(m_pos))// && valid(b & NODEGRAPH_CONNECT_NODES_BTN))
		ngd_parent->finishConnectingNodes(this);
}

void NodeControl::onMouseMove(APoint m_pos, AVec d_pos)
{
	ngd_parent->onControlDrag(this, m_pos + pos);
}

void NodeControl::onDrag(APoint m_pos, AVec d_pos)
{
	APoint p_m_pos = m_pos + pos;

	if(valid(Mouse::getButtonStates() & NODEGRAPH_MOVE_NODES_BTN))
	{
		ngd_parent->moveSelectedNodes(d_pos);//ngd_parent->absoluteToVirtualVec(d_pos));
		//move(transformVec(d_pos, TSpace::O_VIRTUAL, TSpace::O_PARENT));
	}
	else if(pointInside(p_m_pos))
	{
		ngd_parent->onControlDrag(this, p_m_pos);
	}
}


bool NodeControl::respondToClick(APoint m_pos, MouseButton b)
{
	return valid(b & (NODEGRAPH_CONNECT_NODES_NODE_BTN | NODEGRAPH_MOVE_NODES_BTN | NODEGRAPH_SELECT_NODES_BTN));
}

void NodeControl::update(double dt)
{
	ParentElement::update(dt);
	ActiveElement::update(dt);
}

void NodeControl::drawBody(GlInterface &gl)
{
	drawBackground(gl);

	//Draw connectors underneath selection outline
	drawChildren(gl);

	if(selected)
	{
		Color col = (isInFocus() ? Color(0.3f, 1.0f, 0.3f, 1.0f) : Color(0.1f, 0.75f, 0.75f, 1.0f));

		std::vector<TVertex> points;
		points.reserve(5);

		points.push_back(TVertex(APoint(0, 0), col));
		points.push_back(TVertex(APoint(0, size.y), col));
		points.push_back(TVertex(size, col));
		points.push_back(TVertex(APoint(size.x, 0), col));
		points.push_back(TVertex(APoint(0, 0), col));

		gl.drawShape(GL_LINE_STRIP, points);
	}

	//TODO: rest of body design (title, description, colors, etc)
}

void NodeControl::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))// && ngd_parent->nodeIsVisible(node))
	{
		drawBody(gl);
		restoreViewport(gl);
	}
}




/////NODE CONNECTOR CONTROL/////

NodeConnectorControl::NodeConnectorControl(NodeControl *parent_, NCType io_type)
	: Control(parent_, APoint(), AVec(), true, false),
		nc_parent(parent_), ngd(parent_->getNgdParent()), ioType(io_type)
{
	//Set size
	setSize(getConnectorSize(ioType));

	APoint c_pos = nc_parent->getConnectorPoint(ioType) - nc_parent->getPos();
	AVec half_size = size*(1.0f/2.0f);

	//Set pos
	switch(ioType)
	{
	case NCType::DATA_INPUT:
		setPos(c_pos - AVec(0.0f, half_size.y));
		break;
	case NCType::DATA_OUTPUT:
		setPos(c_pos - AVec(size.x, half_size.y));
		break;

	case NCType::INFO_INPUT:
		setPos(c_pos - AVec(half_size.x, 0.0f));
		break;
	case NCType::INFO_OUTPUT:
		setPos(c_pos - AVec(half_size.x, size.y));
		break;

	default:
		setPos(c_pos - AVec(0.0f, half_size.y));
		break;
	}

	setBackgroundColor(ControlState::NONE, Color(0.75f, 0.2f, 0.2f, 1.0f));
	setBackgroundColor(ControlState::HOVERING, Color(0.9f, 0.3f, 0.3f, 1.0f));
	setBackgroundColor(ControlState::CLICKING, Color(1.0f, 0.5f, 0.5f, 1.0f));
	setBackgroundColor(ControlState::DRAGGING, Color(1.0f, 0.5f, 0.5f, 1.0f));
}

AVec NodeConnectorControl::getConnectorSize(NodeConnectionType io_type)
{
	switch(io_type)
	{
	case NCType::DATA_INPUT:
		return AVec(15.0f, 40.0f);
	case NCType::DATA_OUTPUT:
		return AVec(15.0f, 40.0f);

	case NCType::INFO_INPUT:
		return AVec(50.0f, 15.0f);
	case NCType::INFO_OUTPUT:
		return AVec(50.0f, 15.0f);

	default:
		return AVec(30.0f, 30.0f);
	}
}

void NodeConnectorControl::onMouseDown(APoint m_pos, MouseButton b)
{
	//TODO: Choose between which connector to connect (right click menu)
	std::vector<NodeConnector*> connectors = nc_parent->getNode()->getConnectors();

	NCID id = 0;

	for(unsigned int i = 0; i < connectors.size(); i++)
	{
		if(connectors[i]->ioType == ioType)
			id = connectors[i]->getId();
	}

	ngd->startConnectingNodes(nc_parent, &id);
}

void NodeConnectorControl::onMouseUp(APoint m_pos, MouseButton b)
{
	//TODO: Choose between which connector to connect (right click menu)
	if(pointInside(m_pos))
		ngd->finishConnectingNodes(nc_parent);
}

void NodeConnectorControl::onMouseMove(APoint m_pos, AVec d_pos)
{
	if(pointInside(m_pos))
		ngd->onControlDrag(nc_parent, nc_parent->virtualToAbsolutePoint(m_pos) + nc_parent->getPos());
}

void NodeConnectorControl::onDrag(APoint m_pos, AVec d_pos)
{
	if(pointInside(m_pos))
		ngd->onControlDrag(nc_parent, nc_parent->virtualToAbsolutePoint(m_pos) + nc_parent->getPos());
}

bool NodeConnectorControl::respondToClick(APoint m_pos, MouseButton b)
{
	return true;
}


void NodeConnectorControl::draw(GlInterface &gl)
{
	if(visible && isolateViewport(gl))
	{
		drawBackground(gl);
		restoreViewport(gl);
	}
}



/////NODE CONNECTION CONTROL/////

const double	NodeConnectionControl::TRANSITION_TIME = 0.5;
const float		NodeConnectionControl::ARROW_LENGTH = 7.0f,
				NodeConnectionControl::SIN_WAVELENGTH = 2.0f*M_PI,
				NodeConnectionControl::SIN_VELOCITY = -2.0f,
				NodeConnectionControl::SIN_FREQUENCY = NodeConnectionControl::SIN_VELOCITY/NodeConnectionControl::SIN_WAVELENGTH,
				NodeConnectionControl::SIN_K = 2.0f*M_PI/NodeConnectionControl::SIN_WAVELENGTH,
				NodeConnectionControl::SIN_OMEGA = 2.0f*M_PI*NodeConnectionControl::SIN_FREQUENCY;
				//NodeConnectionControl::SPRING_K = 8.0f,
				//NodeConnectionControl::SPRING_B = 10.0f,
				//NodeConnectionControl::SPRING_M = 5.0f;

const Color NodeConnectionControl::inactiveColor(1.0f, 1.0f, 1.0f, 1.0f),
			NodeConnectionControl::forwardActiveColor(0.0f, 1.5f, 0.0f, 1.0f),
			NodeConnectionControl::backwardActiveColor(1.5f, 0.0f, 1.5f, 1.0f),
			NodeConnectionControl::arrowColor(1.5f, 1.5f, 0.0f, 1.0f);

NodeConnectionControl::NodeConnectionControl(NodeGraphDisplay *parent_, NodeControl *from_node, NodeControl *to_node, NCID from_id, NCID to_id)
	: Control(parent_, APoint(), AVec(), true, false),
		ngd_parent(parent_), //baseIoType(NodeConnector::getNodeConnector(from_id)->ioType),//baseNc(NodeConnector::getNodeConnector(from_id)),
		fromNode(from_node), toNode(to_node),
		fromNc(NodeConnector::getNodeConnector(from_id)), toNc(NodeConnector::getNodeConnector(to_id))
{
	pFrom = (fromNode ? (fromNode->getConnectorPoint(fromNc->ioType)) : APoint(0, 0));
	pTo = (toNode ? (toNode->getConnectorPoint(toNc->ioType)) : pFrom);
	pFrom = (fromNode ? pFrom : pTo);

	adjustPos();
	
	//Used as multipliers for color
	setBackgroundColor(ControlState::NONE, Color(0.5f, 0.5f, 0.5f, 1.0f));
	setBackgroundColor(ControlState::HOVERING, Color(0.9f, 0.9f, 0.9f, 1.0f));
	setBackgroundColor(ControlState::CLICKING, Color(1.0f, 1.0f, 1.0f, 1.0f));
	setBackgroundColor(ControlState::DRAGGING, Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void NodeConnectionControl::adjustPos()
{
	if(pFrom.x > pTo.x)
	{
		setX(pTo.x - width);
		setWidth(pFrom.x - pTo.x + 2.0f*width);
	}
	else
	{
		setX(pFrom.x - width);
		setWidth(pTo.x - pFrom.x + 2.0f*width);
	}
	
	if(pFrom.y > pTo.y)
	{
		setY(pTo.y - width);
		setHeight(pFrom.y - pTo.y + 2.0f*width);
	}
	else
	{
		setY(pFrom.y - width);
		setHeight(pTo.y - pFrom.y + 2.0f*width);
	}
}

float NodeConnectionControl::distToLine(APoint p)
{
	if(fromNode && toNode)
	{
		//Find distance to line
		AVec d_pos = pTo - pFrom;
		//return abs(d_pos.x*(pFrom.y - p.y) - d_pos.y*(pFrom.x - p.x))/sqrt(d_pos.x*d_pos.x + d_pos.y*d_pos.y);
		return abs(d_pos.y*p.x + pTo.x*pFrom.y - (d_pos.x*p.y + pFrom.x*pTo.y))/sqrt(d_pos.x*d_pos.x + d_pos.y*d_pos.y);
	}
	else
		return -1.0f;
}

void NodeConnectionControl::setOtherNode(NodeControl *other_node, NCID other_id)
{
	setOtherNode(other_node, NodeConnector::getNodeConnector(other_id));
}

void NodeConnectionControl::setOtherNode(NodeControl *other_node, NodeConnector *other_nc)
{
	if(fromNode && !toNode)
	{
		toNode = other_node;
		toNc = other_nc;
		pTo = other_node->getConnectorPoint(toNc->ioType);
	}
	else if(toNode && !fromNode)
	{
		fromNode = other_node;
		fromNc = other_nc;
		pFrom = other_node->getConnectorPoint(fromNc->ioType);
	}
	else
		std::cout << "ERROR: Node control set other node!\n";

	adjustPos();
}

bool NodeConnectionControl::hasFrom()
{
	return static_cast<bool>(fromNode);
}

bool NodeConnectionControl::hasTo()
{
	return static_cast<bool>(toNode);
}

void NodeConnectionControl::setHangingPos(APoint h_pos)
{
	if(fromNode && !toNode)
	{
		pTo = h_pos;
	}
	else if(toNode && !fromNode)
	{
		pFrom = h_pos;
	}

	adjustPos();
}

void NodeConnectionControl::onMouseDown(APoint m_pos, MouseButton b)
{

}

void NodeConnectionControl::onMouseUp(APoint m_pos, MouseButton b)
{

}

void NodeConnectionControl::onDrag(APoint m_pos, AVec d_pos)
{
	if(fromNode && toNode)
	{
		APoint m_pos_grid = m_pos + pos;
		float dist = distToLine(m_pos);
		
		//std::cout << dist << "\n";
		//std::cout << "(" << pTo.x << ", " << pTo.y << "), (" << pFrom.x << ", " << pFrom.y << "), (" << m_pos.x << ", " << m_pos.y << ")\n";
			
		
		//std::cout << dist << " (" << disconnectRadius << ")\n";

		if(dist > disconnectRadius || !pointInside(m_pos))
		{
			AVec	d_pos_from = pFrom - m_pos,
					d_pos_to = pTo - m_pos;
			float	to_dist_2 = d_pos_to.x*d_pos_to.x + d_pos_to.y*d_pos_to.y,
					from_dist_2 = d_pos_from.x*d_pos_from.x + d_pos_from.y*d_pos_from.y;
			
			AStatus status = fromNc->disconnect(toNc);

			if(!statusGood(status))
			{
				std::cout << status << "\n";
				return;
			}

			if(to_dist_2 < from_dist_2)
			{
				toNode = nullptr;
				toNc = nullptr;
				ngd_parent->restartConnectingNodes(this, fromNode, fromNc->getId());
			}
			else
			{
				fromNode = nullptr;
				fromNc = nullptr;
				ngd_parent->restartConnectingNodes(this, toNode, toNc->getId());
			}

			setHangingPos(m_pos);
		}
	}
}

bool NodeConnectionControl::respondToMouse(APoint m_pos)
{
	return controlState == ControlState::CLICKING || controlState == ControlState::DRAGGING ||
			(fromNode && toNode && (distToLine(m_pos) <= activeRadius));
}

bool NodeConnectionControl::respondToClick(APoint m_pos, MouseButton b)
{
	return fromNode && toNode && (distToLine(m_pos) <= activeRadius);
	//return (controlState == ControlState::HOVERING);
}

void NodeConnectionControl::update(double dt)
{
	if(fromNode && toNode)
	{
		//if(fromNc->isActive())
		//{
			bool	forward = valid(fromNc->activeDir & NDir::FORWARD),
					backward = valid(fromNc->activeDir & NDir::BACKWARD);
	
			f_transition_t += (forward ? 1 : -1)*dt;
			f_transition_t = (f_transition_t > TRANSITION_TIME ? TRANSITION_TIME : f_transition_t);
			f_transition_t = (f_transition_t < 0.0 ? 0.0 : f_transition_t);
			
			b_transition_t += (backward ? 1 : -1)*dt;
			b_transition_t = (b_transition_t > TRANSITION_TIME ? TRANSITION_TIME : b_transition_t);
			b_transition_t = (b_transition_t < 0.0 ? 0.0 : b_transition_t);

			//baseCol =	lerp(inactiveColor, forwardActiveColor, f_transition_t*(1.0/TRANSITION_TIME)) +
			//			lerp(inactiveColor, backwardActiveColor, b_transition_t*(1.0/TRANSITION_TIME));
			//baseCol *= 0.5f;

			baseColor = (forwardActiveColor*f_transition_t + backwardActiveColor*b_transition_t)*(1.0/TRANSITION_TIME);
			baseColor.a = 1.0f;
		//}
		//else// if(transition_t > 0.0)
		//{
		//	transition_t -= dt;
		//	if(transition_t < 0.0)
		//		transition_t = 0.0;
		//}

		//double transition_t = f_transition_t + b_transition_t;
		double transition_t = min(f_transition_t + b_transition_t, TRANSITION_TIME);

		amplitude = smootherInterp(0.0, 1.0, transition_t*(1.0/TRANSITION_TIME));
	}
}

void NodeConnectionControl::draw(GlInterface &gl)
{
	if(visible)
	{
		if(fromNode)
			pFrom = fromNode->getConnectorPoint(fromNc->ioType);
		if(toNode)
			pTo = toNode->getConnectorPoint(toNc->ioType);
		adjustPos();

		const float	length = (pTo - pFrom).length(),
					angle = atan2(pTo.y - pFrom.y, pTo.x - pFrom.x),
					cos_a = cos(angle),
					sin_a = sin(angle);

		APoint start = pFrom;
		AVec step = AVec(cos_a, sin_a);
		AVec perp = AVec(sin_a, -cos_a);

		step.normalize();
		perp.normalize();

		//Multiply color channels by background color
		Color	color_mult = bgStateColors[cState],
				inactive_col(inactiveColor.r*color_mult.r, inactiveColor.g*color_mult.g, inactiveColor.b*color_mult.b, inactiveColor.a*color_mult.a),
				base_col(baseColor.r*color_mult.r, baseColor.g*color_mult.g, baseColor.b*color_mult.b, baseColor.a*color_mult.a),
				arrow_col(arrowColor.r*color_mult.r, arrowColor.g*color_mult.g, arrowColor.b*color_mult.b, arrowColor.a*color_mult.a);

		std::vector<TVertex> points;
		//points.reserve(2*ceil((length - ARROW_LENGTH)/spacing)*(length > ARROW_LENGTH));
		Color col;
		
		/*
		for(float r = 0.0f; r < length - ARROW_LENGTH; r += spacing, even = !even)
		{
			APoint p = start + step*r;
			float	omega_t = SIN_OMEGA*PHYSICS_CLOCK.t,
					phi = SIN_K*r*0.05f,

					height1 = amplitude*(sin(omega_t + phi) + 0.5f)*(sin(omega_t + phi*0.25f) + 1.0f)*0.5f,	//height of sin wave
					offset = width*0.0*amplitude*cos(omega_t*1.0 + phi*0.5f),
					tilt = 0.0f*amplitude*cos(omega_t + phi*0.2),

					even_mult = even ? lerp(1.0f, 0.5f, height) : 1.0f;

			//TODO: Make it look more like its rotating
			//		Maybe have a separate height for each line endpoint with different phases?

			height1 *= even_mult;

			col = lerp(inactive_col, base_col, height1);


			AVec	height_vec1 = perp*lerp(width, (width + radius)*height, amplitude),
					height_vec2 = perp*lerp(width
					tilt_vec = step*tilt,
					offset_vec = perp*offset;

			points.push_back(TVertex(p - height_vec - tilt_vec + offset_vec, col));
			points.push_back(TVertex(p + height_vec + tilt_vec + offset_vec, col));
		}
		*/

		//Add rotating line vertices
		for(float r = 0.0f; r < length - ARROW_LENGTH; r += spacing)
		{
			APoint p = start + step*r;
			float	omega_t = SIN_OMEGA*Clock::getGlobalTime(),
					phi = -SIN_K*r,

					height = amplitude*sin(omega_t + phi),	//height of sin wave
					offset = 0.0f,//amplitude*cos(omega_t*0.5f + phi*0.05f),
					tilt = 2.0f*amplitude*cos(-omega_t + phi);

			//TODO: Make it look more like its rotating

			col = lerp(inactive_col, base_col, -abs(height)*(1.0f + 2.0f*std::max(0.0f, 8.0f*amplitude*(cos(-omega_t*2.0f + phi*0.01f) - 0.9f))));
			//col = lerp(col, Color(0.5f, 0.5f, 0.0f, 1.0f), std::max(0.0f, amplitude*sin(omega_t*40.0f + phi*3.0f)));

			AVec	height_vec = perp*(width + radius*height),
					tilt_vec = step*tilt,
					offset_vec = perp*offset;

			points.push_back(TVertex(p - height_vec - tilt_vec + offset_vec, col));
			points.push_back(TVertex(p + height_vec + tilt_vec + offset_vec, col));
		}


		//Draw lines
		gl.drawShape(GL_LINES, points);
		points.clear();

		col = lerp(inactive_col, arrow_col, amplitude);
		
		//Add arrow point vertices
		points.push_back(TVertex(start + step*(length - ARROW_LENGTH) - perp*width*1.5f, col));
		points.push_back(TVertex(start + step*(length - ARROW_LENGTH) + perp*width*1.5f, col));
		points.push_back(TVertex(start + step*length, col));

		//Draw arrow
		gl.drawShape(GL_TRIANGLES, points);
	}
}