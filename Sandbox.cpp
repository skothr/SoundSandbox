#include "Sandbox.h"

#include "AUtility.h"

#include "Container.h"
#include "NodeGraphControl.h"
#include "ProjectTrackDisplay.h"
#include "PropertiesDisplay.h";

#include "Buttons.h"

#include "RenderNode.h"
//#include "TimeMapNode.h"
#include "IONodes.h"
#include "TrackNodes.h"

#include <unordered_map>

//true --> sandbox, false --> project
#define TEST_SANDBOX false
//#define TEST_SANDBOX true


/////SANDBOX/////

Sandbox::Sandbox(int sample_rate)
	: sampleRate(sample_rate)
{ }

Sandbox::Sandbox(const Sandbox &other)
	: Sandbox(other.sampleRate)
{ }

Sandbox& Sandbox::operator=(const Sandbox &other)
{
	sampleRate = other.sampleRate;
	return *this;
}

void Sandbox::update(Time dt)
{
	graph.update(dt);
}

void Sandbox::createNode(NodeType t, SandboxWindow *window)
{
	switch(t)
	{
	case NType::AUDIO_TRACK:
		graph.addNode(new AudioTrackNode(sampleRate), Point2f(0.0f, 0.0f));
		break;
	case NType::MIDI_TRACK:
		graph.addNode(new MidiTrackNode(), Point2f(0.0f, 0.0f));
		break;
	case NType::AUDIO_MOD_TRACK:
		graph.addNode(new AudioModTrackNode(nullptr), Point2f(0.0f, 0.0f));
		break;
	case NType::MIDI_MOD_TRACK:
		graph.addNode(new MidiModTrackNode(nullptr), Point2f(0.0f, 0.0f));
		break;
	case NType::RENDER:
		graph.addNode(new RenderNode(sampleRate), Point2f(0.0f, 0.0f));
		break;
	case NType::SPEAKER:
		graph.addNode(new SpeakerNode(0, sampleRate, 1, window->trackDisp), Point2f(0.0f, 0.0f));
		break;
	//case NType::TIME_MAP:
	//	graph.addNode(new TimeMapNode(sampleRate), Point2f(0.0f, 0.0f));
	//	break;
	case NType::INSTRUMENT:
		graph.addNode(new InstrumentNode(), Point2f(0.0f, 0.0f));
		break;
	case NType::MIDI_DEVICE:
		//TODO: Make port choosable
		graph.addNode(new MidiDeviceNode(0), Point2f(0.0f, 0.0f));
		break;
	default:
		break;
	}
}

//void Sandbox::updateDesc()
//{
//	objDesc = (objDesc ? objDesc : (ObjDesc*)(new SandboxDesc()));
//	NDesc *desc = dynamic_cast<NDesc*>(objDesc);
//}


/////SANDBOX WINDOW/////

SandboxWindow::SandboxWindow(Point2i pos_, Vec2i size_, int sample_rate, GuiStateFlags s_flags)
	: GuiElement(nullptr, pos_, size_, GuiProps(s_flags, PROP_FLAGS)),
		Window("Sandbox", pos_, size_, s_flags), sb(new Sandbox(sample_rate)),
		cursor(sample_rate, AUDIO_CHUNK_SIZE, 1)
{
	setAllBgStateColors(Color(0.15f, 0.15f, 0.15f, 1.0f));
	setBgStateColor(Color(1.0f, 0.0f, 1.0f, 1.0f), CS::INVALID);

	if(TEST_SANDBOX)
		setLayout(SBLayout::SANDBOX);
	else
		setLayout(SBLayout::PROJECT);
}

SandboxWindow::~SandboxWindow()
{
	clearChildren();

	//if(tmNode)
	//	tmNode->setPlaying(false);

	if(stream)
		stream->getDevice()->stop();

	AU::safeDelete(menu);
	AU::safeDelete(mainArea);
	
	AU::safeDelete(graphDisp);
	AU::safeDelete(trackDisp);
	AU::safeDelete(propDisp);

	AU::safeDelete(playPause);
	AU::safeDelete(reset);

	AU::safeDelete(sb);
}

