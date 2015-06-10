#include "NodeElementContainer.h"

#include "NodeElement.h"
#include "Keyboard.h"

#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

/////NODE ELEMENT CONTAINER/////
const GuiPropFlags NodeElementContainer::PROP_FLAGS = PFlags::HARD_BACK | PFlags::ABSORB_SCROLL;

NodeElementContainer::NodeElementContainer(GuiStateFlags s_flags, APoint graph_min, APoint graph_max, AVec base_graph_step, AVec zoom_step)
	: TransformContainer(GuiProps(s_flags, PROP_FLAGS)),
		//GraphDisplay(graph_min, graph_max, RVec(1.0f, 1.0f), GVec(50.0f, 50.0f), GraphProps::NONE),
		baseGraphStep(base_graph_step), zoomStep(zoom_step)
{
	setAllBgStateColors(Color(0.10f, 0.10f, 0.10f, 1.0f));

	//setMinZoom(RVec(0.3f, 0.3f));
	//setMaxZoom(RVec(3.0f, 3.0f));
	setMinZoom(RVec(0.05f, 0.05f));
	setMaxZoom(RVec(10.0f, 10.0f));
	setMinViewOffset(graph_min);
	setMaxViewOffset(graph_max);

	setViewOffset(-size*(1.0f/2.0f));
	setZoomPivot(size*(1.0f/2.0f));
}

NodeElementContainer::~NodeElementContainer()
{

}

void NodeElementContainer::addNode(NodeElement *e)
{
	nodes.emplace(e);
}

void NodeElementContainer::removeNode(NodeElement *e)
{
	auto iter = std::find(nodes.begin(), nodes.end(), e);

	if(iter != nodes.end())
	{
		if(e)
		{
			removeChild(e, false);
			nodes.erase(iter);
			delete e;
		}
	}
}

void NodeElementContainer::selectNode(NodeElement *e, bool set_active)
{
	if(!e->isSelected())
	{
		selectedNodes.emplace(e);
		e->setSelected(true);
	}

	if(set_active)
		setActiveNode(e);
}

void NodeElementContainer::deselectNode(NodeElement *e)
{
	if(e->isSelected())
	{
		auto iter = std::find(selectedNodes.begin(), selectedNodes.end(), e);

		if(iter != selectedNodes.end())
		{
			//Erase from selected
			selectedNodes.erase(iter);
			e->setSelected(false);
		}
	}
}

void NodeElementContainer::toggleSelectNode(NodeElement *e)
{
	if(e->isSelected())
		deselectNode(e);
	else
		selectNode(e, true);//(selectedNodes.size() > 0));
}

void NodeElementContainer::clearSelected()
{
	for(auto n : selectedNodes)
		n->setSelected(false);
	selectedNodes.clear();

	setActiveNode(nullptr);
}

void NodeElementContainer::moveSelected(APoint m_pos, AVec d_pos)
{
	for(auto n : selectedNodes)
	{
		n->setPos(n->getPos() + d_pos);
		
		/*
		Rect nr = n->getRect();

		bool stuck = false;

		for(auto nn : nodes)
		{
			Rect nnr = nn->getRect();
			if(nn != n && Rect::intersects(nr, nnr))
			{
				Rect intersection = Rect::findIntersection(nr, nnr);
				APoint	nmp = nr.pos + nr.size*(1.0f/2.0f),
						nnmp = nnr.pos + nnr.size*(1.0f/2.0f);

				if(intersection.size.x > intersection.size.y)
					n->setY(nmp.y > nnmp.y ? (nnr.pos.y + nnr.size.y) : (nnr.pos.y - nr.size.y));
				else
					n->setX(nmp.x > nnmp.x ? (nnr.pos.x + nnr.size.x) : (nnr.pos.x - nr.size.x));
			}
		}

		if(!stuck)
			n->setPos(nr.pos + m_pos - n->guidePoint);

		//n->setPos(nr.pos);
		*/
	}
}

bool NodeElementContainer::stickySelect() const
{
	return Keyboard::keyDown(Keys::K_LSHIFT) || Keyboard::keyDown(Keys::K_RSHIFT);
}

