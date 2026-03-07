#ifndef MAIN_UNIX_H__
#define MAIN_UNIX_H__

#include "client/renderer/gles.h"
#include "App.h"
#include "NinecraftApp.h"
#include "platform/input/Mouse.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Keyboard.h"

#define Screen X11Screen
#define Font   X11Font
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#undef Font
#undef Screen

#include <EGL/egl.h>
#include <png.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

static bool g_running_unix = true;
static bool g_mouseGrabbed = false;
static Cursor g_invisibleCursor = None;
static int g_lastX = 0;
static int g_lastY = 0;

static Display* g_dpy = nullptr;
static Window g_win = None;

static bool loadNetWmIcon(const std::string& path, std::vector<unsigned long>& iconData) {
	std::ifstream source(path.c_str(), std::ios::binary);
	if (!source) return false;
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngPtr) {
		return false;
	}

	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (!infoPtr) { png_destroy_read_struct(&pngPtr, NULL, NULL); return false; }
	png_set_read_fn(pngPtr, (png_voidp)&source, [](png_structp ptr, png_bytep data, png_size_t len) { ((std::istream*)png_get_io_ptr(ptr))->read((char*)data, len); });
	png_read_info(pngPtr, infoPtr);
	const png_uint_32 width = png_get_image_width(pngPtr, infoPtr);
	const png_uint_32 height = png_get_image_height(pngPtr, infoPtr);
	if (width == 0 || height == 0) {
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		return false;
	}

	const png_byte colorType = png_get_color_type(pngPtr, infoPtr);
	const png_byte bitDepth = png_get_bit_depth(pngPtr, infoPtr);

	if (bitDepth == 16) png_set_strip_16(pngPtr);
	if (colorType == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(pngPtr);
	if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) png_set_expand_gray_1_2_4_to_8(pngPtr);
	if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(pngPtr);
	if (colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_PALETTE) png_set_add_alpha(pngPtr, 0xFF, PNG_FILLER_AFTER);
	if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(pngPtr);

	png_read_update_info(pngPtr, infoPtr);
	std::vector<png_byte> rgba(width * height * 4);
	std::vector<png_bytep> rows(height);
	for (png_uint_32 y = 0; y < height; ++y) rows[y] = &rgba[y * width * 4];
	png_read_image(pngPtr, rows.data());
	png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
	iconData.clear(); iconData.reserve(2 + width * height);
	iconData.push_back((unsigned long)width); iconData.push_back((unsigned long)height);
	for (png_uint_32 i = 0; i < width * height; ++i) {
		const uint32_t r = rgba[i * 4 + 0], g = rgba[i * 4 + 1], b = rgba[i * 4 + 2], a = rgba[i * 4 + 3];
		iconData.push_back((unsigned long)((a << 24) | (r << 16) | (g << 8) | b));
	}
	return true;
}

// calls from MouseHandler
void platform_setMouseGrabbed(bool grab) {
    if (!g_dpy || g_win == None) return;
    if (grab == g_mouseGrabbed) return;
    g_mouseGrabbed = grab;

    if (grab) {
        if (g_invisibleCursor == None) {
            char data[1] = {0};
            Pixmap blank = XCreateBitmapFromData(g_dpy, g_win, data, 1, 1);
            XColor dummy;
            g_invisibleCursor = XCreatePixmapCursor(g_dpy, blank, blank, &dummy, &dummy, 0, 0);
            XFreePixmap(g_dpy, blank);
        }
        XDefineCursor(g_dpy, g_win, g_invisibleCursor);
        XGrabPointer(g_dpy, g_win, True, 0, GrabModeAsync, GrabModeAsync, g_win, None, CurrentTime);
        
        XWindowAttributes wa;
        XGetWindowAttributes(g_dpy, g_win, &wa);
        g_lastX = wa.width / 2;
        g_lastY = wa.height / 2;
        XWarpPointer(g_dpy, None, g_win, 0, 0, 0, 0, g_lastX, g_lastY);
    } else {
        XUndefineCursor(g_dpy, g_win);
        XUngrabPointer(g_dpy, CurrentTime);

		Window root_return, child_return;
        int root_x, root_y, win_x, win_y;
        unsigned int mask_return;
        if (XQueryPointer(g_dpy, g_win, &root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask_return)) {
            Mouse::feed(0, 0, win_x, win_y, 0, 0);
            Multitouch::feed(0, 0, win_x, win_y, 0);
        }
    }
}

