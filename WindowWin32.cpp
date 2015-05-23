#include "WindowWin32.h"

#include <Windows.h>

#include "AUtility.h"
#include "Window.h"

#include <sstream>

//Converts an std::string to an std::wstring
std::wstring toWString(std::string str)
{
	return std::wstring(str.begin(), str.end());
}
//Converts an std::wstring to an std::string
std::string toString(std::wstring wstr)
{
	return std::string(wstr.begin(), wstr.end());
}

std::vector<WindowClass*> WindowWin32::classes;

WindowWin32::WindowWin32(Window *owner_, Vec2i size, std::string title, WindowStyle &style)
	: m_lastSize(size), owner(owner_)
{
	//Get position and size
	HDC screenDC = GetDC(nullptr);
	int x = (GetDeviceCaps(screenDC, HORZRES) - static_cast<int>(size.x))/2;
	int y = (GetDeviceCaps(screenDC, VERTRES) - static_cast<int>(size.y))/2;

	//Choose window style
	windowStyle = style.getWindowStyle();
	DWORD winExStyle = style.getWindowExStyle();
	//Default style:
	//DWORD winStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

	//Adjust width and height to match client area with size
	RECT r = {0, 0, size.x, size.y};
	AdjustWindowRect(&r, windowStyle, false);
	size.x = r.right - r.left;
	size.y = r.bottom - r.top;

	//Window class
	windowClass = getClass(style);
	if(!windowClass)
		return;	//ERROR
	
	LPCWSTR szClassName = windowClass->windowClass.lpszClassName;

	//Create window
	//m_handle = CreateWindowEx(winExStyle, szClassName, toWString(title).c_str(), winStyle, x, y, size.x, size.y, nullptr, nullptr, GetModuleHandle(nullptr), this);
	m_handle = CreateWindow(szClassName, toWString(title).c_str(), windowStyle, x, y, size.x, size.y, nullptr, nullptr, GetModuleHandle(nullptr), this);

	context = new GlContext();
	context->init(m_handle);
}

WindowWin32::~WindowWin32()
{
	close();

	if(m_icon)
		DestroyIcon(m_icon);

	if(m_handle)
		DestroyWindow(m_handle);
}

void WindowWin32::cleanUp()
{
	//Unregister all classes
	for(unsigned int i = 0; i < classes.size(); i++)
		classes[i]->deregisterClass();

	classes.clear();
}


WindowClass* WindowWin32::getClass(WindowStyle &style)
{
	UINT class_style = style.getClassStyle();

	for(unsigned int i = 0; i < classes.size(); i++)
	{
		//TODO: other conditions too (icon, cursor, etc)
		if(classes[i]->windowClass.style == class_style)
			return classes[i];
	}

	//Register new class
	return createWindowClass(style);
}


bool WindowWin32::classNameFree(std::wstring class_name)
{
	for(unsigned int i = 0; i < classes.size(); i++)
	{
		if(std::wcscmp(classes[i]->windowClass.lpszClassName, class_name.c_str()))
			return false;
	}

	return true;
}

WindowClass* WindowWin32::createWindowClass(WindowStyle &style, std::string class_name)
{
	WNDCLASSEX wc;

	std::wstring w_class_name;

	if(class_name == "")
	{
		int i = 0;
		w_class_name = L"WindowClass0";
		std::wstringstream ss(w_class_name);
		
		while(!classNameFree(w_class_name))
		{
			i++;
			w_class_name.clear();
			ss << L"WindowClass" << i;
		}
	}
	else
	{
		w_class_name = toWString(class_name);
	}

	//Register window class
	//TODO: Make WindowStyle style able to affect more areas (if necessary)
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = style.getClassStyle();
	wc.lpfnWndProc = &WindowWin32::gEventCallback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//(handled by WindowClass)
	//wc.hbrBackground = CreateSolidBrush(RGB(defaultFillColor[0], defaultFillColor[1], defaultFillColor[2]));//(HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = w_class_name.c_str();
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	

	WindowClass *pWc = new WindowClass(wc);
	classes.push_back(pWc);
	AStatus status = pWc->registerClass();

	if(status == AS::A_SUCCESS)
	{
		return pWc;
	}
	else
	{
		std::cout << status;
		return nullptr;
	}
}

