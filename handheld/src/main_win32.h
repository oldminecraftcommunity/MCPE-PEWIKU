#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <png.h>
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
static int g_savedMouseX = 0;
static int g_savedMouseY = 0;
static HICON g_windowIcon = NULL;
static bool g_isFullscreen = false;
static WINDOWPLACEMENT g_windowPlacement = { sizeof(g_windowPlacement) };
static DWORD g_windowedStyle = 0;
static DWORD g_windowedExStyle = 0;

void platform_setMouseGrabbed(bool grab);

static const char* getWindowTitle() {
#ifdef DEBUG
	return "Minecraft PE: Pewiku, v0.6.1-dev";
#else
	return "Minecraft PE: Pewiku, v0.6.1";
#endif
}

static std::string getExecutableDirectory() {
	char modulePath[MAX_PATH] = { 0 };
	const DWORD modulePathLength = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
	if (modulePathLength > 0 && modulePathLength < MAX_PATH) {
		std::string executablePath(modulePath, modulePathLength);
		const size_t slashPos = executablePath.find_last_of("\\/");
		if (slashPos != std::string::npos) {
			return executablePath.substr(0, slashPos);
		}
	}
	return ".";
}

static bool loadPngRgba(const char* path, std::vector<unsigned char>& pixels, int& width, int& height) {
	FILE* fp = fopen(path, "rb");
	if (!fp) {
		return false;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose(fp);
		return false;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		return false;
	}

	if (setjmp(png_jmpbuf(png))) {
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		return false;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	width = (int)png_get_image_width(png, info);
	height = (int)png_get_image_height(png, info);
	png_byte colorType = png_get_color_type(png, info);
	png_byte bitDepth = png_get_bit_depth(png, info);

	if (bitDepth == 16)
		png_set_strip_16(png);
	if (colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		png_set_expand_gray_1_2_4_to_8(png);
	if (png_get_valid(png, info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png);
	if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	pixels.resize((size_t)width * (size_t)height * 4);
	std::vector<png_bytep> rows((size_t)height);
	for (int y = 0; y < height; ++y) {
		rows[(size_t)y] = pixels.data() + (size_t)y * (size_t)width * 4;
	}

	png_read_image(png, rows.data());
	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);
	return true;
}

static HICON createIconFromRgba(const std::vector<unsigned char>& pixels, int width, int height) {
	BITMAPV5HEADER bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bV5Size = sizeof(bi);
	bi.bV5Width = width;
	bi.bV5Height = -height;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	bi.bV5RedMask = 0x00FF0000;
	bi.bV5GreenMask = 0x0000FF00;
	bi.bV5BlueMask = 0x000000FF;
	bi.bV5AlphaMask = 0xFF000000;

	void* dibPixels = NULL;
	HDC hdc = GetDC(NULL);
	HBITMAP colorBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &dibPixels, NULL, 0);
	ReleaseDC(NULL, hdc);
	if (!colorBitmap || !dibPixels) {
		if (colorBitmap)
			DeleteObject(colorBitmap);
		return NULL;
	}

	unsigned char* dst = reinterpret_cast<unsigned char*>(dibPixels);
	for (int i = 0; i < width * height; ++i) {
		dst[i * 4 + 0] = pixels[(size_t)i * 4 + 2];
		dst[i * 4 + 1] = pixels[(size_t)i * 4 + 1];
		dst[i * 4 + 2] = pixels[(size_t)i * 4 + 0];
		dst[i * 4 + 3] = pixels[(size_t)i * 4 + 3];
	}

	HBITMAP maskBitmap = CreateBitmap(width, height, 1, 1, NULL);
	if (!maskBitmap) {
		DeleteObject(colorBitmap);
		return NULL;
	}

	ICONINFO iconInfo;
	ZeroMemory(&iconInfo, sizeof(iconInfo));
	iconInfo.fIcon = TRUE;
	iconInfo.hbmColor = colorBitmap;
	iconInfo.hbmMask = maskBitmap;

	HICON icon = CreateIconIndirect(&iconInfo);
	DeleteObject(colorBitmap);
	DeleteObject(maskBitmap);
	return icon;
}

static HICON loadWindowIcon() {
	const std::string executableDirectory = getExecutableDirectory();
	const char* relativeCandidates[] = {
		"icon.png"
	};

	for (size_t i = 0; i < sizeof(relativeCandidates) / sizeof(relativeCandidates[0]); ++i) {
		std::string path = executableDirectory + "\\" + relativeCandidates[i];
		std::vector<unsigned char> pixels;
		int width = 0;
		int height = 0;
		if (loadPngRgba(path.c_str(), pixels, width, height)) {
			HICON icon = createIconFromRgba(pixels, width, height);
			if (icon) {
				return icon;
			}
		}
	}

	HICON fallback = LoadIcon(NULL, IDI_APPLICATION);
	return fallback ? CopyIcon(fallback) : NULL;
}

static void toggleFullscreen(HWND hwnd) {
	if (!hwnd) {
		return;
	}

	if (!g_isFullscreen) {
		g_windowedStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);
		g_windowedExStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		g_windowPlacement.length = sizeof(g_windowPlacement);
		GetWindowPlacement(hwnd, &g_windowPlacement);

		HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo;
		monitorInfo.cbSize = sizeof(monitorInfo);
		GetMonitorInfo(monitor, &monitorInfo);

		SetWindowLongPtr(hwnd, GWL_STYLE, g_windowedStyle & ~WS_OVERLAPPEDWINDOW);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, g_windowedExStyle & ~(WS_EX_WINDOWEDGE | WS_EX_APPWINDOW));
		SetWindowPos(hwnd, HWND_TOP,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
		ShowWindow(hwnd, SW_MAXIMIZE);
		g_isFullscreen = true;
	}
	else {
		SetWindowLongPtr(hwnd, GWL_STYLE, g_windowedStyle);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, g_windowedExStyle);
		SetWindowPlacement(hwnd, &g_windowPlacement);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		ShowWindow(hwnd, SW_SHOWNORMAL);
		g_isFullscreen = false;
	}
}

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
	case WM_SYSCOMMAND:
		// no menu mode
		if ((wParam & 0xFFF0) == SC_KEYMENU) {
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (wParam == VK_F10) {
			unsigned char key = Keyboard::KEY_F10;
			Keyboard::feed(key, uMsg == WM_SYSKEYDOWN ? 1 : 0);
			return 0;
		}
		break;

	case WM_KEYDOWN: {
		if (wParam == VK_F11) {
			toggleFullscreen(hWnd);
			return 0;
		}
		if (wParam == 33) toggleResolutions(hWnd, -1);
		if (wParam == 34) toggleResolutions(hWnd, +1);
		unsigned char key = (unsigned char)wParam;
		if (wParam >= VK_F1 && wParam <= VK_F12)
			key = Keyboard::KEY_F1 + (wParam - VK_F1);
		else if (wParam >= 'A' && wParam <= 'Z')
			key = 'a' + (wParam - 'A');

		Keyboard::feed(key, 1);
		return 0;
	}
	case WM_KEYUP: {
		unsigned char key = (unsigned char)wParam;
		if (wParam >= VK_F1 && wParam <= VK_F12)
			key = Keyboard::KEY_F1 + (wParam - VK_F1);
		else if (wParam >= 'A' && wParam <= 'Z')
			key = 'a' + (wParam - 'A');

		Keyboard::feed(key, 0);
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

		POINT pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(hWnd, &pt);

		int x = g_mouseGrabbed ? Mouse::getX() : pt.x;
		int y = g_mouseGrabbed ? Mouse::getY() : pt.y;

		if (delta > 0) {
			Mouse::feed(3, 0, x, y, 0, 1);
		}
		else if (delta < 0) {
			Mouse::feed(3, 0, x, y, 0, -1);
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
			int width = GET_X_LPARAM(lParam);
			int height = GET_Y_LPARAM(lParam);
			if (wParam == SIZE_MINIMIZED || width <= 0 || height <= 0)
				return 0;

			g_app->setSize(width, height);

			if (g_mouseGrabbed) {
				RECT rect;
				GetClientRect(hWnd, &rect);
				g_centerX = (rect.right - rect.left) / 2;
				g_centerY = (rect.bottom - rect.top) / 2;

				POINT pt = { g_centerX, g_centerY };
				ClientToScreen(hWnd, &pt);
				Mouse::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, 0, 0);
				Multitouch::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, 0);
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
	const char* title = getWindowTitle();

	wRect.left = 0L;
	wRect.right = (long)width;
	wRect.top = 0L;
	wRect.bottom = (long)height;

	hInstance = GetModuleHandle(NULL);
	if (!g_windowIcon) {
		g_windowIcon = loadWindowIcon();
	}

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)windowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = g_windowIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OGLES";

	RegisterClass(&wc);

	AdjustWindowRectEx(&wRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, "OGLES", title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, wRect.right - wRect.left, wRect.bottom - wRect.top, NULL, NULL, hInstance, NULL);
	if (hwnd && g_windowIcon) {
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_windowIcon);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_windowIcon);
	}
	*result = hwnd;
}

