#ifndef APOLLO_NODE_GRAPH_CONTROL_H
#define APOLLO_NODE_GRAPH_CONTROL_H

#include "NodeElementContainer.h"

#include <vector>

#include <unordered_map>
#include <unordered_set>

class Node;
class NodeGraph;
class PropertiesDisplay;

class NodeControl;
class NodeConnection;
class NodeConnectorControl;
class NodeConnectionControl;

typedef int NCID;

class NodeGraphControl : public NodeElementContainer
{
protected:
	NodeGraph			*nodeGraph = nullptr;
	PropertiesDisplay	*propDisp = nullptr;

	std::unordered_set<NodeConnectionControl*>	connections;
	std::unordered_map<Node*, NodeControl*>		nodeControls;	//Set of Nodes that are set up

	NodeConnectionControl						*movingConnection = nullptr;

	void updateGraph();

	virtual void onSizeChanged(AVec d_size) override;

	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;

	virtual void onActiveNodeChanged(NodeElement *e) override;

public:
	NodeGraphControl(ParentElement *parent_, APoint a_pos, AVec a_size, GuiStateFlags s_flags, NodeGraph *node_graph);
	virtual ~NodeGraphControl();

	static const GuiPropFlags PROP_FLAGS;

	static const APoint			GRAPH_MIN,
								GRAPH_MAX;
	static const AVec			UNIT_SIZE,
								ZOOM_STEP;

	void setPropDisplay(PropertiesDisplay *prop_disp);
	void setGraph(NodeGraph *node_graph);

	void startConnect(NodeControl *nc, NCID nc_id);
	void startConnect(NodeConnectorControl *ncc);

	void finishConnect(NodeControl *nc, NCID nc_id);
	void finishConnect(NodeConnectorControl *ncc);

	NodeControl* getNodeControl(Node *node);

	friend class NodeConnectionControl;
	friend class NodeControl;
};


#endif	//APOLLO_NODE_GRAPH_CONTROL_H