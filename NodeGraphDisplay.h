#ifndef APOLLO_NODE_GRAPH_DISPLAY_H
#define APOLLO_NODE_GRAPH_DISPLAY_H

#include "NodeElementContainer.h"
#include "NodeElement.h"

#include "Node.h"

class NodeGraph;
class PropertiesDisplay;
class Node;

class NodeControl;
class NodeConnectorControl;
class NodeConnectionControl;

class Label;

static const MouseButton NODEGRAPH_SELECT_NODES_BTN = MB::LEFT;
static const MouseButton NODEGRAPH_MOVE_NODES_BTN = MB::LEFT;
static const MouseButton NODEGRAPH_CONNECT_NODES_NODE_BTN = MB::MIDDLE;		//Connecting nodes by clicking on the nodes' body
static const MouseButton NODEGRAPH_CONNECT_NODES_CONNECTOR_BTN = MB::LEFT;	//Connecting nodes by clicking on the node connector
static const MouseButton NODEGRAPH_MOVE_OFFSET_BTN = MB::MIDDLE;

class NodeGraphDisplay : public NodeElementContainer
{
protected:
	NodeGraph		*nodeGraph = nullptr;


	std::vector<NodeControl*>			nodeControls;
	std::vector<NodeControl*>			selectedControls;

	std::vector<NodeConnectionControl*> connections;

	NodeControl		*draggedNode = nullptr;
	NCID			draggedConnector = 0;
	int				connectionIndex = -1;

	APoint			selectionPivot;
	Rect			selectionRect;

	PropertiesDisplay *propDisp = nullptr;

	static const AVec BASE_GRAPH_STEP;
	static const AVec ZOOM_STEP;
	
	//Initialization
	void initGraph();
	//void initNodeControl(Node *n, Point2f pos, const Vec2f &step, std::unordered_set<Node*> &visited_nodes, Point2f &max_pos, Point2f &min_pos);

	void addNode(Node *n, Point2f graph_pos);
	void addConnection(NodeControl *from_nc, NodeControl *to_nc, NCID from_id, NCID to_id);
	void addConnection(NCID from_id, NCID to_id);

	void removeConnection(int index);

	//Selection
	void clearSelected();
	void selectNode(NodeControl *nc, bool set_active);
	void deselectNode(NodeControl *nc);
	void selectNodeToggle(NodeControl *nc);

	//Returns whether selection should be sticky --> doesn't deselect any nodes
	bool stickySelect();
	//Returns whether selection should be toggle-sticky --> deselect nodes by clicking on them, otherwise doesnt deselect
	bool toggleStickySelect();

	//Chooses a selected node to be active (for properties display)
	NodeControl* chooseActiveNode();
	
	void setActiveNode(NodeControl *nc);

public:
	NodeGraphDisplay(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, NodeGraph *node_graph);
	virtual ~NodeGraphDisplay();

	static const GuiPropFlags PROP_FLAGS;

	//Clears all nodes/connections then re-creates with nodeGraph
	void reset();

	int getNodeIndex(Node *node_ptr);
	bool nodeIsVisible(unsigned int index);
	bool nodeIsVisible(Node *node_ptr);

	void setPropDisp(PropertiesDisplay *prop_disp);
	PropertiesDisplay* getPropDisp();

	NodeControl *getNodeControl(unsigned int index);
	NodeControl *getNodeControl(Node *node_ptr);

	//If connector is null, guesses which connector it should connect to
	void startConnectingNodes(NodeControl *from_node, NCID *connector = nullptr);
	void finishConnectingNodes(NodeControl *to_node, NCID *connector = nullptr);

	void restartConnectingNodes(NodeConnectionControl *connection, NodeControl *connected_node, NCID connector);

	void moveSelectedNodes(AVec d_pos);
	void onControlClick(NodeControl *nc);
	void onControlDrag(NodeControl *nc, APoint m_pos);

	virtual void onSizeChanged(AVec d_size) override;

	virtual void onScroll(APoint m_pos, Vec2f d_scroll) override;
	virtual void onMouseDown(APoint m_pos, MouseButton b) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b) override;
	virtual void onMouseMove(APoint m_pos, AVec d_pos) override;
	virtual void onDrag(APoint m_pos, AVec d_pos) override;

	virtual bool respondToClick(APoint m_pos, MouseButton b) override;
	
	virtual void update(const Time &dt) override;

	void drawGraph(GlInterface &gl);
	virtual void draw(GlInterface &gl) override;
};

class NodeControl : public NodeElement
{
protected:
	NodeGraphDisplay			*ngd_parent = nullptr;
	Node						*node = nullptr;
	Label						*nodeName = nullptr;

	std::vector<NodeControl*>	selectedControl;

	NodeConnectorControl		*connectorControls[static_cast<unsigned int>(NCType::COUNT)];
	
	void initControl();
	