bool NodeElementContainer::toggleStickySelect() const
{
	return Keyboard::keyDown(Keys::K_LCTRL) || Keyboard::keyDown(Keys::K_RCTRL);
}

NodeElement* NodeElementContainer::chooseActiveNode() const
{
	//TODO: Make this smarter
	return (selectedNodes.size() > 0 ? *selectedNodes.begin() : nullptr);
}

void NodeElementContainer::setActiveNode(NodeElement *e)
{
	for(auto n : nodes)
		n->setFocus(n == e);

	onActiveNodeChanged(e);
}

void NodeElementContainer::nodeClicked(NodeElement *e)
{
	if(stickySelect())
		selectNode(e, true);
	else if(toggleStickySelect())
		toggleSelectNode(e);
	else
	{
		//Only clear selection list if this element is not in it
		if(!e->isSelected())
			clearSelected();

		selectNode(e, true);
	}
}

void NodeElementContainer::nodeDragged(NodeElement *e, APoint m_pos, AVec d_pos)
{
	if(!e->isSelected())
		nodeClicked(e);

	moveSelected(m_pos, d_pos);
}


void NodeElementContainer::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(direct && draggedNode)
	{
		//TODO
	}
}

void NodeElementContainer::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && valid(b & MB::LEFT))
	{
		if(!stickySelect() && !toggleStickySelect())
			clearSelected();

		//Start selection rectangle
		selecting = true;
		selectionPivot = m_pos;
		selectionRect.pos = selectionPivot;
		selectionRect.size = AVec(0.0f, 0.0f);

		setActiveNode(nullptr);
	}
}

void NodeElementContainer::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	//if(direct && draggedNode)
	//{
//
	//}
	if(selecting && valid(b & MB::LEFT))
	{
		setActiveNode(chooseActiveNode());
		selecting = false;

		for(auto n : nodes)
			n->toggleBuffer = n->isSelected();
	}
}

void NodeElementContainer::onScroll(APoint m_pos, AVec d_scroll, bool direct)
{
	if(direct)
	{
		RVec a_zoom = getZoom();
		AVec mult = AVec(1.0f, 1.0f) - zoomStep*(d_scroll.x + d_scroll.y);

	
		//APoint old_center = transform.absoluteToVirtualPoint(size*(1.0f/2.0f));
		scaleZoom(mult);
		//APoint new_center = transform.absoluteToVirtualPoint(size*(1.0f/2.0f));
		//
		//AVec	center_diff = old_center - new_center;
		//moveViewOffset(center_diff);
	}
}

void NodeElementContainer::onDrag(APoint m_pos, AVec d_pos, bool direct)
{
	if(valid(Mouse::getButtonStates() & MB::MIDDLE))
	{
		moveViewOffset(-transform.absoluteToVirtualVec(d_pos));//absoluteToGraphVec(d_pos));
	}
	else if(selecting)
	{
		//Adjust selection rect
		selectionRect.pos.x = (selectionPivot.x > m_pos.x ? m_pos.x : selectionPivot.x);
		selectionRect.size.x = (selectionPivot.x - m_pos.x)*(selectionPivot.x > m_pos.x ? 1.0f : -1.0f);
		
		selectionRect.pos.y = (selectionPivot.y > m_pos.y ? m_pos.y : selectionPivot.y);
		selectionRect.size.y = (selectionPivot.y - m_pos.y)*(selectionPivot.y > m_pos.y ? 1.0f : -1.0f);

		
		//Clamp selection rect within absolute bounds
		Rect bounds(APoint(0.0f, 0.0f), size);
		selectionRect = Rect::findIntersection(selectionRect, Rect(APoint(0.0f, 0.0f), size));//Rect(absoluteToGraphPoint(APoint(0.0f, 0.0f)), absoluteToGraphVec(size)));

		//Update selected nodes
		if(!stickySelect() && !toggleStickySelect())
			clearSelected();

		for(auto n : nodes)
		{
			if(Rect::intersects(transform.virtualToAbsoluteRect(n->getRect()), selectionRect))
			{
				if(stickySelect())
					selectNode(n, false);
				else if(toggleStickySelect())
				{
					if(n->toggleBuffer == n->isSelected())
						toggleSelectNode(n);
				}
				else
					selectNode(n, false);
			}
			else if(toggleStickySelect() && n->toggleBuffer != n->isSelected())
			{
				n->setSelected(n->toggleBuffer);
			}
		}
	}
}

