#include "MenuBar.h"

#include <GL/glew.h>

#include "ContextWindow.h"

/////MENU BAR/////
const GuiPropFlags MenuBar::PROP_FLAGS	= PFlags::HARD_BACK | PFlags::ABSORB_SCROLL | PFlags::UNCONTAINED;
const float MenuBar::DEFAULT_BAR_HEIGHT = 25.0f;
const int MenuBar::TEXT_HEIGHT			= 14;

MenuBar::MenuBar(ParentElement *parent_, GuiStateFlags s_flags, const ContextTree &tree, float bar_height)
	: GuiElement(parent_, APoint(), AVec(), GuiProps(s_flags, PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, PROP_FLAGS)),
		height(bar_height <= 0.0f ? DEFAULT_BAR_HEIGHT : bar_height)
{
	AVec parent_size = parent->getSize();

	setSize(AVec(parent_size.x, height));

	std::vector<GuiElement*> children;
	GuiStateFlags b_state(SFlags::VISIBLE | SFlags::ENABLED | SFlags::FLOATING);

	int offset = 0;
	for(unsigned int i = 0; i < tree.children.size(); i++)
	{
		MenuBarButton *b = new MenuBarButton(this, b_state, tree.children[i], offset, height);
		children.push_back(b);
		menuButtons.push_back(b);

		offset += b->getSize().x;
	}

	CompoundControl::init(nullptr, &children);

	//Attach to top of parent container
	attachTo(parent, AttachSide::LEFT, 0.0f);
	attachTo(parent, AttachSide::RIGHT, 0.0f);
	attachTo(parent, AttachSide::TOP, 0.0f);

	setDefaultBg();
}

MenuBar::~MenuBar()
{
	for(auto b : menuButtons)
		if(b) delete b;

	menuButtons.clear();
}

void MenuBar::setDefaultBg()
{
	setAllBgStateColors(Color(0.1f, 0.1f, 0.1f, 1.0f));
	setBgStateColor(Color(1.0f, 0.0f, 1.0f, 1.0f), CS::INVALID);
}

void MenuBar::onLostFocus()
{
	closeAllMenus();
}

void MenuBar::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct)
	{
		closeAllMenus();
	}
	else if(pointInside(m_pos))
	{
		for(auto mb : menuButtons)
			if(mb->pointInside(m_pos))
				return;
		
		//If not over any of the buttons, close menus
		closeAllMenus();
	}
}

void MenuBar::closeAllMenus(MenuBarButton *except)
{
	for(auto b : menuButtons)
		if(b != except)
			b->closeMenu();
}

bool MenuBar::submenuOpen()
{
	for(auto b : menuButtons)
		if(b->menuIsOpen())
			return true;

	return false;
}

void MenuBar::draw(GlInterface &gl)
{
	if(isolateViewport(gl, false))	//Dont clamp children
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}


/////MENU BAR BUTTON/////
const float MenuBarButton::LABEL_SIDE_PADDING	= 15;
const AVec MenuBarButton::PADDING				= AVec(1.0f, 2.0f);	//Horizontal padding doubled between buttons

MenuBarButton::MenuBarButton(ParentElement *parent_, GuiStateFlags s_flags, const ContextTree &tree, float x_offset, float height)
	: GuiElement(parent_, APoint(x_offset, 0.0f), AVec(0.0f, height), GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		menu(dynamic_cast<MenuBar*>(parent_))
{
	Window *parent_window = parent->getTopWindow();

	label = new Label(this, APoint(0.0f, 0.0f), s_flags, tree.label, MenuBar::TEXT_HEIGHT);
	list = new ContextWindow(this, parent_window, nullptr, parent_window->getClientPos() + getPos() + AVec(0.0f, height), tree.children, (SFlags::ENABLED | SFlags::FLOATING));

	setWidth(label->getSize().x + 2.0f*(LABEL_SIDE_PADDING + PADDING.x));
	label->centerAround(APoint(size*(1.0f/2.0f)));

	std::vector<GuiElement*> children {label, list};

	CompoundControl::init(nullptr, &children);
	
	setDefaultBg();
}

MenuBarButton::~MenuBarButton()
{
	if(list) delete list;
	list = nullptr;

	if(label) delete label;
	label = nullptr;
}

void MenuBarButton::onLostFocus()
{
	//closeMenu();
}

void MenuBarButton::onMouseDown(APoint m_pos, MouseButton b, bool direct)
{
	if(direct)
	{
		toggleMenu();
		//openMenu();
	}
	//else
	//{
	//	closeMenu();
	//}
}

void MenuBarButton::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	if(direct)
	{
		//if(!menuOpen)
		//	topMenu->closeAllMenus();

		//toggleMenu();
	}
}

