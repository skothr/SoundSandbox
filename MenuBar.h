#ifndef APOLLO_MENU_BAR_H
#define APOLLO_MENU_BAR_H

#include "CompoundControl.h"
#include "SimpleContainers.h"
#include "Label.h"

#include "Callbacks.h"

class MenuBarButton;
class MenuList;
class MenuListItem;

//Represents a menu as a tree.
//	- Children are submenus.
struct MenuTree
{
	std::string label = "";
	voidCallback clickAction = nullptr;

	std::vector<MenuTree> children;

	MenuTree();
	MenuTree(std::string label_, voidCallback click_action);
	void addChild(const MenuTree &child);
};

class MenuBar : public CompoundControl
{
protected:
	std::vector<MenuBarButton*> menuButtons;
	float height;

	virtual void setDefaultBg() override;

	virtual void onLostFocus() override;
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;

public:
	MenuBar(ParentElement *parent_, GuiStateFlags s_flags, const MenuTree &menu, float bar_height);
	virtual ~MenuBar();

	static const GuiPropFlags PROP_FLAGS;
	static const float DEFAULT_BAR_HEIGHT;
	static const int TEXT_HEIGHT;

	void closeAllMenus();
	
	virtual void draw(GlInterface &gl) override;
};

class MenuBarButton : public CompoundControl
{
protected:
	MenuBar		*topMenu = nullptr;
	MenuList	*list = nullptr;
	Label		*label = nullptr;
	
	bool		menuOpen = false;
	
	virtual void onLostFocus() override;

	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;

	virtual void drawBackground(GlInterface &gl) override;
	virtual void setDefaultBg() override;

public:
	MenuBarButton(ParentElement *parent_, GuiStateFlags s_flags, const MenuTree &menu, float x_offset, float height);
	virtual ~MenuBarButton();

	static const float	LABEL_SIDE_PADDING;
	static const AVec	PADDING;

	void openMenu();
	void closeMenu();
	void toggleMenu();
	bool menuIsOpen() const;
	
	virtual void draw(GlInterface &gl) override;

	friend class MenuBar;
};

class MenuList : public CompoundControl
{
protected:
	MenuBar *topMenu = nullptr;
	std::vector<MenuListItem*> listItems;

public:
	MenuList(ParentElement *parent_, MenuBar *top_menu, APoint a_pos, GuiStateFlags s_flags, const std::vector<MenuTree> &menu);
	virtual ~MenuList();

	void closeMenus();

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

#endif	//APOLLO_MENU_BAR_H