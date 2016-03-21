/*
 * Nunki (simple rendering engine)
 * Copyright (C) 2015 Canio Massimo Tristano <massimo.tristano@gmail.com>.
 * For licensing info see LICENSE.
 */

#include "nu_window.h"
#include "nu_libs.h"
#include "nu_base.h"
#include "Windows.h"

static struct
{
	bool initialized;
	HWND dummyWindowHandle;
	HDC dummyWindowHDC;
} gWindowModule;

#define EnforceInitialized() nEnforce(gWindowModule.initialized, "Window module not initialized.");

typedef struct NuWindowImpl
{
	HWND handle;
	uint width;
	uint height;
	NuWindowEvent *event;
	int oldCursor[2];
	uint fullscreen : 1;
	uint isCursorHidden : 1;
} Window;

static const char* gClassName = TEXT("NunkiWindowClass");
static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM lParam);

NuResult nuCreateWindow(NuWindowCreateInfo const *info, NuAllocator* allocator, NuWindow *outWindow)
{
	EnforceInitialized();
	allocator = nGetAllocator(allocator);

	nEnforce(info, "Null info provided.");
	nEnforce(outWindow, "Null handle provided.");
	
	*outWindow = NULL;
	HWND handle = 0;

	/* Calculate window dimensions */
	uint x = (GetSystemMetrics(SM_CXSCREEN) - info->width) / 2;
	uint y = (GetSystemMetrics(SM_CYSCREEN) - info->height) / 2;

	RECT winrect;
	winrect.left = x;
	winrect.right = x + info->width;
	winrect.top = y;
	winrect.bottom = y + info->height;

	/* Show the cursor by default */
	ShowCursor(TRUE);

	/* Setup window style */
	DWORD windstyle;
	DWORD extstyle;
	windstyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	extstyle = WS_EX_APPWINDOW;

	/* Setup window dimensions and style */
	AdjustWindowRectEx(&winrect, windstyle, FALSE, extstyle);

	/* Adjust style flags to fullscreen and resizable values. */
	if (info->style == NU_WINDOW_STYLE_FULLSCREEN) {
		windstyle |= WS_POPUP | WS_VISIBLE;
	} else {
		windstyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		if (info->style == NU_WINDOW_STYLE_RESIZABLE) {
			windstyle |= WS_MAXIMIZEBOX | WS_SIZEBOX;
			extstyle |= WS_EX_WINDOWEDGE;
		}
	}

	/* If the window has to be opened in fullscreen we need to call ChangeDisplaySettings accordingly. */
	if (info->style == NU_WINDOW_STYLE_FULLSCREEN) {
		DEVMODE dm;
		ZeroMemory(&dm, sizeof(dm));
		dm.dmSize = sizeof(dm);
		dm.dmPelsWidth = info->width;
		dm.dmPelsHeight = info->height;
		dm.dmBitsPerPel = 32;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			return NU_FAILURE;
		}

		x = 0;
		y = 0;
	}

	/* Now create the real window. */
	AdjustWindowRectEx(&winrect, windstyle, FALSE, extstyle);

	//Adjust for adornments
	int windowWidth = winrect.right - winrect.left;
	int windowHeight = winrect.bottom - winrect.top;

	handle = CreateWindowEx(
		extstyle,
		gClassName,
		info->title,
		windstyle,
		x, y, windowWidth, windowHeight,
		NULL, /* Parent window */
		NULL, /* menu */
		GetModuleHandle(NULL), NULL); /* pass this to WM_CREATE */

	if (!handle) {
		return NU_FAILURE;
	}

	ShowWindow(handle, SW_SHOW);
	SetForegroundWindow(handle);

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = handle;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	/* create the window */
	Window* window = nNew(Window, allocator);
	window->handle = handle;
	window->width = info->width;
	window->height = info->height;

	/* initialize the user memory pointer to the window */
	SetWindowLongPtr(handle, GWLP_USERDATA, (uintptr_t)window);

	*outWindow = window;
	return NU_SUCCESS;
}

void nuDestroyWindow(NuWindow window, NuAllocator* allocator)
{
	if (!window) return;
	allocator = nGetAllocator(allocator);

	if (window->fullscreen) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(window->handle);
	window->handle = 0;

	nFree(window, allocator);
}

void nuWindowSetTitle(NuWindow window, const char* title)
{
	nEnforce(window, "Invalid window provided.");
	if (window->handle) SetWindowText(window->handle, title);
}