Vec2i WindowWin32::getSize()
{
	RECT r;
	GetClientRect(m_handle, &r);
	return Vec2i(r.right - r.left, r.bottom - r.top);
}

Point2i WindowWin32::getPosition()
{
	RECT wr;
	GetWindowRect(m_handle, &wr);

	return Point2i(wr.left, wr.top);
}

Point2i WindowWin32::getClientPosition()
{
	POINT p;
	p.x = 0;
	p.y = 0;

	ClientToScreen(m_handle, &p);

	return Point2i(p.x, p.y);
}

Point2i WindowWin32::getClientMousePos()
{
	POINT p;
	p.x = Mouse::getPos().x;
	p.y = Mouse::getPos().y;

	if(!ScreenToClient(m_handle, &p))
	{
		std::cout << "Failed to adjust mouse screen pos to client!\n";
	}

	return Point2i(p.x, p.y);
}

void WindowWin32::setSize(Vec2i size)
{
	RECT r = {0, 0, static_cast<long>(size.x), static_cast<long>(size.y)};

	AdjustWindowRect(&r, GetWindowLong(m_handle, GWL_STYLE), false);
	int w = r.right - r.left;
	int h = r.bottom - r.top;

	SetWindowPos(m_handle, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
}

void WindowWin32::setPosition(Point2i pos)
{
	SetWindowPos(m_handle, nullptr, pos.x, pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WindowWin32::setVisible(bool visible)
{
	ShowWindow(m_handle, visible ? SW_SHOW : SW_HIDE);
}

void WindowWin32::setActive()
{
	context->setActive();
}

void WindowWin32::setFocus(bool in_focus)
{
	inFocus = in_focus;
}
bool WindowWin32::isInFocus()
{
	return inFocus;
}


void WindowWin32::pushEvent(Event e)
{
	events.push_back(e);
}

//pop event off FRONT
//TODO: replace this shit with an actual queue...
Event WindowWin32::popEvent()
{
	Event e;
	if(events.size() > 0)
	{
		e = events[0];
		events.erase(events.begin());
	}
	else
	{
		e.type = Events::INVALID;
	}

	return e;
}

void WindowWin32::handleEvents()
{
	MSG msg;
	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool WindowWin32::checkEvent(Event &e)
{
	handleEvents();
	Event ev = popEvent();

	e = ev;
	return (e.type != Events::INVALID);
}

void WindowWin32::processEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(!m_handle) return;

	Event e;
	Vec2i currSize;
	Point2i winPos;
	RECT r;

	//TODO: fix this mess (WM_?BUTTONUP wParam isnt the button that had the event, its a list of buttons that are still down!)
	// Use downButtons (member) instead
	bool btn_up_skip = false;

	switch(msg)
	{
	case WM_CLOSE:
		e.type = Events::CLOSED;
		owner->handleEvent(e);
		//pushEvent(e);
		break;

	case WM_DESTROY:
		close();
		break;

	case WM_PAINT:
		break;

		/*
	case WM_SETFOCUS:
		setFocus(true);
		e.type = EVENT_GAINED_FOCUS;
		e.focus.in_focus = true;
		pushEvent(e);
		break;

	case WM_KILLFOCUS:
		setFocus(false);
		e.type = EVENT_LOST_FOCUS;
		e.focus.in_focus = false;
		pushEvent(e);
		break;
		*/
		
	case WM_ACTIVATE:
		setFocus(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE);
		e.type = (inFocus) ? Events::GAINED_FOCUS : Events::LOST_FOCUS;
		e.focus.in_focus = inFocus;
		owner->handleEvent(e);
		//pushEvent(e);
		break;
		
	case WM_SIZE:
		/*
		winPos = getPosition();
		currSize = Vec2i(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if(wParam != SIZE_MINIMIZED && (m_lastSize.x != currSize.x || m_lastSize.y != currSize.y))
		{
			//Update last size
			m_lastSize = currSize;
			//Push event
			e.type = Events::RESIZED;
			e.size.w = currSize.x;
			e.size.h = currSize.y;
			e.size.x = winPos.x;
			e.size.y = winPos.y;
			pushEvent(e);
		}
		*/

		if(wParam == SIZE_MAXIMIZED)
			e.type = Events::MAXIMIZED;
		else if(wParam == SIZE_MINIMIZED)
			e.type = Events::MINIMIZED;
		else
			e.type = Events::RESIZED;
		
		e.size.w = GET_X_LPARAM(lParam);
		e.size.h =  GET_Y_LPARAM(lParam);

		//e.size.x = r.left;
		//e.size.y = r.top;
		//e.size.w = r.right - r.left;
		//e.size.h = r.bottom - r.top;

		owner->handleEvent(e);

		break;
		
	case WM_EXITSIZEMOVE:
		break;
	case WM_SIZING:
		//TODO: Use lParam values instead of re-getting size/position (need to convert rect from window to screen)
		//AdjustWindowRect(*(LPRECT*)lParam, windowStyle, TRUE);

		r = *(RECT*)lParam;

		//winPos = Point2i(r.left, r.top);
		//currSize = Vec2i(r.right - r.left, r.bottom - r.top);

		//std::cout << winPos << ", " << currSize << "\n";

		winPos = getPosition();//Point2i(r->left, r->top);
		currSize = getSize();//Vec2i(r.right - r.left, r.bottom - r.top);
		//Redraw window (if function provided)
		if(resizeFunc) resizeFunc(winPos, currSize);
		

		break;

	case WM_XBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetCapture(m_handle);

		e.type = Events::MOUSEBUTTON_DOWN;
		e.mouseButton.button = convertWindowsMouseButton(wParam);

		e.mouseButton.x = GET_X_LPARAM(lParam);
		e.mouseButton.y = GET_Y_LPARAM(lParam);
		owner->handleEvent(e);

		//pushEvent(e);
		break;
		
	case WM_XBUTTONUP:
		//e.mouseButton.button = MB::X1;			//TODO: How to get which x button?
		//btn_up_skip = true;
	case WM_LBUTTONUP:
		//if(!btn_up_skip)
		//	e.mouseButton.button = MB::LEFT;
		//btn_up_skip = true;
	case WM_MBUTTONUP:
		//if(!btn_up_skip)
		//	e.mouseButton.button = MB::MIDDLE;
		//btn_up_skip = true;
	case WM_RBUTTONUP:
		//if(!btn_up_skip)
		//	e.mouseButton.button = MB::RIGHT;
		ReleaseCapture();
		
		e.type = Events::MOUSEBUTTON_UP;
		e.mouseButton.button = convertWindowsMouseButton(wParam);

		e.mouseButton.x = GET_X_LPARAM(lParam);
		e.mouseButton.y = GET_Y_LPARAM(lParam);
		owner->handleEvent(e);

		//pushEvent(e);
		break;

	case WM_MOUSEMOVE:
		e.type = Events::MOUSEMOVED;

		e.mouseMove.x = GET_X_LPARAM(lParam);
		e.mouseMove.y = GET_Y_LPARAM(lParam);

		owner->handleEvent(e);
		//pushEvent(e);
		break;

	case WM_MOUSEWHEEL:
		e.type = Events::MOUSEWHEEL_MOVED;
		e.mouseWheel.delta_y = (float)GET_WHEEL_DELTA_WPARAM(wParam)/(float)WHEEL_DELTA;
		e.mouseWheel.delta_x = 0.0f;

		winPos = getClientPosition();
		e.mouseWheel.x = GET_X_LPARAM(lParam) - winPos.x;
		e.mouseWheel.y = GET_Y_LPARAM(lParam) - winPos.y;

		//std::cout << e.mouseWheel.x << ", " << e.mouseWheel.y << "\n";
		
		owner->handleEvent(e);
		//pushEvent(e);
		break;

	case WM_MOUSEHWHEEL:
	case WM_HSCROLL:
		std::cout << "HORIZONTAL SCROLL!!!\n";
		break;

	case WM_KEYDOWN:
		e.type = Events::KEY_DOWN;
		e.key.keyCode = convertWindowsKeyCode(wParam, lParam);
		e.key.control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
		e.key.alt = HIWORD(GetAsyncKeyState(VK_MENU)) != 0;
		e.key.shift = HIWORD(GetAsyncKeyState(VK_SHIFT)) != 0;
		e.key.system = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
		owner->handleEvent(e);
		//pushEvent(e);
		break;

	case WM_KEYUP:
		e.type = Events::KEY_UP;
		e.key.keyCode = convertWindowsKeyCode(wParam, lParam);
		e.key.control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
		e.key.alt = HIWORD(GetAsyncKeyState(VK_MENU)) != 0;
		e.key.shift = HIWORD(GetAsyncKeyState(VK_SHIFT)) != 0;
		e.key.system = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
		owner->handleEvent(e);
		//pushEvent(e);
		break;

	default:
		break;
	}
}

void WindowWin32::close()
{
	AU::safeDelete(context);
}

void WindowWin32::setResizeFunction(ResizeCallback resize_func)
{
	resizeFunc = resize_func;
}

void WindowWin32::swapBuffers()
{
	context->swapBuffers();
}

LRESULT CALLBACK WindowWin32::gEventCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//std::cout << msg << "\n";

	if(msg == WM_CREATE)
	{
		//Get instance of WindowWin32
		LONG_PTR window = (LONG_PTR)reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
		
		//Set as user data of window
		SetWindowLongPtr(hwnd, GWLP_USERDATA, window);
	}

	//Get pointer to this WindowWin32 class (stored as GWLP_USERDATA)
	WindowWin32 *window = hwnd ? reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) : nullptr;

	//Call function for that window
	if(window)
		window->processEvent(msg, wParam, lParam);

	//Make sure OS doesn't automatically close window
	if(msg == WM_CLOSE)
		return FALSE;

	//Dont steal focus with alt or f10 (??)
	if(msg == WM_SYSCOMMAND && wParam == SC_KEYMENU)
		return FALSE;

	//Dont erase background while resizing (prevents flickering)
	if(msg == WM_ERASEBKGND)
		return TRUE;

	return DefWindowProc(hwnd, msg, wParam, lParam);
}



/////WINDOW CLASS

short WindowClass::defaultFillColor[3] = {0, 0, 0};

WindowClass::WindowClass(WNDCLASSEX &wc)
	: windowClass(wc)
{
	//Init fill color
	fillColor[0] = defaultFillColor[0];
	fillColor[1] = defaultFillColor[1];
	fillColor[2] = defaultFillColor[2];

	//Create stable location for class name
	className = windowClass.lpszClassName;
	windowClass.lpszClassName = className.c_str();

	windowClass.hbrBackground = CreateSolidBrush(RGB(fillColor[0], fillColor[1], fillColor[2]));
}

void WindowClass::setDefaultFillColor(float r, float g, float b)
{
	defaultFillColor[0] = static_cast<short>(r*255.0f);
	defaultFillColor[1] = static_cast<short>(g*255.0f);
	defaultFillColor[2] = static_cast<short>(b*255.0f);
}

AStatus WindowClass::setFillColor(float r, float g, float b)
{
	AStatus status;

	fillColor[0] = static_cast<short>(r*255.0f);
	fillColor[1] = static_cast<short>(g*255.0f);
	fillColor[2] = static_cast<short>(b*255.0f);
	
	windowClass.hbrBackground = CreateSolidBrush(RGB(fillColor[0], fillColor[1], fillColor[2]));

	if(registered)
		SetClassLongPtr(FindWindow(windowClass.lpszClassName, nullptr), GCLP_HBRBACKGROUND, (LONG)windowClass.hbrBackground);

	return status;
}

AStatus WindowClass::registerClass()
{
	AStatus status;

	if(registered)
	{
		status.setError(AS::ErrorType::GENERAL, "Window class has already been registered.");
	}
	else if(!RegisterClassEx(&windowClass))
	{
		status.setError(AS::ErrorType::GENERAL, "Failed to register window class!");
	}
	else
	{
		registered = true;
	}
	
	return status;
}

AStatus WindowClass::deregisterClass()
{
	AStatus status;

	if(!UnregisterClass(windowClass.lpszClassName, GetModuleHandle(nullptr)))
	{
		status.setError(AS::ErrorType::GENERAL, "Failed to deregister window class!");
	}
	else
	{
		registered = false;
	}

	return status;
}