// key translator X11, wasd -> WASD (˶ᵔ ᵕ ᵔ˶)
static unsigned char transformKey(KeySym keysym) {
    if (keysym >= XK_a && keysym <= XK_z) return keysym - 0x20; // 'a'-'z' to 'A'-'Z'
    if (keysym >= XK_A && keysym <= XK_Z) return keysym;
    if (keysym >= XK_0 && keysym <= XK_9) return keysym;
    
    switch(keysym) {
        case XK_Escape: return 27;
        case XK_space: return 32;
        case XK_Return: return 13;
        case XK_BackSpace: return 8;
        case XK_Tab: return 9;
        case XK_Shift_L:
        case XK_Shift_R: return 16;
        case XK_Control_L:
        case XK_Control_R: return 17;
        case XK_Up: return 38;
        case XK_Down: return 40;
        case XK_Left: return 37;
        case XK_Right: return 39;
    }
    return 0;
}

static void handleXEvent(Display* dpy, Window win, XEvent& ev, App* app) {
    switch (ev.type) {
    case ClientMessage:
        g_running_unix = false;
        break;
    case ConfigureNotify:
        if (app) app->setSize(ev.xconfigure.width, ev.xconfigure.height);
        break;
    case KeyPress: {
        KeySym keysym = XLookupKeysym(&ev.xkey, 0);
        unsigned char transformed = transformKey(keysym);
        if (transformed) Keyboard::feed(transformed, 1);

        char buf[8];
        int len = XLookupString(&ev.xkey, buf, sizeof(buf), nullptr, nullptr);
        for (int i = 0; i < len; ++i) {
            unsigned char ch = static_cast<unsigned char>(buf[i]);
            if (ch >= 32) Keyboard::feedText(ch);
        }
        break;
    }
    case KeyRelease: {
        KeySym keysym = XLookupKeysym(&ev.xkey, 0);
        unsigned char transformed = transformKey(keysym);
        if (transformed) Keyboard::feed(transformed, 0);
        break;
    }
    case ButtonPress: {
        int button = ev.xbutton.button;
        int x = ev.xbutton.x;
        int y = ev.xbutton.y;

        if (button == Button4) {
            Mouse::feed(3, 0, x, y, 0, -1); // 3 = ACTION_WHEEL
        } else if (button == Button5) {
            Mouse::feed(3, 0, x, y, 0, 1);
        } else {
            int action = 0;
            if (button == Button1) action = 1;
            else if (button == Button3) action = 2;
            
            if (action) {
				// force update cursor position
                Mouse::feed(0, 0, x, y, 0, 0);
                
                Mouse::feed(action, 1, x, y);
                Multitouch::feed(action, 1, x, y, 0);
            }
        }
        break;
    }
    case ButtonRelease: {
        int button = ev.xbutton.button;
        int x = ev.xbutton.x;
        int y = ev.xbutton.y;
        int action = 0;
        if (button == Button1) action = 1;
        else if (button == Button3) action = 2;
        if (action) {
            Mouse::feed(action, 0, x, y);
            Multitouch::feed(action, 0, x, y, 0);
        }
        break;
    }
    case MotionNotify: {
        int x = ev.xmotion.x;
        int y = ev.xmotion.y;

        if (g_mouseGrabbed) {
            int dx = x - g_lastX;
            int dy = y - g_lastY;

            if (dx != 0 || dy != 0) {
                Mouse::feed(0, 0, x, y, dx, dy);
                Multitouch::feed(0, 0, x, y, 0);

                // center cursor back
                XWindowAttributes wa;
                XGetWindowAttributes(dpy, win, &wa);
                g_lastX = wa.width / 2;
                g_lastY = wa.height / 2;
                XWarpPointer(dpy, None, win, 0, 0, 0, 0, g_lastX, g_lastY);
            }
        } else {
            Mouse::feed(0, 0, x, y, 0, 0);
            Multitouch::feed(0, 0, x, y, 0);
        }
        break;
    }
    default:
        break;
    }
}

