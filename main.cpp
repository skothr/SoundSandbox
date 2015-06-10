#include "Apollo.h"

#include <thread>

////Memory leak stuff
#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
////


#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#endif	//_USE_MATH_DEFINES


HRes_Clock PHYSICS_CLOCK(true);

bool	running = true,
		paused = false;

void button_click(ImageButton *btn)
{
	std::cout << "Button clicked!\n";
}

void click_test()
{
	std::cout << "Void callback called!\n";
}


void physicsLoop(SandboxWindow *sb);
void drawLoop(SandboxWindow *sb);

//int main(int argc, char* argv[])
//int WINAPI WinMain(HINSTANCE h, HINSTANCE g, LPSTR p, int t)
int main()
{
	initApollo();

	MidiDevice::printDevices();

	if(MidiDevice::numPorts() > 0)
	{
		std::cout << "Which MIDI device would you like to use? (-1 for virtual device)\t";
		std::cin >> midi_port;
	}
	
	AudioDevice::printDevices();


	//Sandbox sb2(Point2i(2500, 50), Vec2i(1000, 500), SAMPLE_RATE);
	//sb.setBackgroundColor(Color(0.15f, 0.15f, 0.15f, 1.0f));

	//---------------------
	//Sandbox result(0);

	//FileSystem::writeSandboxFile(Path("E:/test.sb"), *sb.getSandbox());
	//FileSystem::readSandboxFile(Path("E:/test.sb"), result);
	
	//std::cout << sb.getSandbox()->sampleRate << " --> " << result.sampleRate << "\n\n";
	//---------------------
	
	SandboxWindow *sb = new SandboxWindow(Point2i(100, 50), Vec2i(1500, 850), SAMPLE_RATE, DEFAULT_STATE | SFlags::FOCUSED);
	sb->setUpGui();
	
	//std::thread physicsThread(physicsLoop, &sb);
	//std::thread drawThread(drawLoop, sb);

	while (sb->isOpen())
	{
		//SPACE pauses drawing
		paused = Keyboard::keyDown(Keys::K_SPACE);
		
		sb->handleWindow();

		//PHYSICS//
		if(!paused)
		{
			PHYSICS_CLOCK.nextTimeFrame();
			sb->update(PHYSICS_CLOCK.dt);
		}
		///////////

		//DRAWING
		if(!paused)
		{
			sb->draw();
		}

	}

	//TODO: Stop audio devices before exiting (causing errors i think)

	running = false;
	//physicsThread.join();
	//drawThread.join();

	delete sb;

	cleanupApollo();
	
    _CrtDumpMemoryLeaks();
	return 0;
}

#define PHYSICS_DT 5
/*
void physicsLoop(SandboxWindow *sb)
{
	while(sb->isOpen())
	{
		//PHYSICS
		if(!paused)
		{
			PHYSICS_CLOCK.nextTimeFrame();
			sb->update(PHYSICS_CLOCK.dt);
		}
		Sleep(PHYSICS_DT);
	}
}
*/

void drawLoop(SandboxWindow *sb)
{
	while(sb->isOpen())
	{
		Sleep(10);
	}
}