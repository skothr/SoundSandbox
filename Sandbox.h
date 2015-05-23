#ifndef APOLLO_SANDBOX_H
#define APOLLO_SANDBOX_H

#include "Window.h"
#include "Vector.h"
#include "NodeGraph.h"
#include "MenuBar.h"

#include "Cursor.h"

enum class SandboxLayout
{
	INVALID = -1,

	EMPTY = 0,
	SANDBOX = 1,
	PROJECT
};
typedef SandboxLayout SBLayout;

class NodeGraphControl;
class ProjectTrackDisplay;
class PropertiesDisplay;
class ScrollArea;
class SpeakerNode;
class TimeMapNode;

class ImageButton;

class SandboxWindow;

struct Sandbox// : public Saveable
{
	NodeGraph	graph;
	int			sampleRate = 0;

	Sandbox(int sample_rate);
	Sandbox(const Sandbox &other);
	Sandbox(const ObjDesc &obj_desc);
	~Sandbox() = default;

	Sandbox& operator=(const Sandbox &other);

	void update(Time dt);
	void createNode(NodeType t, SandboxWindow *window);
	
//protected:
//	virtual void updateDesc() override;
};

class SandboxWindow : public Window
{
private:
	Sandbox				*sb = nullptr;

	MenuBar				*menu = nullptr;

	Area				*mainArea = nullptr;
	NodeGraphControl	*graphDisp = nullptr;
	ProjectTrackDisplay	*trackDisp = nullptr;
	PropertiesDisplay	*propDisp = nullptr;
	
	ImageButton			*playPause = nullptr,
						*reset = nullptr;

	SpeakerNode			*stream = nullptr;

	Cursor				cursor;
	
	MenuTree			menuTree;
	void initMenu();

public:
	static const int MENU_BAR_WIDTH = 25;

	SandboxWindow(Point2i pos, Vec2i size, int sample_rate, GuiStateFlags s_flags);
	~SandboxWindow();
	
	//static void loadResources();

	void setUpGui();
	void setLayout(SandboxLayout layout);

	void togglePlay();
	void resetCursor();

	Sandbox* getSandbox();
	void setSandbox(const Sandbox &new_sb);

	virtual void update(Time dt) override;
	virtual void draw(GlInterface &gl) override;
	void draw();

	friend class Sandbox;
};

#endif	//APOLLO_SANDBOX_H