void SandboxWindow::setUpGui()
{
	initMenu();

	//Create elments
	menu = new MenuBar(this, DEFAULT_STATE_FLOAT, menuTree, (float)MENU_BAR_WIDTH);
	mainArea = new Area(this, Point2i(0, MENU_BAR_WIDTH), Vec2i(size.x, size.y - MENU_BAR_WIDTH), DEFAULT_STATE);

	graphDisp = new NodeGraphControl(mainArea, APoint(50, 50), Vec2i(size.x - 10, 400), DEFAULT_STATE, &sb->graph);
	trackDisp = new ProjectTrackDisplay(mainArea, APoint(50, 50), AVec(100, 400), DEFAULT_STATE, &cursor);
	propDisp = new PropertiesDisplay(mainArea, APoint(850.0f, 0.0f), AVec(size.x - 850.0f, size.y), DEFAULT_STATE, trackDisp);

	playPause = new ImageButton(mainArea, Point2i(), DEFAULT_STATE, "PLAY/PAUSE");
	reset = new ImageButton(mainArea, Point2i(), DEFAULT_STATE, "RESET");

	graphDisp->setPropDisplay(propDisp);

	//Attach main area
	mainArea->attachTo(this, Side::LEFT, 0);
	mainArea->attachTo(this, Side::RIGHT, 0);
	mainArea->attachTo(this, Side::TOP, MENU_BAR_WIDTH);
	mainArea->attachTo(this, Side::BOTTOM, 0);
	
	//Attach Buttons
	playPause->attachTo(mainArea, Side::LEFT, 25);
	playPause->attachTo(mainArea, Side::TOP, 5);
	
	reset->attachTo(playPause, Side::LEFT, 5);
	reset->attachTo(mainArea, Side::TOP, 5);

	//Attach prop display
	propDisp->attachTo(mainArea, Side::RIGHT, 0);
	propDisp->attachTo(mainArea, Side::TOP, 0);
	propDisp->attachTo(mainArea, Side::BOTTOM, 0);
	
	//Attach graph display
	graphDisp->attachTo(mainArea, Side::LEFT, 5);
	graphDisp->attachTo(mainArea, Side::BOTTOM, 5);
	graphDisp->attachTo(propDisp, Side::RIGHT, 5);
	graphDisp->attachTo(trackDisp, Side::TOP, 5);
	
	//Attach time map display
	trackDisp->attachTo(mainArea, Side::LEFT, 5);
	trackDisp->attachTo(propDisp, Side::RIGHT, 5);
	trackDisp->attachTo(playPause, Side::TOP, 5);

	//Set click functions
	playPause->setClickFunction(std::bind(&SandboxWindow::togglePlay, this));
	reset->setClickFunction(std::bind(&SandboxWindow::resetCursor, this));

	//Add elements
	//addFront(menu);
	//addBody(mainArea);
	//mainArea->addBody(propDisp);
	//mainArea->addBody(graphDisp);
	//mainArea->addBody(timeMapDisp);
	//mainArea->addBody(playPause);
	//mainArea->addBody(reset);

	updateResources();
	
	stream->setTrackDisplay(trackDisp);


	//NodeTree *nt = graph.getNodeTree();
	//nt->print();
	//std::cout << "\n\n";
	//delete nt;
}

void SandboxWindow::setLayout(SandboxLayout layout)
{
	sb->graph.reset();

	if(propDisp)
		propDisp->setActiveNode(nullptr);
	
	const std::unordered_map<Node*, Point2f> *g_nodes = nullptr;

	switch(layout)
	{
	case SBLayout::EMPTY:
		//No nodes
		stream = nullptr;
		//tmNode = nullptr;
		break;

	case SBLayout::SANDBOX:
		sb->graph.setPreset(NodeGraphPreset::SANDBOX, sb->sampleRate);

		g_nodes = sb->graph.getNodes();

		for(auto n : *g_nodes)
		{
			if(dynamic_cast<SpeakerNode*>(n.first))
				stream = dynamic_cast<SpeakerNode*>(n.first);

		}

		//stream = dynamic_cast<SpeakerNode*>(sb->graph.);
		//tmNode = nullptr;
		break;

	case SBLayout::PROJECT:
		sb->graph.setPreset(NodeGraphPreset::PROJECT, sb->sampleRate);
		
		g_nodes = sb->graph.getNodes();

		for(auto n : *g_nodes)
		{
			if(dynamic_cast<SpeakerNode*>(n.first))
				stream = dynamic_cast<SpeakerNode*>(n.first);
			//else if(dynamic_cast<TimeMapNode*>(n.first))
			//	tmNode = dynamic_cast<TimeMapNode*>(n.first);

		}
		//stream = dynamic_cast<SpeakerNode*>(sb->graph.getNode(4));
		//tmNode = dynamic_cast<TimeMapNode*>(sb->graph.getNode(5));
		break;

	default:
		std::cout << "ERROR -- Not a valid layout.\n";
		break;
	};

	//if(trackDisp)
	//	timeMapDisp->setNode(tmNode);

	if(graphDisp)
		graphDisp->clearGraph();	//Reset stuff using new node graph
}


