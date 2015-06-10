#ifndef APOLLO_MENU_BAR_H
#define APOLLO_MENU_BAR_H

#include "CompoundControl.h"
#include "SimpleContainers.h"
#include "Label.h"

#include "Callbacks.h"
#include "ContextTree.h"

class MenuBarButton;
class ContextWindow;

class MenuBar : public CompoundControl
{
protected:
	std::vector<MenuBarButton*> menuButtons;
	float height;

	virtual void setDefaultBg() override;

	virtual void onLostFocus() override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;

public:
	MenuBar(ParentElement *parent_, GuiStateFlags s_flags, const ContextTree &tree, float bar_height);
	virtual ~MenuBar();

	static const GuiPropFlags PROP_FLAGS;
	static const float DEFAULT_BAR_HEIGHT;
	static const int TEXT_HEIGHT;

	void closeAllMenus(MenuBarButton *except = nullptr);
	bool submenuOpen();		//Return whether a child menu button has its menu open.
	
	virtual void draw(GlInterface &gl) override;
};

class MenuBarButton : public CompoundControl
{
protected:
	MenuBar			*menu = nullptr;
	ContextWindow	*list = nullptr;
	Label			*label = nullptr;
	
	bool		menuOpen = false;
	
	virtual void onLostFocus() override;

	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;

	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;

	virtual void drawBackground(GlInterface &gl) override;
	virtual void setDefaultBg() override;

public:
	MenuBarButton(ParentElement *parent_, GuiStateFlags s_flags, const ContextTree &tree, float x_offset, float height);
	virtual ~MenuBarButton();

	static const float	LABEL_SIDE_PADDING;
	static const AVec	PADDING;

	void openMenu();
	void closeMenu();
	void toggleMenu();
	bool menuIsOpen() const;
	bool submenuOpen();
	
	virtual void draw(GlInterface &gl) override;

	friend class MenuBar;
};
/*
class MenuList : public CompoundControl
{
protected:
	MenuBar *topMenu = nullptr;
	std::vector<MenuListItem*> listItems;

public:
	MenuList(ParentElement *parent_, MenuBar *top_menu, APoint a_pos, GuiStateFlags s_flags, const std::vector<MenuTree> &menu);
	virtual ~MenuList();

	void closeMenus();
	bool submenuOpen();

	virtual void draw(GlInterface &gl) override;

};

class MenuListItem : public CompoundControl
{
protected:
	MenuBar			*topMenu = nullptr;
	MenuList		*parentList = nullptr,
					*childList = nullptr;
	Label			*label = nullptr;

	voidCallback	clickAction;

	bool			menuOpen = false;
	
	virtual void onLostFocus() override;
	
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;

	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;

	virtual void onSizeChanged(AVec d_size) override;

	virtual void setDefaultBg() override;

public:
	MenuListItem(ParentElement *parent_, MenuBar *top_menu, APoint a_pos, GuiStateFlags s_flags, const MenuTree &menu);
	virtual ~MenuListItem();

	static const int	LABEL_SIDE_PADDING,
						HEIGHT;
	
	void openMenu();
	void closeMenu();
	void toggleMenu();
	bool menuIsOpen() const;

	bool isLeaf() const;
	
	virtual void draw(GlInterface &gl) override;
};
*/
#endif	//APOLLO_MENU_BAR_H