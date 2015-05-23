#ifndef WINDOW_MAIN_OVERRIDE_H
#define WINDOW_MAIN_OVERRIDE_H

	#if defined(_WIN32)
		#include <windows.h>

		//Allow standard main function to be implemented elsewhere
		extern int main(int argc, char* argv[]);

		int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
		{
			return main(__argc, __argv);
		}
	#endif // _WIN32

#endif //WINDOW_MAIN_OVERRIDE_H