void nuWindowShowCursor(NuWindow window, bool show)
{
	nEnforce(window, "Invalid window provided.");
	while (ShowCursor(show) > -1) {}
	window->isCursorHidden = show;
}

bool nuWindowPollEvents(NuWindow window, NuWindowEvent *e)
{
	nEnforce(window, "Invalid window provided.");

	/* save the event address in the user data struct. This will be accessed by WndProc. */
	window->event = e;
	e->type = NU_WINDOW_EVENT_TYPE_NONE;

	MSG msg;
	do {
		if (PeekMessage(&msg, window->handle, 0, 0, PM_REMOVE) == FALSE) return false;
		DispatchMessage(&msg);
	} while (e->type == NU_WINDOW_EVENT_TYPE_NONE);

	window->event = NULL;
	return e->type != NU_WINDOW_EVENT_TYPE_NONE;
}

NuSize2i nuWindowGetSize(NuWindow window)
{
	return (NuSize2i) { window->width, window->height };
}

void* nuWindowGetNativeHandle(NuWindow window)
{
	nEnforce(window, "Invalid window provided.");
	return window->handle;
}


NuResult nInitWindowModule(void)
{
	nEnforce(!gWindowModule.initialized, "Window module has already been initialized.");
	
	/* Retrieve current module of this process */
	HINSTANCE module = GetModuleHandle(NULL);

	/* Initialize the nunki application window class */
	WNDCLASSEX winclass;
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.lpszClassName = gClassName;
	winclass.lpfnWndProc = (WNDPROC)wndProc;
	winclass.hInstance = module;
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	winclass.hbrBackground = NULL;
	winclass.lpszMenuName = NULL;
	winclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hIconSm = NULL;

	/* Try registering this class. */
	if (!RegisterClassEx(&winclass)) return false;

	HWND handle = gWindowModule.dummyWindowHandle = CreateWindowEx(WS_EX_APPWINDOW, gClassName, "", WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 1, 1, NULL, NULL, module, NULL);
	if (!handle) {
		return NU_FAILURE;
	}

	/* Retrieve the device context from the window */
	HDC hDC = gWindowModule.dummyWindowHDC = GetDC(handle);
	if (!hDC) {
		DestroyWindow(handle);
		return false;
	}

	gWindowModule.initialized = true;
	return NU_SUCCESS;
}

void nDeinitWindowModule(void)
{
	if (!gWindowModule.initialized) return;
	ReleaseDC(gWindowModule.dummyWindowHandle, gWindowModule.dummyWindowHDC);
	DestroyWindow(gWindowModule.dummyWindowHandle);
	UnregisterClass(gClassName, GetModuleHandle(NULL));
	gWindowModule.initialized = false;
}

void* nGetDummyWindowHandle(void)
{
	EnforceInitialized();
	return gWindowModule.dummyWindowHandle;
}

/*-------------------------------------------------------------------------------------------------
 * WinAPI details
 *-----------------------------------------------------------------------------------------------*/

