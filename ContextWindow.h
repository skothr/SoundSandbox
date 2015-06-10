#ifndef APOLLO_CONTEXT_WINDOW_H
#define APOLLO_CONTEXT_WINDOW_H

#include "GuiElement.h"
#include "Window.h"
#include "CompoundControl.h"
#include "Callbacks.h"

#include "ContextTree.h"

#include <vector>
#include <string>

class WindowWin32;
class ContextWindow;
class Label;

class ContextListItem : public CompoundControl
{
protected:
	ContextWindow	*childWindow = nullptr,
					*parentWindow = nullptr,
					*topWindow = nullptr;

	Label			*label = nullptr;
	
	bool			menuOpen = false;

	voidCallback	clickAction = nullptr;
	
	virtual void onLostFocus() override;
	
	virtual void onMouseDown(APoint m_pos, MouseButton b, bool direct) override;
	virtual void onMouseUp(APoint m_pos, MouseButton b, bool direct) override;

	virtual void onMouseMove(APoint m_pos, AVec d_pos, bool direct) override;

	virtual void onSizeChanged(AVec d_size) override;

	virtual void setDefaultBg() override;

public:
	ContextListItem(ContextWindow *parent_, ContextWindow *top, APoint a_pos, const ContextTree &child_tree, GuiStateFlags s_flags);
	virtual ~ContextListItem();

	static const GuiPropFlags PROP_FLAGS;
	
	void openMenu();
	void closeMenu();
	void toggleMenu();

	bool menuIsOpen() const;

	bool isLeaf() const;
	
	//virtual void update(const Time &dt) override;
	virtual void draw(GlInterface &gl) override;
};

class ContextWindow : public Window
{
protected:
	std::vector<ContextListItem*>	items;
	
	ContextWindow					*topWindow = nullptr;
	Window							*parentWindow = nullptr;

	virtual void onLostFocus() override;
	virtual void onGainedFocus() override;
	virtual void onHide() override;

public:
	ContextWindow(ParentElement *parent_, Window *parent_window, ContextWindow *top, APoint a_pos, const std::vector<ContextTree> &tree, GuiStateFlags s_flags);
	virtual ~ContextWindow();

	static const float ContextWindow::CONTEXT_WIDTH;
	static const GuiPropFlags PROP_FLAGS;

	virtual void setFocus(bool in_focus) override;


	void closeAllMenus(ContextListItem *except = nullptr);
	bool submenuOpen() const;

	virtual void draw() override;
	virtual void draw(GlInterface &gl) override;
};


#endif	//APOLLO_CONTEXT_WINDOW_H