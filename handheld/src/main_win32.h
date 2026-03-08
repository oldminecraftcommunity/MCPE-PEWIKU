#include <windows.h>
#include <windowsx.h>
#include <WinSock2.h>
#include <process.h>
#include <cstdio>
#include <cstring>
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "util/Mth.h"
#include "AppPlatform_win32.h"
#include "App.h"

static App* g_app = 0;
static volatile bool g_running = true;
static HWND g_hwnd = NULL;
static bool g_mouseGrabbed = false;
static int g_centerX = 0;
static int g_centerY = 0;
static volatile LONG g_rawDeltaX = 0;
static volatile LONG g_rawDeltaY = 0;

void platform_setMouseGrabbed(bool grab);

void resizeWindow(HWND hWnd, int nWidth, int nHeight) {
	RECT rcClient, rcWindow;
	POINT ptDiff;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, nWidth + ptDiff.x, nHeight + ptDiff.y, TRUE);
}

void toggleResolutions(HWND hwnd, int direction) {
	static int n = 0;
	static int sizes[][3] = {
		{854, 480, 1},
		{800, 480, 1},
		{480, 320, 1},
		{1024, 768, 1},
		{1280, 800, 1},
		{1024, 580, 1}
	};
	static int count = sizeof(sizes) / sizeof(sizes[0]);
	n = (count + n + direction) % count;

	int* size = sizes[n];
	int k = size[2];

	resizeWindow(hwnd, k * size[0], k * size[1]);
}