void NodeElementContainer::onSizeChanged(AVec d_size)
{
	moveViewOffset(transform.absoluteToVirtualVec(-d_size*(1.0f/2.0f)));
	setZoomPivot(size*(1.0f/2.0f));
}

void NodeElementContainer::drawBackground(GlInterface &gl)
{
	TransformContainer::drawBackground(gl);

	//Draw standard scale as the background
	//ViewRect curr_view = gl.getCurrView();
	APoint v_origin = transform.absoluteToVirtualPoint(APoint(0.0f, 0.0f));//curr_view.pos);
	AVec v_size = transform.absoluteToVirtualVec(size);//curr_view.size);

	APoint	low = v_origin,
			high = v_origin + v_size;

	GPoint	start_lines = GPoint(floor(low.x/baseGraphStep.x)*baseGraphStep.x,
									floor(low.y/baseGraphStep.y)*baseGraphStep.y),
			end_lines = GPoint(ceil(high.x/baseGraphStep.x)*baseGraphStep.x,
									ceil(high.y/baseGraphStep.y)*baseGraphStep.y);

	//Vertical lines (x dimension) --> GREY
	gl.setColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
	for(float g_x = start_lines.x; g_x <= end_lines.x; g_x += baseGraphStep.x)
		gl.drawLine(transform.virtualToAbsolutePoint(APoint(g_x, low.y)), transform.virtualToAbsolutePoint(APoint(g_x, high.y)));
	
	//Horizontal lines (y dimension) --> GREY
	gl.setColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
	for(float g_y = start_lines.y; g_y <= end_lines.y; g_y += baseGraphStep.y)
		gl.drawLine(transform.virtualToAbsolutePoint(APoint(low.x, g_y)), transform.virtualToAbsolutePoint(APoint(high.x, g_y)));

	gl.setColor(Color(0.0f, 0.0f, 1.0f, 1.0f));
	gl.drawLine(transform.virtualToAbsolutePoint(GPoint(minViewOffset.x, 0.0f)), transform.virtualToAbsolutePoint(GPoint(maxViewOffset.x, 0.0f)) + GVec(size.x, 0.0f));
	gl.drawLine(transform.virtualToAbsolutePoint(GPoint(0.0f, minViewOffset.y)), transform.virtualToAbsolutePoint(GPoint(0.0f, maxViewOffset.y)) + GVec(0.0f, size.y));

}
	
void NodeElementContainer::drawGraph(GlInterface &gl)
{
	drawBackground(gl);
}

void NodeElementContainer::drawSelectionRect(GlInterface &gl)
{
	APoint	p = selectionRect.pos;
	AVec	v = selectionRect.size;

	gl.setColor(Color(0.2f, 0.6f, 0.75f, 1.0f));
	gl.drawLine(p, APoint(p.x + v.x, p.y));
	gl.drawLine(APoint(p.x + v.x, p.y), APoint(p.x + v.x, p.y + v.y));
	gl.drawLine(APoint(p.x + v.x, p.y + v.y), APoint(p.x, p.y + v.y));
	gl.drawLine(APoint(p.x, p.y + v.y), p);
}

void NodeElementContainer::clearGraph()
{
	clearChildren();
	draggedNode = nullptr;
	clearSelected();

	//Delete nodes
	for(auto n : nodes)
		if(n)
			delete n;
	nodes.clear();
}


void NodeElementContainer::draw(GlInterface &gl)
{
	if(TransformContainer::isolateViewport(gl))
	{
		drawGraph(gl);
		drawChildren(gl);

		if(selecting)
			drawSelectionRect(gl);

		restoreViewport(gl);
	}
}

