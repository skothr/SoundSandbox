#ifndef APOLLO_NODE_CONTROL_H
#define APOLLO_NODE_CONTROL_H

#include "Node.h"
#include "Control.h"
#include "NodeElement.h"
#include "Label.h"

#include <vector>

enum class MouseButton;

class Node;
class NodeConnector;
class NodeConnectionControl;
class NodeConnectorControl;
class NodeGraphControl;

enum class NodeConnectionType;

enum class HighlightLevel
{
	NORMAL = 0,
	WRONG = 1,
	RIGHT = 2,

};
typedef HighlightLevel HLevel;

class NodeControl : public NodeElement
{
protected:
	NodeGraphControl *ngc_parent = nullptr;
	Node *node;
	Label nameLabel;

	std::vector<NodeConnectorControl*> connectorControls;
	
	AVec getNodeSize();
	NodeConnector* getDefaultConnector();

	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;

	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;


	virtual void onPosChanged(AVec d_pos) override;


public:
	NodeControl(NodeGraphControl *parent_, APoint g_pos, GuiStateFlags s_flags, Node *n);
	virtual ~NodeControl();

	static const GuiStateFlags STATE_FLAGS;
	static const GuiPropFlags PROP_FLAGS;
	
	static const AVec	BRANCH_SIZE,
						LEAF_SIZE,
						DEFAULT_SIZE;
	
	APoint getConnectorPoint(NodeConnectionType io_type);

	Node* getNode();

	void setHighlight(IOType io_type, NodeDataType data_type);

	friend class NodeConnectionControl;
	friend class NodeConnectorControl;
	friend class NodeGraphControl;
};


class NodeConnectorControl : public Control
{
protected:
	NodeControl			*nc_parent = nullptr;
	NodeGraphControl	*ngc_parent = nullptr;
	NCID				id;
	IOType				ioType = IOType::INVALID;
	NodeDataType		dataType = NodeDataType::INVALID;
	
	static AVec getConnectorSize(IOType io_type);
	
	//virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;

public:
	NodeConnectorControl(NodeControl *parent_, GuiStateFlags s_flags, NCID nc_id);
	virtual ~NodeConnectorControl();

	static const GuiPropFlags PROP_FLAGS;

	NCID getId() const;
	IOType getIoType() const;
	NodeDataType getDataType() const;

	void setHighlight(HighlightLevel level);

	virtual void draw(GlInterface &gl) override;
};


#endif	//APOLLO_NODE_CONTROL_H