void MenuBarButton::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(direct && menu->submenuOpen())
	{
		openMenu();
		//setFocus(true);
		//getTopWindow()->setFocus(true);
	}
}

void MenuBarButton::drawBackground(GlInterface &gl)
{
	//Recede border for padding
	gl.setColor(bgStateColors[cState]);
	gl.drawRect(APoint(PADDING), size - PADDING*2.0f);
	
	if(inFocus)
	{
		AVec offset = PADDING + AVec(0.5f, 0.5f);

		gl.setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
		gl.drawLine(offset,											APoint(offset.x, size.y - offset.y));
		gl.drawLine(APoint(offset.x, size.y - offset.y),			APoint(size.x - offset.x, size.y - offset.y));
		gl.drawLine(APoint(size.x - offset.x, size.y - offset.y),	APoint(size.x - offset.x, offset.y));
		gl.drawLine(APoint(size.x - offset.x, offset.y),			offset);
	}
}

void MenuBarButton::setDefaultBg()
{
	setAllBgStateColors(Color(1.0f, 0.0f, 1.0f, 1.0f));
	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::DRAGGING);
}


void MenuBarButton::openMenu()
{
	menu->closeAllMenus(this);

	menuOpen = true;
	//
	//TODO: New state for this scenario?
	//
	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::NONE);

	//Make list items visible
	list->show();
}

void MenuBarButton::closeMenu()
{
	menuOpen = false;

	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);

	//Hide context list
	list->hide();
}

void MenuBarButton::toggleMenu()
{
	if(menuOpen)	closeMenu();
	else			openMenu();
}

bool MenuBarButton::menuIsOpen() const
{
	return menuOpen;
}


void MenuBarButton::draw(GlInterface &gl)
{
	if(isolateViewport(gl, false))	//Dont clamp children
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}


