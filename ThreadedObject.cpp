#include "ThreadedObject.h"


/////THREADED OBJECT/////

ThreadedObject::ThreadedObject()
{ }

ThreadedObject::~ThreadedObject()
{ }

void ThreadedObject::threadStarted()
{
	threadIsRunning = true;
}

bool ThreadedObject::threadRunning() const
{
	return threadIsRunning;
}