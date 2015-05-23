#ifndef APOLLO_GUI_H
#define APOLLO_GUI_H


#include "GuiElement.h"
#include "ActiveElement.h"
#include "StaticElement.h"

#include "Container.h"
#include "Control.h"
#include "Display.h"
#include "TransformContainer.h"
#include "GraphDisplay.h"

//TEMP
//#include "TestGraphDisplay.h"
//

#include "SimpleContainers.h"
#include "ScrollArea.h"
#include "MenuBar.h"

#include "Label.h"
#include "Buttons.h"
#include "Checkbox.h"
#include "RadioButtonGroup.h"
#include "CollapseList.h"

#include "Window.h"
#include "Text.h"
#include "Texture.h"

#include "Mouse.h"
#include "Keyboard.h"


inline void loadGuiResources()
{
	Mouse::updateStates();
	Keyboard::updateStates();

	Font::loadResources();
	Text::loadResources();
	Texture::loadResources();

	Label::initResources();

	ImageButton::loadResources();
}

inline void cleanUpGui()
{
	Texture::cleanUp();
	WindowWin32::cleanUp();
}


#endif	//APOLLO_GUI_H