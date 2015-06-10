#include "ContextWindow.h"

#include "WindowWin32.h"
#include "Label.h"

#include "VirtualTransform.h"


/////CONTEXT ITEM/////
const GuiPropFlags ContextListItem::PROP_FLAGS = PFlags::ABSORB_SCROLL | PFlags::HARD_BACK | PFlags::UNCONTAINED;

ContextListItem::ContextListItem(ContextWindow *parent_, ContextWindow *top, APoint a_pos, const ContextTree &child_tree, GuiStateFlags s_flags)
	: GuiElement(parent_, a_pos, AVec(100.0f, 25.0f), GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		parentWindow(parent_), topWindow(top)
{
	label = new Label(this, APoint(0.0f, 0.0f), DEFAULT_STATE, child_tree.label, 12);
	//text->setHeight(12);
	
	setSize(label->getSize() + AVec(40.0f, 10.0f));

	label->centerAround(size*0.5f);
	label->setX(20.0f);

	updateResources();
	
	if(child_tree.children.size() > 0)
	{
		childWindow = new ContextWindow(this, parent_, parent_, parent_->getPos() + APoint(a_pos.x + ContextWindow::CONTEXT_WIDTH, a_pos.y), child_tree.children, SFlags::ENABLED);
	}
}

ContextListItem::~ContextListItem()
{
	if(childWindow)
		delete childWindow;
	childWindow = nullptr;

	if(label)
		delete label;
	label = nullptr;
}


void ContextListItem::onLostFocus()
{
	//closeMenu();
}
	
void ContextListItem::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && !isLeaf())
	{
		toggleMenu();
	}
	else if(isLeaf())
	{
		//parentList->closeMenus();
	}
}
void ContextListItem::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && isLeaf())
	{
		topWindow->closeAllMenus();
		topWindow->hide();
		if(clickAction)
			clickAction();
	}
}

void ContextListItem::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(direct)
	{
		openMenu();
	}
	//else
	//{
	//	setFocus(false);
	//}
}

void ContextListItem::onSizeChanged(AVec d_size)
{
	if(childWindow)
		childWindow->setX(size.x);
}

void ContextListItem::setDefaultBg()
{
	setAllBgStateColors(Color(1.0f, 0.0f, 1.0f, 1.0f));
	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::DRAGGING);
}

void ContextListItem::openMenu()
{
	parentWindow->closeAllMenus(this);

	menuOpen = true;
	inFocus = true;

	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::NONE);

	if(childWindow)
		childWindow->show();
}

void ContextListItem::closeMenu()
{
	menuOpen = false;
	inFocus = false;

	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);

	//Close all submenus and hide list items
	if(childWindow)
	{
		childWindow->closeAllMenus();
		childWindow->hide();
	}
}

void ContextListItem::toggleMenu()
{
	if(menuOpen)	closeMenu();
	else			openMenu();
}

bool ContextListItem::menuIsOpen() const
{
	return menuOpen;
}

bool ContextListItem::isLeaf() const
{
	return (childWindow == nullptr);
}


void ContextListItem::draw(GlInterface &gl)
{
	if(isolateViewport(gl))
	{
		drawBackground(gl);
		drawChildren(gl);
		
		restoreViewport(gl);
	}
	
	if(childWindow)
	{
		childWindow->draw();
		//restoreViewport(gl);
		gl.setViewport(gl.getCurrView());
	}
}


/////CONTEXT WINDOW/////
const GuiPropFlags ContextWindow::PROP_FLAGS = PFlags::HARD_BACK | PFlags::ABSORB_SCROLL | PFlags::UNCONTAINED;
const float ContextWindow::CONTEXT_WIDTH = 200.0f;

ContextWindow::ContextWindow(ParentElement *parent_, Window* parent_window, ContextWindow *top, APoint a_pos, const std::vector<ContextTree> &tree, GuiStateFlags s_flags)
	: GuiElement(parent_, a_pos, AVec(CONTEXT_WIDTH, 500.0), GuiProps(s_flags & ~SFlags::FOCUSED, PROP_FLAGS)),
		Window(a_pos, AVec(100.0f, 500.0f), s_flags & ~SFlags::FOCUSED),
		topWindow(top), parentWindow(parent_window)
{
	topWindow = topWindow ? topWindow : this;

	//Init window
	WindowStyle style;
	style.visible = GuiProps::isVisible(s_flags);
	style.border = WindowBorder::NONE;
	style.hasTitleBar = false;
	style.titleButtons = TitleButton::NONE;
	style.popup = true;
	
	windowImpl = new WindowWin32(this, parentWindow->getWindowImpl(), Vec2i(size), "TEST", style);
	setPos(pos.x, pos.y);
	
	//windowImpl->setResizeFunction(std::bind(&Window::onResize, this, std::placeholders::_1, std::placeholders::_2));

	//updateResources();

	float	offset = 0.0f,
			max_width = CONTEXT_WIDTH;

	items.reserve(tree.size());

	//windowImpl->setActive();

	for(auto it : tree)
	{
		ContextListItem *item = new ContextListItem(this, topWindow, APoint(0.0f, offset), it, DEFAULT_STATE);
		items.push_back(item);

		offset += item->getSize().y;
		if(item->getSize().x > max_width)
			max_width = item->getSize().x;
	}

	for(auto it : items)
		it->setWidth(max_width);

	setSize(AVec(max_width, offset));

	setAllBgStateColors(Color(0.2f, 0.2f, 0.2f, 1.0f));

	updateResources();
}

ContextWindow::~ContextWindow()
{ }

void ContextWindow::onLostFocus()
{
	//if(!submenuOpen())
	//{
	//	closeAllMenus();
	//	hide();
	//}
}

void ContextWindow::onGainedFocus()
{
	//windowImpl->setFocus(false);
}

void ContextWindow::setFocus(bool in_focus)
{
	ActiveElement::setFocus(in_focus);
}

void ContextWindow::onHide()
{
	Window::onHide();
	closeAllMenus();
	//parentWindow->setFocus(true);
}

void ContextWindow::closeAllMenus(ContextListItem *except)
{
	for(auto it : items)
		if(it != except)
			it->closeMenu();
}

bool ContextWindow::submenuOpen() const
{
	for(auto it : items)
		if(it->menuIsOpen())
			return true;

	return false;
}

void ContextWindow::draw()
{
	if(windowImpl && visible)
	{
		windowImpl->setActive();
		GlInterface gl2(ViewRect(APoint(0, 0), size));

		//Draw background
		resetView();
		glClear(GL_COLOR_BUFFER_BIT);

		drawChildren(gl2);
		windowImpl->swapBuffers(windowImpl->getHDC());
	}
	
	if(parentWindow && parentWindow->getWindowImpl())
		parentWindow->getWindowImpl()->setActive();
}

void ContextWindow::draw(GlInterface &gl)
{
	draw();
}
