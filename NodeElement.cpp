#include "NodeElement.h"

#include "NodeElementContainer.h"

/////MOVABLE CONTROL/////
const GuiPropFlags NodeElement::PROP_FLAGS = PFlags::HARD_BACK;

NodeElement::NodeElement(NodeElementContainer *parent_, GuiStateFlags s_flags)
	: CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		nec_parent(parent_)
{

}

NodeElement::~NodeElement()
{

}


void NodeElement::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{

}

void NodeElement::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && nec_parent && valid(b & MB::LEFT))
	{
		guidePoint = m_pos;
		dragging = true;//selected || (nec_parent->selectedNodes.size() <= 1);
		//nec_parent->nodeDragged(this, m_pos, AVec());
		nec_parent->nodeClicked(this);
	}
	//	nec_parent->nodeClicked(this);
}

void NodeElement::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	//if(direct && nec_parent && valid(b & MB::LEFT))
	//	nec_parent->nodeClicked(this);
	dragging = false;
}

void NodeElement::onScroll(APoint m_pos, AVec d_scroll, bool direct)
{
	//NodeElementContainer handles scrolling
}

void NodeElement::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	if(nec_parent && (direct || cState == CS::DRAGGING) && dragging)
		nec_parent->nodeDragged(this, m_pos, d_pos);
}

void NodeElement::setSelected(bool is_selected)
{
	selected = is_selected;
}

bool NodeElement::isSelected() const
{
	return selected;
}


void NodeElement::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);

		if(selected)
		{
			APoint p(0.0f, 0.0f);
			AVec v(size.x, size.y);

			gl.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));//Color(0.1f, 0.3f, 0.8f, 1.0f));
			gl.drawLine(p, APoint(p.x + v.x, p.y));
			gl.drawLine(APoint(p.x + v.x, p.y), APoint(p.x + v.x, p.y + v.y));
			gl.drawLine(APoint(p.x + v.x, p.y + v.y), APoint(p.x, p.y + v.y));
			gl.drawLine(APoint(p.x, p.y + v.y), p);
		}

		restoreViewport(gl);
	}
}