int main(int argc, char** argv) {
	if (argc > 0 && argv[0]) {
		std::string path = argv[0];
		std::size_t pos = path.find_last_of('/');
		if (pos != std::string::npos) {
			path = path.substr(0, pos);
			chdir(path.c_str());
		}
	}

	Display* dpy = XOpenDisplay(nullptr);
	if (!dpy) return 1;

	int screen = DefaultScreen(dpy);
	Window root = RootWindow(dpy, screen);

	int width = 848;
	int height = 480;

	Window win = XCreateSimpleWindow(
		dpy, root, 0, 0, static_cast<unsigned int>(width), static_cast<unsigned int>(height),
		0, BlackPixel(dpy, screen), BlackPixel(dpy, screen)
	);

	g_dpy = dpy;
	g_win = win;

	XStoreName(dpy, win, "Pewiku");
	XSelectInput(dpy, win, ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDelete, 1);

	std::vector<unsigned long> netWmIcon;
	if (loadNetWmIcon("data/app/ios/icons/Icon.png", netWmIcon)) {
		Atom netWmIconAtom = XInternAtom(dpy, "_NET_WM_ICON", False);
		if (netWmIconAtom != None) {
			XChangeProperty(dpy, win, netWmIconAtom, XA_CARDINAL, 32, PropModeReplace, (const unsigned char*)netWmIcon.data(), (int)netWmIcon.size());
		}
	}

	XMapWindow(dpy, win);
	XFlush(dpy);

	// EGL init 
	EGLDisplay eglDisplay = eglGetDisplay((EGLNativeDisplayType)dpy);
	eglInitialize(eglDisplay, nullptr, nullptr);

	static const EGLint eglConfigAttribs[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 16, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT, EGL_NONE };
	EGLConfig config = nullptr;
	EGLint numConfigs = 0;
	if (!eglChooseConfig(eglDisplay, eglConfigAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
		std::fprintf(stderr, "eglChooseConfig failed\n");
		return 1;
	}

	static const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 1, EGL_NONE };
	eglBindAPI(EGL_OPENGL_ES_API);

	EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config, (EGLNativeWindowType)win, nullptr);
	EGLContext eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT, ctxAttribs);
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	glInit();

	AppContext appContext;
	appContext.display = eglDisplay;
	appContext.context = eglContext;
	appContext.surface = eglSurface;
	appContext.doRender = true;

	class AppPlatform_unix : public AppPlatform {
	public:
		bool isTouchscreen() { return false; }
		TextureData loadTexture(const std::string& filename_, bool textureFolder) {
			TextureData out;
			std::string filename = textureFolder ? std::string("data/images/") + filename_ : filename_;
			std::ifstream source(filename.c_str(), std::ios::binary);
			if (source) {
				png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
				if (!pngPtr)
					return out;

				png_infop infoPtr = png_create_info_struct(pngPtr);
				if (!infoPtr) {
					png_destroy_read_struct(&pngPtr, NULL, NULL);
					return out;
				}

				png_set_read_fn(pngPtr, (png_voidp)&source, [](png_structp pngPtr, png_bytep data, png_size_t length) {
					((std::istream*)png_get_io_ptr(pngPtr))->read((char*)data, length);
				});
				png_read_info(pngPtr, infoPtr);
				out.w = png_get_image_width(pngPtr, infoPtr);
				out.h = png_get_image_height(pngPtr, infoPtr);
				png_bytep* rowPtrs = new png_bytep[out.h];
				out.data = new unsigned char[4 * out.w * out.h];
				out.memoryHandledExternally = false;
				int rowStrideBytes = 4 * out.w;
				for (int i = 0; i < out.h; ++i) {
					rowPtrs[i] = (png_bytep)&out.data[i * rowStrideBytes];
				}
				png_read_image(pngPtr, rowPtrs);
				png_destroy_read_struct(&pngPtr, &infoPtr, (png_infopp)0);
				delete[] (png_bytep)rowPtrs;
				source.close();
				return out;
			} else {
				LOGI("Couldn't find file: %s\n", filename.c_str());
				return out;
			}
		}
		int getScreenWidth()  { return 848; }
		int getScreenHeight() { return 480; }
		float getPixelsPerMillimeter() { return 4.0f; }
		void saveScreenshot(const std::string&, int, int) {}
		void finish() {}
	};

	AppPlatform_unix* platform = new AppPlatform_unix();
	appContext.platform = platform;

	App* app = new MAIN_CLASS();
	((MAIN_CLASS*)app)->externalStoragePath = std::string(getenv("HOME") ? getenv("HOME") : ".") + "/.pewiku/";
	((MAIN_CLASS*)app)->externalCacheStoragePath = ((MAIN_CLASS*)app)->externalStoragePath;

	app->init(appContext);
	app->setSize(width, height);

	while (g_running_unix && !app->wantToQuit()) {
		while (XPending(dpy)) {
			XEvent ev;
			XNextEvent(dpy, &ev);
			handleXEvent(dpy, win, ev, app);
		}
		app->update();
	}

	delete app;
	platform->finish();
	delete platform;

	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(eglDisplay, eglContext);
	eglDestroySurface(eglDisplay, eglSurface);
	eglTerminate(eglDisplay);

	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}

#endif /*MAIN_UNIX_H__*/