/*
/////MENU LIST/////

MenuList::MenuList(ParentElement *parent_, MenuBar *top_menu, APoint a_pos, GuiStateFlags s_flags, const std::vector<MenuTree> &menu_items)
	: GuiElement(parent_, a_pos, AVec(), GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		topMenu(top_menu)
{
	std::vector<GuiElement*> children;

	GuiStateFlags item_state = SFlags::VISIBLE | SFlags::ENABLED | SFlags::FLOATING;

	int max_width = 0;
	for(unsigned int i = 0; i < menu_items.size(); i++)
	{
		MenuListItem *li = new MenuListItem(this, topMenu, APoint(0.0f, i*MenuListItem::HEIGHT), item_state, menu_items[i]);
		children.push_back(li);
		listItems.push_back(li);

		int w = li->getSize().x;
		max_width = (w > max_width) ? w : max_width;
	}

	setSize(AVec(max_width, menu_items.size()*MenuListItem::HEIGHT));

	CompoundControl::init(nullptr, &children);
	
	//Standardize width within the vertical menu
	for(auto item : listItems)
		item->setWidth(max_width);
	
	setDefaultBg();
}

MenuList::~MenuList()
{
	for(auto item : listItems)
		if(item) delete item;
	listItems.clear();
}

void MenuList::closeMenus()
{
	for(auto item : listItems)
		item->closeMenu();
}

bool MenuList::submenuOpen()
{
	for(auto item : listItems)
		if(item->menuIsOpen())
			return true;

	return false;
}

void MenuList::draw(GlInterface &gl)
{
	if(isolateViewport(gl, false))	//Dont clamp children
	{
		drawBackground(gl);
		drawChildren(gl);

		restoreViewport(gl);
	}
}


/////MENU LIST ITEM/////
const int	MenuListItem::LABEL_SIDE_PADDING	= 30,
			MenuListItem::HEIGHT				= 20;

MenuListItem::MenuListItem(ParentElement *parent_, MenuBar *top_menu, APoint a_pos, GuiStateFlags s_flags, const MenuTree &menu)
	: GuiElement(parent_, a_pos, AVec(0.0f, HEIGHT), GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		CompoundControl(GuiProps(s_flags, MenuBar::PROP_FLAGS)),
		topMenu(top_menu), parentList(dynamic_cast<MenuList*>(parent_)), menuOpen(false), clickAction(menu.clickAction)
{
	label = new Label(this, APoint(), s_flags, menu.label, MenuBar::TEXT_HEIGHT);
	setWidth(label->getSize().x + 2.0f*LABEL_SIDE_PADDING);
	label->centerAround(APoint(size*(1.0f/2.0f)));

	childList = (menu.children.size() > 0 ? new MenuList(this, top_menu, APoint(size.x, 0.0f), (SFlags::ENABLED | SFlags::FLOATING), menu.children) : nullptr);

	std::vector<GuiElement*> children {label, childList};

	CompoundControl::init(nullptr, &children);
	
	setDefaultBg();
}

MenuListItem::~MenuListItem()
{
	if(childList) delete childList;
	childList = nullptr;

	if(label) delete label;
	label = nullptr;
}

void MenuListItem::onLostFocus()
{
	//closeMenu();
}

void MenuListItem::onMouseDown(APoint m_pos, MouseButton b, bool direct)
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

void MenuListItem::onMouseUp(APoint m_pos, MouseButton b, bool direct)
{
	if(direct && isLeaf())
	{
		topMenu->closeAllMenus();
		if(clickAction)
			clickAction();
	}
	//if(!menuOpen)
	//	parentList->closeMenus();

	//if(direct)
	//{
	//	if(isLeaf())
	//	{
	//		topMenu->closeAllMenus();
	//		if(clickAction)
	//			clickAction();
	//	}
	//}
}

void MenuListItem::onMouseMove(APoint m_pos, AVec d_pos, bool direct)
{
	if(direct && parentList->submenuOpen())
	{
		parentList->closeMenus();
		openMenu();
		inFocus = true;
	}
}

void MenuListItem::onSizeChanged(AVec d_size)
{
	if(childList) childList->setX(size.x);
}

void MenuListItem::setDefaultBg()
{
	setAllBgStateColors(Color(1.0f, 0.0f, 1.0f, 1.0f));
	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::HOVERING);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::CLICKING);
	setBgStateColor(Color(0.3f, 0.4f, 0.5f, 1.0f), CS::DRAGGING);
}

void MenuListItem::openMenu()
{
	parentList->closeMenus();

	menuOpen = true;
	//
	//TODO: New state for this scenario?
	//
	setBgStateColor(Color(0.2f, 0.2f, 0.2f, 1.0f), CS::NONE);

	//Make list items visible
	if(childList) childList->show();
}

void MenuListItem::closeMenu()
{
	menuOpen = false;

	setBgStateColor(Color(0.3f, 0.3f, 0.3f, 1.0f), CS::NONE);

	//Close all submenus and hide list items
	if(childList)
	{
		childList->closeMenus();
		childList->hide();
	}
}

void MenuListItem::toggleMenu()
{
	if(menuOpen)	closeMenu();
	else			openMenu();
}

bool MenuListItem::menuIsOpen() const
{
	return menuOpen;
}


bool MenuListItem::isLeaf() const
{
	return !childList;
}

void MenuListItem::draw(GlInterface &gl)
{
	if(isolateViewport(gl, false))	//Dont clamp children
	{
		drawBackground(gl);

		if(!isLeaf())
		{
			const float T_HEIGHT = 5.0f,
						T_WIDTH = 5.0f,
						T_OFFSET_X = 4.0f,
						T_OFFSET_Y = (size.y - T_HEIGHT)*(1.0f/2.0f);

			//Draw arrow (triangle)
			const APoint	p1(size.x - T_OFFSET_X - T_WIDTH, T_OFFSET_Y),
							p2(size.x - T_OFFSET_X - T_WIDTH, size.y - T_OFFSET_Y),
							p3(size.x - T_OFFSET_X, size.y*(1.0f/2.0f));

			std::vector<TVertex> vertices {	TVertex(p1, Color(0.4f, 0.5f, 0.6f, 1.0f)),
											TVertex(p2, Color(0.4f, 0.5f, 0.6f, 1.0f)),
											TVertex(p3, Color(0.4f, 0.5f, 0.6f, 1.0f))};
			
			gl.drawShape(GL_TRIANGLES, vertices, false);
		}

		drawChildren(gl);

		restoreViewport(gl);
	}
}

*/