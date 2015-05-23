#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H

#include <Windows.h>
#include <GL/glew.h>
#include <vector>

class GlContext
{
private:
	static std::vector<GlContext*> contexts;
	static int activeIndex;

	unsigned int thisIndex;

	HDC m_hDC = nullptr;
	HGLRC m_hGLRC = nullptr;
	HPALETTE m_hPalette = nullptr;
	HWND m_hWnd = nullptr;

	void setUpPixelFormat();
	void setUpPalette();

public:
	GlContext();
	~GlContext();

	//static void setActiveContext(GlContext *c);

	void init(HWND hwnd);
	void setActive();
	void swapBuffers();

	bool isActive();
};


#endif	//GL_CONTEXT_H