LRESULT WINAPI windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_KEYDOWN: {
		if (wParam == 33) toggleResolutions(hWnd, -1);
		if (wParam == 34) toggleResolutions(hWnd, +1);
		Keyboard::feed((unsigned char)wParam, 1);
		return 0;
	}
	case WM_KEYUP: {
		Keyboard::feed((unsigned char)wParam, 0);
		return 0;
	}
	case WM_CHAR: {
		if (wParam >= 32)
			Keyboard::feedText(wParam);
		return 0;
	}
	case WM_LBUTTONDOWN: {
		Mouse::feed(MouseAction::ACTION_LEFT, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Multitouch::feed(1, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		return 0;
	}
	case WM_LBUTTONUP: {
		Mouse::feed(MouseAction::ACTION_LEFT, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		Multitouch::feed(1, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		return 0;
	}
	case WM_RBUTTONDOWN: {
		Mouse::feed(MouseAction::ACTION_RIGHT, 1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_RBUTTONUP: {
		Mouse::feed(MouseAction::ACTION_RIGHT, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE && g_mouseGrabbed) {
			platform_setMouseGrabbed(true);
		}
		else if (LOWORD(wParam) == WA_INACTIVE) {
			ClipCursor(NULL);
		}
		return 0;

	case WM_MOUSEMOVE: {
		if (!g_mouseGrabbed) {
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			Mouse::feed(MouseAction::ACTION_MOVE, 0, x, y, 0, 0);
			Multitouch::feed(MouseAction::ACTION_MOVE, 0, x, y, 0);
		}
		return 0;
	}

	case WM_MOUSEWHEEL: {
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (delta > 0) {
			Mouse::feed(3, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, 1);
		}
		else if (delta < 0) {
			Mouse::feed(3, 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0, -1);
		}
		return 0;
	}

	case WM_INPUT: {
		if (g_mouseGrabbed) {
			RAWINPUT raw;
			UINT size = sizeof(raw);
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != (UINT)-1) {
				if (raw.header.dwType == RIM_TYPEMOUSE) {
					InterlockedExchangeAdd(&g_rawDeltaX, raw.data.mouse.lLastX);
					InterlockedExchangeAdd(&g_rawDeltaY, raw.data.mouse.lLastY);
				}
			}
		}
		return 0;
	}

	case WM_SIZE:
		if (g_app) {
			g_app->setSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (g_mouseGrabbed) {
				RECT rect;
				GetClientRect(hWnd, &rect);
				g_centerX = (rect.right - rect.left) / 2;
				g_centerY = (rect.bottom - rect.top) / 2;

				POINT pt = { g_centerX, g_centerY };
				ClientToScreen(hWnd, &pt);
				SetCursorPos(pt.x, pt.y);
			}
		}
		return 0;

	case WM_NCDESTROY:
		g_running = false;
		return 0;

	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void platform(HWND* result, int width, int height) {
	WNDCLASS wc;
	RECT wRect;
	HWND hwnd;
	HINSTANCE hInstance;

	wRect.left = 0L;
	wRect.right = (long)width;
	wRect.top = 0L;
	wRect.bottom = (long)height;

	hInstance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OGLES";

	RegisterClass(&wc);

	AdjustWindowRectEx(&wRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, "OGLES", "main", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, wRect.right - wRect.left, wRect.bottom - wRect.top, NULL, NULL, hInstance, NULL);
	*result = hwnd;
}

void platform_setMouseGrabbed(bool grab) {
	g_mouseGrabbed = grab;
	if (!g_hwnd) return;

	if (grab) {
		RECT rect;
		GetClientRect(g_hwnd, &rect);

		g_centerX = (rect.right - rect.left) / 2;
		g_centerY = (rect.bottom - rect.top) / 2;

		MapWindowPoints(g_hwnd, NULL, (LPPOINT)&rect, 2);
		ClipCursor(&rect);

		POINT pt = { g_centerX, g_centerY };
		ClientToScreen(g_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);

		InterlockedExchange(&g_rawDeltaX, 0);
		InterlockedExchange(&g_rawDeltaY, 0);
	}
	else {
		ClipCursor(NULL);

		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(g_hwnd, &pt);
		Mouse::feed(MouseAction::ACTION_MOVE, 0, pt.x, pt.y, 0, 0);
		Multitouch::feed(MouseAction::ACTION_MOVE, 0, pt.x, pt.y, 0);
	}
}

void processMouseInput() {
	if (g_mouseGrabbed && g_hwnd) {
		LONG dx = InterlockedExchange(&g_rawDeltaX, 0);
		LONG dy = InterlockedExchange(&g_rawDeltaY, 0);

		if (dx != 0 || dy != 0) {
			Mouse::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, (int)dx, (int)dy);
			Multitouch::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, 0);
		}

		POINT pt = { g_centerX, g_centerY };
		ClientToScreen(g_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}
}

int main(void) {
	SetProcessDPIAware();
	AppContext appContext;
	MSG sMessage;

#ifndef STANDALONE_SERVER

	EGLint aEGLAttributes[] = {
		EGL_RED_SIZE,		8,
		EGL_GREEN_SIZE,		8,
		EGL_BLUE_SIZE,		8,
		EGL_ALPHA_SIZE,		8,
		EGL_DEPTH_SIZE,		16,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	EGLConfig m_eglConfig[1];
	EGLint nConfigs;

	HWND hwnd;
	g_running = true;

	appContext.platform = new AppPlatform_win32();
	platform(&hwnd, appContext.platform->getScreenWidth(), appContext.platform->getScreenHeight());
	g_hwnd = hwnd;
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (ShowCursor(FALSE) >= 0);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
		printf("Failed to register raw input: %d\n", GetLastError());
	}

	appContext.display = eglGetDisplay(GetDC(hwnd));
	eglInitialize(appContext.display, NULL, NULL);
	eglChooseConfig(appContext.display, aEGLAttributes, m_eglConfig, 1, &nConfigs);
	appContext.surface = eglCreateWindowSurface(appContext.display, m_eglConfig[0], (NativeWindowType)hwnd, 0);
	appContext.context = eglCreateContext(appContext.display, m_eglConfig[0], EGL_NO_CONTEXT, NULL);
	if (!appContext.context) {
		printf("EGL error: %d\n", eglGetError());
	}
	eglMakeCurrent(appContext.display, appContext.surface, appContext.surface, appContext.context);

	glInit();

#endif
	App* app = new MAIN_CLASS();

	g_app = app;
	((MAIN_CLASS*)g_app)->externalStoragePath = ".";
	((MAIN_CLASS*)g_app)->externalCacheStoragePath = ".";
	g_app->init(appContext);
	g_app->setSize(appContext.platform->getScreenWidth(), appContext.platform->getScreenHeight());

	while (g_running && !app->wantToQuit())
	{
		while (PeekMessage(&sMessage, NULL, 0, 0, PM_REMOVE) > 0) {
			if (sMessage.message == WM_QUIT) {
				g_running = false;
				break;
			}
			TranslateMessage(&sMessage);
			DispatchMessage(&sMessage);
		}

		processMouseInput();
		app->update();
	}

	delete app;
	appContext.platform->finish();
	delete appContext.platform;

#ifndef STANDALONE_SERVER
	eglMakeCurrent(appContext.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(appContext.display, appContext.context);
	eglDestroySurface(appContext.display, appContext.surface);
	eglTerminate(appContext.display);
#endif

	return 0;
}