	virtual void onMouseDown(APoint m_pos, MouseButton b) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b) override;
	virtual void onMouseMove(APoint m_pos, AVec d_pos) override;
	virtual void onDrag(APoint m_pos, AVec d_pos) override;
	
	virtual bool respondToClick(APoint m_pos, MouseButton b) override;

public:
	NodeControl(NodeGraphDisplay *parent_, APoint graph_pos, Node *node_);
	virtual ~NodeControl();

	static const GuiPropFlags PROP_FLAGS;
	static const GuiStateFlags STATE_FLAGS;
	
	static const AVec	BRANCH_SIZE,
						LEAF_SIZE,
						DEFAULT_SIZE;
	
	bool selected = false;

	NodeGraphDisplay* getNgdParent();

	Node* getNode();

	AVec getNodeSize();
	//Returns where the connector for this node goes, depending on the type of connection (i/o)
	APoint getConnectorPoint(NCType io_type);
	NodeConnector* getDefaultConnector();

	virtual void update(const Time &dt) override;

	void drawBody(GlInterface &gl);
	virtual void draw(GlInterface &gl) override;
};


class NodeConnectorControl : public Control
{
private:
	NodeControl			*nc_parent = nullptr;
	NodeGraphDisplay	*ngd = nullptr;
	NCType				ioType = NCType::DATA_INPUT;

public:
	NodeConnectorControl(NodeControl *parent_, NCType io_type);
	virtual ~NodeConnectorControl() = default;

	static AVec getConnectorSize(NCType io_type);

	virtual void onMouseDown(APoint m_pos, MouseButton b) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b) override;
	virtual void onMouseMove(APoint m_pos, AVec d_pos) override;
	virtual void onDrag(APoint m_pos, AVec d_pos) override;
	
	virtual bool respondToClick(APoint m_pos, MouseButton b) override;

	virtual void draw(GlInterface &gl) override;
};

class NodeConnectionControl : public Control
{
protected:
	NodeGraphDisplay *ngd_parent = nullptr;

	NodeControl		*fromNode = nullptr,
					*toNode = nullptr;

	NodeConnector	*fromNc = nullptr,
					*toNc = nullptr;

	APoint			pFrom,
					pTo;

	float			radius = 5.0f,						//Height of sin wave
					radius_inv = 1.0f/radius,
					width = 3.0f,						//Base height
					spacing = 3.0f,						//Space between lines
					activeRadius = width*(4.0f/3.0f),	//Distance away from center line where control will respond to mouse
					disconnectRadius = width*2.0f;		//Distance away from center line where dragging will disconnect a side


	static const double	TRANSITION_TIME;	//In seconds
	static const float	ARROW_LENGTH,
						SIN_WAVELENGTH,		//NOTE: Everything is derived from wavelength and velocity.
						SIN_VELOCITY,
						SIN_FREQUENCY,
						SIN_K,
						SIN_OMEGA;
	
	static const Color	inactiveColor,			//Color of connection when inactive
						forwardActiveColor,		//Color of connection when data is being pushed
						backwardActiveColor,	//Color of connection when data is being pulled
						arrowColor;				//Color of the connection arrow when active
	
	Color			baseColor = Color(0.0f, 0.0f, 0.0f, 0.0f);	//Base color (before adjusting for height, amplitude, etc)


	float			amplitude = 0.0f,		//Current amplitude for "active" sin wave (0 to 1)
					aVel = 0.0f;			//amplitude velocity
	//Transition time value for forward/backward activity
	double			f_transition_t = 0.0f,
					b_transition_t = 0.0f;

	void adjustPos();

	float distToLine(APoint p);
	
public:
	//Use this constructor if connection is not complete at time of creation
	//NodeConnectionControl(NodeGraphDisplay *parent_, NodeControl *base_node, NCType base_io_type);
	//Use this constructor if connection is already complete at time of creation
	NodeConnectionControl(NodeGraphDisplay *parent_, NodeControl *from_node, NodeControl *to_node, NCID from_id, NCID to_id);

	virtual ~NodeConnectionControl() = default;

	void setOtherNode(NodeControl *other_node, NCID other_id);
	void setOtherNode(NodeControl *other_node, NodeConnector *other_nc);

	bool hasFrom();
	bool hasTo();

	void setHangingPos(APoint h_pos);

	void setNodeConnector(NodeConnector *connector);

	virtual void onMouseDown(APoint m_pos, MouseButton b) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b) override;
	virtual void onDrag(APoint m_pos, AVec d_pos) override;
	
	virtual bool respondToMouse(APoint m_pos) override;
	virtual bool respondToClick(APoint m_pos, MouseButton b) override;

	virtual void update(const Time &dt) override;
	virtual void draw(GlInterface &gl) override;
};

#endif	//APOLLO_NODE_GRAPH_DISPLAY_H