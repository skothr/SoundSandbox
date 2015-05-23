#include <GL/glew.h>
#include "ApolloGui.h"

#include "Waveform.h"
#include "Samplers.h"
#include "Sandbox.h"

#include "Mouse.h"
#include "Keyboard.h"

#include "MidiDevice.h"
#include "AudioDevice.h"

#include "FileSystem.h"

#include <thread>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#endif	//_USE_MATH_DEFINES

#define SAMPLE_RATE 44100

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

void setVSync(bool vsync_on)
{
	typedef bool (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
	wglSwapIntervalEXT = 
		(PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT)
	wglSwapIntervalEXT(vsync_on ? 1 : 0);
}

void physicsLoop(SandboxWindow *sb);

//int main(int argc, char* argv[])
//int WINAPI WinMain(HINSTANCE h, HINSTANCE g, LPSTR p, int t)
int main()
{
	FileSys::setDefaultBaseDir();

	WindowClass::setDefaultFillColor(0.15, 0.15, 0.15);
	setA4(440.0);//432.0);

	Clock::initGlobalTime();

	MidiDevice::initDevices();
	MidiDevice::printDevices();

	if(MidiDevice::numPorts() > 0)
	{
		std::cout << "Which MIDI device would you like to use? (-1 for virtual device)\t";
		std::cin >> midi_port;
	}

	AudioDevice::initDevices();
	AudioDevice::printDevices();

	SandboxWindow sb(Point2i(100, 50), Vec2i(1500, 850), SAMPLE_RATE, DEFAULT_STATE);
	//Sandbox sb2(Point2i(2500, 50), Vec2i(1000, 500), SAMPLE_RATE);
	//sb.setBackgroundColor(Color(0.15f, 0.15f, 0.15f, 1.0f));

	//---------------------
	//Sandbox result(0);

	//FileSystem::writeSandboxFile(Path("E:/test.sb"), *sb.getSandbox());
	//FileSystem::readSandboxFile(Path("E:/test.sb"), result);
	
	//std::cout << sb.getSandbox()->sampleRate << " --> " << result.sampleRate << "\n\n";
	//---------------------
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glPointSize(10.0f);
	
	setVSync(false);

	loadGuiResources();
	sb.setUpGui();

	std::thread physicsThread(physicsLoop, &sb);


	while (sb.isOpen())
	{
		//SPACE pauses drawing
		paused = Keyboard::keyDown(Keys::K_SPACE);

		sb.handleWindow();

		//DRAWING
		if(!paused)
			sb.draw();
	}

	running = false;

	physicsThread.join();

	cleanUpGui();
	
	return 0;
}

#define PHYSICS_DT 5

void physicsLoop(SandboxWindow *sb)
{
	while(running)
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