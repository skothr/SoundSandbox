#include "Container.h"

/////CONTAINER/////

Container::Container(GuiProps flags)
	: ActiveElement(flags)
{ }

Container::~Container()
{ }

void Container::update(double dt)
{
	ParentElement::update(dt);
	ActiveElement::update(dt);
}


/////STATIC CONSTAINER/////

StaticContainer::StaticContainer()
{ }

StaticContainer::~StaticContainer()
{ }