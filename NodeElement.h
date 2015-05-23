#ifndef APOLLO_MOVABLE_ELEMENT_H
#define APOLLO_MOVABLE_ELEMENT_H

#include "CompoundControl.h"

class NodeElementContainer;

class NodeElement : public CompoundControl
{
protected:
	NodeElement(NodeElementContainer *parent_, GuiStateFlags s_flags);

	NodeElementContainer	*nec_parent = nullptr;

	bool					selected = false,
							dragging = false;

	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onScroll(APoint m_pos, AVec d_scroll, bool direct) override;
	virtual void onDrag(APoint m_pos, AVec d_pos, bool direct) override;

	//virtual void onStartDragging(APoint m_pos, AVec d_pos) override;
	//virtual void onDrop(APoint m_pos, ActiveElement *element) override;

public:
	virtual ~NodeElement();
	
	APoint					guidePoint = APoint();

	static const GuiPropFlags PROP_FLAGS;

	bool					toggleBuffer = false;	//if toggleBuffer != selected, dont toggle (holds last selected value)

	void setSelected(bool is_selected);
	bool isSelected() const;

	virtual void draw(GlInterface &gl) override;

};

#endif	//APOLLO_MOVABLE_ELEMENT_H