void platform_setMouseGrabbed(bool grab) {
	g_mouseGrabbed = grab;
	if (!g_hwnd) return;

	if (grab) {
		RECT rect;
		GetClientRect(g_hwnd, &rect);

		POINT saved;
		GetCursorPos(&saved);
		g_savedMouseX = saved.x;
		g_savedMouseY = saved.y;

		g_centerX = (rect.right - rect.left) / 2;
		g_centerY = (rect.bottom - rect.top) / 2;

		MapWindowPoints(g_hwnd, NULL, (LPPOINT)&rect, 2);
		ClipCursor(&rect);

		POINT pt = { g_centerX, g_centerY };
		ClientToScreen(g_hwnd, &pt);
		Mouse::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, 0, 0);
		Multitouch::feed(MouseAction::ACTION_MOVE, 0, g_centerX, g_centerY, 0);
		SetCursorPos(pt.x, pt.y);

		InterlockedExchange(&g_rawDeltaX, 0);
		InterlockedExchange(&g_rawDeltaY, 0);
		while (ShowCursor(FALSE) >= 0);
	}
	else {
		ClipCursor(NULL);

		POINT pt;
		pt.x = g_savedMouseX;
		pt.y = g_savedMouseY;
		ScreenToClient(g_hwnd, &pt);
		Mouse::feed(MouseAction::ACTION_MOVE, 0, pt.x, pt.y, 0, 0);
		Multitouch::feed(MouseAction::ACTION_MOVE, 0, pt.x, pt.y, 0);
		SetCursorPos(g_savedMouseX, g_savedMouseY);
		while (ShowCursor(TRUE) < 0);
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

	HWND hwnd;
	g_running = true;

	appContext.platform = new AppPlatform_win32();
	platform(&hwnd, appContext.platform->getScreenWidth(), appContext.platform->getScreenHeight());
	g_hwnd = hwnd;
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	ShowCursor(TRUE);

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;
	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
		printf("Failed to register raw input: %d\n", GetLastError());
	}

	HDC hdc = GetDC(hwnd);
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	int pf = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, pf, &pfd);
	HGLRC hglrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hglrc);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW\n");
	}

	glInit();

#endif
	App* app = new MAIN_CLASS();

	g_app = app;
	std::string executableDirectory = getExecutableDirectory();
	((MAIN_CLASS*)g_app)->externalStoragePath = executableDirectory;
	((MAIN_CLASS*)g_app)->externalCacheStoragePath = executableDirectory;
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
		Multitouch::commit();
		app->update();
		SwapBuffers(hdc);
	}

	delete app;
	appContext.platform->finish();
	delete appContext.platform;

#ifndef STANDALONE_SERVER
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
	ReleaseDC(hwnd, hdc);
	if (g_windowIcon) {
		DestroyIcon(g_windowIcon);
		g_windowIcon = NULL;
	}
#endif

	return 0;
}
