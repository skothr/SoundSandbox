#ifndef APOLLO_THREADED_OBJECT_H
#define APOLLO_THREADED_OBJECT_H


//Represents an object that 
class ThreadedObject
{
protected:
	bool threadIsRunning = false;

public:
	ThreadedObject();
	virtual ~ThreadedObject();

	void threadStarted();
	bool threadRunning() const;

	//What to do to shut down the thread
	virtual void shutDownThread() = 0;
	//virtual void 
};


#endif	//APOLLO_THREADED_OBJECT_H