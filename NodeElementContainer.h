#ifndef APOLLO_GRAPH_MOVABLE_CONTAINER_H
#define APOLLO_GRAPH_MOVABLE_CONTAINER_H

#include "TransformContainer.h"
#include "GraphDisplay.h"

#include <unordered_set>

class NodeElement;

//A graph-like TransformContainer that holds a set of MovableElements that can be selected, moved around, etc.
class NodeElementContainer : public TransformContainer//, public GraphDisplay
{
protected:
	NodeElementContainer(GuiStateFlags s_flags, APoint graph_min, APoint graph_max, AVec base_graph_step, AVec zoom_step);

	std::unordered_set<NodeElement*>	nodes;
	std::unordered_set<NodeElement*>	selectedNodes;

	//Current node that is being dragged
	NodeElement					*draggedNode = nullptr;
	
	bool						selecting = false;
	APoint						selectionPivot;
	Rect						selectionRect;

	AVec						baseGraphStep,
								zoomStep;

	void addNode(NodeElement *new_element);
	void removeNode(NodeElement *e);

	void selectNode(NodeElement *e, bool set_active);
	void deselectNode(NodeElement *e);
	void toggleSelectNode(NodeElement *e);
	void clearSelected();
	
	void moveSelected(APoint m_pos, AVec d_pos);

	//Returns whether selection should be sticky --> doesn't deselect any nodes.
	virtual bool stickySelect() const;
	//Returns whether selection should be toggle-sticky --> deselect nodes by clicking on them, otherwise doesn't deselect
	virtual bool toggleStickySelect() const;

	//Chooses which node should be active
	NodeElement* chooseActiveNode() const;
	void setActiveNode(NodeElement *e);

	virtual void onActiveNodeChanged(NodeElement *current_active)		{ }

	//Called by child NodeElements when the following happens
	void nodeClicked(NodeElement *e);
	void nodeDragged(NodeElement *e, APoint m_pos, AVec d_pos);
	//
	
	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onScroll(APoint m_pos, AVec d_scroll, bool direct) override;
	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;

	virtual void onSizeChanged(AVec d_size) override;

	virtual void drawBackground(GlInterface &gl) override;
	virtual void drawGraph(GlInterface &gl);
	void drawSelectionRect(GlInterface &gl);

public:
	virtual ~NodeElementContainer();

	static const GuiPropFlags PROP_FLAGS;

	void clearGraph();

	virtual void draw(GlInterface &gl) override;

	friend class NodeElement;
};

#endif	//APOLLO_GRAPH_MOVABLE_CONTAINER_H