/** Translates a Windows key to the corresponding key */
static NuKey translate_key(WPARAM wParam, LPARAM lParam)
{
#undef DELETE
	/* Simulate always on numpad. */
	if ((HIWORD(lParam) & 0x100) == 0) {
		switch (MapVirtualKey(HIWORD(lParam) & 0xFF, 1)) {
			case VK_INSERT:   return NU_KEY_NUMPAD0;
			case VK_END:      return NU_KEY_NUMPAD1;
			case VK_DOWN:     return NU_KEY_NUMPAD2;
			case VK_NEXT:     return NU_KEY_NUMPAD3;
			case VK_LEFT:     return NU_KEY_NUMPAD4;
			case VK_CLEAR:    return NU_KEY_NUMPAD5;
			case VK_RIGHT:    return NU_KEY_NUMPAD6;
			case VK_HOME:     return NU_KEY_NUMPAD7;
			case VK_UP:       return NU_KEY_NUMPAD8;
			case VK_PRIOR:    return NU_KEY_NUMPAD9;
			case VK_DIVIDE:   return NU_KEY_NUMPADSLASH;
			case VK_MULTIPLY: return NU_KEY_NUMPADASTERISK;
			case VK_SUBTRACT: return NU_KEY_NUMPADMINUS;
			case VK_ADD:      return NU_KEY_NUMPADPLUS;
			case VK_DELETE:   return NU_KEY_NUMPADDOT;
			default:          break;
		}
	}

	/* Special handling for system keys */
	switch (wParam) {
		case VK_ESCAPE:        return NU_KEY_ESCAPE;
		case VK_TAB:           return NU_KEY_TAB;
		case VK_BACK:          return NU_KEY_BACKSPACE;
		case VK_HOME:          return NU_KEY_HOME;
		case VK_END:           return NU_KEY_END;
		case VK_PRIOR:         return NU_KEY_PAGEUP;
		case VK_NEXT:          return NU_KEY_PAGEDOWN;
		case VK_INSERT:        return NU_KEY_INSERT;
		case VK_DELETE:        return NU_KEY_DELETE;
		case VK_LEFT:          return NU_KEY_LEFT;
		case VK_UP:            return NU_KEY_UP;
		case VK_RIGHT:         return NU_KEY_RIGHT;
		case VK_DOWN:          return NU_KEY_DOWN;
		case VK_F1:            return NU_KEY_F1;
		case VK_F2:            return NU_KEY_F2;
		case VK_F3:            return NU_KEY_F3;
		case VK_F4:            return NU_KEY_F4;
		case VK_F5:            return NU_KEY_F5;
		case VK_F6:            return NU_KEY_F6;
		case VK_F7:            return NU_KEY_F7;
		case VK_F8:            return NU_KEY_F8;
		case VK_F9:            return NU_KEY_F9;
		case VK_F10:           return NU_KEY_F10;
		case VK_F11:           return NU_KEY_F11;
		case VK_F12:           return NU_KEY_F12;
		case VK_F13:           return NU_KEY_F13;
		case VK_F14:           return NU_KEY_F14;
		case VK_F15:           return NU_KEY_F15;
		case VK_F16:           return NU_KEY_F16;
		case VK_F17:           return NU_KEY_F17;
		case VK_F18:           return NU_KEY_F18;
		case VK_F19:           return NU_KEY_F19;
		case VK_F20:           return NU_KEY_F20;
		case VK_F21:           return NU_KEY_F21;
		case VK_F22:           return NU_KEY_F22;
		case VK_F23:           return NU_KEY_F23;
		case VK_F24:           return NU_KEY_F24;
		case VK_NUMLOCK:       return NU_KEY_NUMLOCK;
		case VK_CAPITAL:       return NU_KEY_CAPSLOCK;
		case VK_SNAPSHOT:      return NU_KEY_PRINTSCREEN;
		case VK_SCROLL:        return NU_KEY_SCROLLLOCK;
		case VK_PAUSE:         return NU_KEY_PAUSE;
		case VK_LWIN:          return NU_KEY_LEFTSUPER;
		case VK_RWIN:          return NU_KEY_RIGHTSUPER;
		case VK_APPS:          return NU_KEY_MENU;

		/* Numpad */
		case VK_NUMPAD0:       return NU_KEY_NUMPAD0;
		case VK_NUMPAD1:       return NU_KEY_NUMPAD1;
		case VK_NUMPAD2:       return NU_KEY_NUMPAD2;
		case VK_NUMPAD3:       return NU_KEY_NUMPAD3;
		case VK_NUMPAD4:       return NU_KEY_NUMPAD4;
		case VK_NUMPAD5:       return NU_KEY_NUMPAD5;
		case VK_NUMPAD6:       return NU_KEY_NUMPAD6;
		case VK_NUMPAD7:       return NU_KEY_NUMPAD7;
		case VK_NUMPAD8:       return NU_KEY_NUMPAD8;
		case VK_NUMPAD9:       return NU_KEY_NUMPAD9;
		case VK_DIVIDE:        return NU_KEY_NUMPADSLASH;
		case VK_MULTIPLY:      return NU_KEY_NUMPADASTERISK;
		case VK_SUBTRACT:      return NU_KEY_NUMPADMINUS;
		case VK_ADD:           return NU_KEY_NUMPADPLUS;
		case VK_DECIMAL:       return NU_KEY_NUMPADDOT;

		/* Special keys */
		case VK_SHIFT:
		{
			const DWORD scancode = MapVirtualKey(VK_RSHIFT, 0);
			if ((DWORD)((lParam & 0x01ff0000) >> 16) == scancode)
				return NU_KEY_RIGHTSHIFT;
			return NU_KEY_LEFTSHIFT;
		}

		case VK_CONTROL:
		{
			MSG next;
			DWORD time;

			if (lParam & 0x01000000)
				return NU_KEY_RIGHTCTRL;

			/* Use GLFW trick: "Alt Gr" sends LCTRL, then RALT. We only */
			/* want the RALT message, so we try to see if the next message */
			/* is a RALT message. In that case, this is a false LCTRL! */
			time = GetMessageTime();

			if (PeekMessage(&next, NULL, 0, 0, PM_NOREMOVE)) {
				if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN
					|| next.message == WM_KEYUP || next.message == WM_SYSKEYUP) {
					if (next.wParam == VK_MENU && (next.lParam & 0x01000000) && next.time == time) {
						/* Next message is a RALT down message, which */
						/* means that this is not a proper LCTRL message */
						return NU_KEY_UNKNOWN;
					}
				}
			}

			return NU_KEY_LEFTCTRL;
		}

		case VK_MENU:
			if (lParam & 0x01000000)
				return NU_KEY_RIGHTALT;
			return NU_KEY_LEFTALT;

			/* The ENTER keys require special handling */
		case VK_RETURN:
			if (lParam & 0x01000000)
				return NU_KEY_NUMPADENTER;
			return NU_KEY_ENTER;

			/* Printable keys */
		case VK_SPACE: return NU_KEY_SPACE;
		case 0x30: return NU_KEY_0;
		case 0x31: return NU_KEY_1;
		case 0x32: return NU_KEY_2;
		case 0x33: return NU_KEY_3;
		case 0x34: return NU_KEY_4;
		case 0x35: return NU_KEY_5;
		case 0x36: return NU_KEY_6;
		case 0x37: return NU_KEY_7;
		case 0x38: return NU_KEY_8;
		case 0x39: return NU_KEY_9;
		case 0x41: return NU_KEY_A;
		case 0x42: return NU_KEY_B;
		case 0x43: return NU_KEY_C;
		case 0x44: return NU_KEY_D;
		case 0x45: return NU_KEY_E;
		case 0x46: return NU_KEY_F;
		case 0x47: return NU_KEY_G;
		case 0x48: return NU_KEY_H;
		case 0x49: return NU_KEY_I;
		case 0x4A: return NU_KEY_J;
		case 0x4B: return NU_KEY_K;
		case 0x4C: return NU_KEY_L;
		case 0x4D: return NU_KEY_M;
		case 0x4E: return NU_KEY_N;
		case 0x4F: return NU_KEY_O;
		case 0x50: return NU_KEY_P;
		case 0x51: return NU_KEY_Q;
		case 0x52: return NU_KEY_R;
		case 0x53: return NU_KEY_S;
		case 0x54: return NU_KEY_T;
		case 0x55: return NU_KEY_U;
		case 0x56: return NU_KEY_V;
		case 0x57: return NU_KEY_W;
		case 0x58: return NU_KEY_X;
		case 0x59: return NU_KEY_Y;
		case 0x5A: return NU_KEY_Z;
		case 0xBD: return NU_KEY_MINUS;
		case 0xBB: return NU_KEY_EQUAL;
		case 0xDB: return NU_KEY_LEFTBRACKET;
		case 0xDD: return NU_KEY_RIGHTBRACKET;
		case 0xDC: return NU_KEY_BACKSLASH;
		case 0xBA: return NU_KEY_SEMICOLON;
		case 0xDE: return NU_KEY_APOSTROPHE;
		case 0xC0: return NU_KEY_GRAVEACCENT;
		case 0xBC: return NU_KEY_COMMA;
		case 0xBE: return NU_KEY_PERIOD;
		case 0xBF: return NU_KEY_SLASH;
		case 0xDF: return NU_KEY_WORLD1;
		case 0xE2: return NU_KEY_WORLD2;
		default: break;
	}

	/* No matching translation was found */
	return NU_KEY_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM lParam)
{
	Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (!window || !window->event)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	NuWindowEvent *e = window->event;
	if (!e) return DefWindowProc(hWnd, uMsg, wParam, lParam);

	static bool isShiftDown = false;
	static bool isCtrlDown = false;
	static bool isAltDown = false;

	switch (uMsg) {
		case WM_CREATE:
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)(lParam);
			SetWindowLongPtr(hWnd, 0, (LONG_PTR)cs->lpCreateParams);
			break;
		}

		case WM_CLOSE:
			e->type = NU_WINDOW_EVENT_TYPE_CLOSE;
			break;

		case WM_SYSCOMMAND:
		{
			switch (wParam & 0xfff0) {
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				{
					if (window->fullscreen) {
						/* We are running in fullscreen mode, so disallow */
						/* screen saver and screen blanking */
						return 0;
					} else
						break;
				}

				/* User trying to access application menu using ALT? */
				case SC_KEYMENU:
					return 0;
			}
			break;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			e->key.isDown = true;
			e->key.repetitions = lParam & 0x7FFF;
			goto kenup;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			e->key.isDown = false;
		kenup:
			e->type = NU_WINDOW_EVENT_TYPE_KEY;
			e->key.code = translate_key(wParam, lParam);
			e->key.scancode = (lParam >> 16) & 0xff;
			if (e->key.code == NU_KEY_LEFTSHIFT || e->key.code == NU_KEY_RIGHTSHIFT) isShiftDown = e->key.isDown;
			if (e->key.code == NU_KEY_LEFTCTRL || e->key.code == NU_KEY_RIGHTCTRL) isCtrlDown = e->key.isDown;
			if (e->key.code == NU_KEY_LEFTALT || e->key.code == NU_KEY_RIGHTALT) isAltDown = e->key.isDown;
			if (e->key.code == NU_KEY_UNKNOWN) e->type = NU_WINDOW_EVENT_TYPE_NONE;
			e->key.isAnyShiftDown = isShiftDown;
			e->key.isAnyCtrlDown = isCtrlDown;
			e->key.isAnyAltDown = isAltDown;
			break;

		case WM_MOUSEMOVE:
		{
			int newCursor[] = { LOWORD(lParam), HIWORD(lParam) };

			if (newCursor[0] != window->oldCursor[0] || newCursor[1] != window->oldCursor[1]) {
				int cursor[2];

				if (window->isCursorHidden) {
					for (int i = 0; i < 2; ++i) cursor[i] = newCursor[i] - window->oldCursor[i];
				} else {
					for (int i = 0; i < 2; ++i) cursor[i] = newCursor[i];
				}

				e->type = NU_WINDOW_EVENT_TYPE_MOUSE_MOVE;
				for (int i = 0; i < 2; ++i) {
					e->mouseMotion.relativePosition[i] = newCursor[i] - window->oldCursor[i];
					e->mouseMotion.position[i] = newCursor[i];
					window->oldCursor[i] = newCursor[i];
				}
			}
			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			e->type = NU_WINDOW_EVENT_TYPE_BUTTON;
			SetCapture(hWnd);
			e->mouseButton.isDown = true;
			e->mouseButton.position[0] = window->oldCursor[0];
			e->mouseButton.position[1] = window->oldCursor[1];

			if (uMsg == WM_LBUTTONDOWN)
				e->mouseButton.button = NU_MOUSE_BUTTON_LEFT;
			else if (uMsg == WM_RBUTTONDOWN)
				e->mouseButton.button = NU_MOUSE_BUTTON_RIGHT;
			else if (uMsg == WM_MBUTTONDOWN)
				e->mouseButton.button = NU_MOUSE_BUTTON_MIDDLE;
			else {
				if (HIWORD(wParam) == XBUTTON1)
					e->mouseButton.button = NU_MOUSE_BUTTON_BUTTON4;
				else if (HIWORD(wParam) == XBUTTON2)
					e->mouseButton.button = NU_MOUSE_BUTTON_BUTTON5;
				return TRUE;
			}
			return 0;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			ReleaseCapture();
			e->mouseButton.isDown = false;
			e->type = NU_WINDOW_EVENT_TYPE_BUTTON;
			e->mouseButton.position[0] = window->oldCursor[0];
			e->mouseButton.position[1] = window->oldCursor[1];

			if (uMsg == WM_LBUTTONUP)
				e->mouseButton.button = NU_MOUSE_BUTTON_LEFT;
			else if (uMsg == WM_RBUTTONUP)
				e->mouseButton.button = NU_MOUSE_BUTTON_RIGHT;
			else if (uMsg == WM_MBUTTONUP)
				e->mouseButton.button = NU_MOUSE_BUTTON_MIDDLE;
			else {
				if (HIWORD(wParam) == XBUTTON1)
					e->mouseButton.button = NU_MOUSE_BUTTON_BUTTON4;
				else if (HIWORD(wParam) == XBUTTON2)
					e->mouseButton.button = NU_MOUSE_BUTTON_BUTTON5;
				return TRUE;
			}
			return 0;

		case WM_SIZE:
			e->size.width = LOWORD(lParam);
			e->size.height = HIWORD(lParam);
			window->width = e->size.width;
			window->height = e->size.height;
			e->type = NU_WINDOW_EVENT_TYPE_RESIZE;
			break;

		default: break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#pragma endregion