void SandboxWindow::initMenu()
{
	MenuTree file("FILE", nullptr);
		MenuTree file1("New Project", nullptr);
			MenuTree file11("Item 1Ai", nullptr);
					file11.addChild(MenuTree("Item 1Aia", nullptr));
					file11.addChild(MenuTree("Item 1Aib", nullptr));
			file1.addChild(file11);
			file1.addChild(MenuTree("Item 1Aii", nullptr));
			file1.addChild(MenuTree("Item 1Aiii", nullptr));
		file.addChild(file1);
		file.addChild(MenuTree("Open Project", nullptr));
		file.addChild(MenuTree("Close Project", nullptr));
		MenuTree file2("LoadPreset", nullptr);
			file2.addChild(MenuTree("EMPTY", std::bind(&SandboxWindow::setLayout, this, SBLayout::EMPTY)));
			file2.addChild(MenuTree("SANDBOX", std::bind(&SandboxWindow::setLayout, this, SBLayout::SANDBOX)));
			file2.addChild(MenuTree("PROJECT", std::bind(&SandboxWindow::setLayout, this, SBLayout::PROJECT)));
		file.addChild(file2);
	menuTree.addChild(file);

	MenuTree edit("EDIT", nullptr);
		MenuTree edit1("Item 1B", nullptr);
			edit1.addChild(MenuTree("Item 1Bi", nullptr));
			edit1.addChild(MenuTree("Item 1Bii", nullptr));
			edit1.addChild(MenuTree("Item 1Biii", nullptr));
		edit.addChild(edit1);
		edit.addChild(MenuTree("Item 2B", nullptr));
		edit.addChild(MenuTree("Item 3B", nullptr));
	menuTree.addChild(edit);
	
	MenuTree create("CREATE", nullptr);
		MenuTree node("Node", nullptr);
			node.addChild(MenuTree("Audio Buffer", std::bind(&Sandbox::createNode, sb, NType::AUDIO_TRACK, this)));
			node.addChild(MenuTree("Midi Buffer", std::bind(&Sandbox::createNode, sb, NType::MIDI_TRACK, this)));
			node.addChild(MenuTree("Midi Device", std::bind(&Sandbox::createNode, sb, NType::MIDI_DEVICE, this)));
			node.addChild(MenuTree("Speaker", std::bind(&Sandbox::createNode, sb, NType::SPEAKER, this)));
			node.addChild(MenuTree("Render", std::bind(&Sandbox::createNode, sb, NType::RENDER, this)));
			node.addChild(MenuTree("Waveform", std::bind(&Sandbox::createNode, sb, NType::INSTRUMENT, this)));
		create.addChild(node);
	menuTree.addChild(create);

	MenuTree project("PROJECT", nullptr);
		project.addChild(MenuTree("Item 1C", nullptr));
		project.addChild(MenuTree("Item 2C", nullptr));
		project.addChild(MenuTree("Item 3C", nullptr));
	menuTree.addChild(project);

	MenuTree view("VIEW", nullptr);
		view.addChild(MenuTree("Item 1D", nullptr));
		view.addChild(MenuTree("Item 2D", nullptr));
		view.addChild(MenuTree("Item 3D", nullptr));
	menuTree.addChild(view);
}

void SandboxWindow::togglePlay()
{
	cursor.active = !cursor.active;
	trackDisp->updateCursorPos();
}

void SandboxWindow::resetCursor()
{
	cursor.active = false;
	cursor.setTimeSamples(0);
	
	trackDisp->moveViewOffset(AVec(-trackDisp->getViewOffset().x, 0.0f));
	trackDisp->updateCursorPos();
}

Sandbox* SandboxWindow::getSandbox()
{
	return sb;
}

void SandboxWindow::setSandbox(const Sandbox &new_sb)
{
	*sb = new_sb;
}

void SandboxWindow::update(double dt)
{
	ParentElement::update(dt);
	sb->update(dt);
}


void SandboxWindow::draw(GlInterface &gl)
{
	Window::draw(gl);
}

void SandboxWindow::draw()
{
	Window::draw();
}