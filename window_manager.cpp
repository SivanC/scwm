#include "window_manager.hpp"
extern "C" {
#include <X11/Xutil.h>
#include <X11/keysym.h>
}
#include <glog/logging.h>

using ::std::unique_ptr;

bool WindowManager::wm_detected_;

unique_ptr<WindowManager> WindowManager::Create() {
	// open display
	Display* display = XOpenDisplay(nullptr);
	if (display == nullptr) {
		LOG(ERROR) << "Failed to open X display " << XDisplayName(nullptr);
		return nullptr;
	}
	// make wm instance
	return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
	: display_(CHECK_NOTNULL(display)),
	  root_(DefaultRootWindow(display_)) {
}

WindowManager::~WindowManager() {
	XCloseDisplay(display_);
}

void WindowManager::Run() {
	// init
	wm_detected_ = false;
	XSetErrorHandler(&WindowManager::OnWMDetected);
	XSelectInput(
		display_,
		root_,
		SubstructureRedirectMask | SubstructureNotifyMask);
	XSync(display_, false);
	if (wm_detected_) {
		LOG(ERROR) << "Detected another window manager on display "
				   << XDisplayString(display_);
		return;
	}
	XSetErrorHandler(&WindowManager::OnXError);

    XGrabServer(display_);
    Window returned_root, returned_parent;
    Window* top_level_windows;
    unsigned int num_top_level_windows;
    CHECK(XQueryTree(
            display_,
            root_,
            &returned_root,
            &returned_parent,
            &top_level_windows,
            &num_top_level_windows));
	CHECK_EQ(returned_root, root_);
	for (unsigned int i = 0; i < num_top_level_windows; ++i) {
		Frame(top_level_windows[i], true);
	}
	XFree(top_level_windows);
	XUngrabServer(display_);

	// loop
	for (;;) {
		// next event
		XEvent e;
		XNextEvent(display_, &e);
		LOG(INFO) << "Received event: " << e.type; //<< ToString(e);

		// dispatch
		switch (e.type) {
			// window state notification events
			case CreateNotify:
				OnCreateNotify(e.xcreatewindow);
				break;
			case DestroyNotify:
				OnDestroyNotify(e.xdestroywindow);
				break;
			case ReparentNotify:
				OnReparentNotify(e.xreparent);
				break;
			case CirculateNotify:
				OnCirculateNotify(e.xcirculate);
				break;
			case ConfigureNotify:
				OnConfigureNotify(e.xconfigure);
				break;
			case GravityNotify:
			case MapNotify:
				OnMapNotify(e.xmap);
				break;
			case MappingNotify:
                break;
			case UnmapNotify:
                OnUnmapNotify(e.xunmap);
                break;
			case VisibilityNotify:
                break;
			// keyboard
			case KeyPress:
				OnKeyPress(e.xkey);
                break;
			case KeyRelease:
                break;
			// pointer events
			case ButtonPress:
                break;
			case ButtonRelease:
                break;
			case MotionNotify:
                break;
			// window crossing
			case EnterNotify:
                break;
			case LeaveNotify:
                break;
			// input focus
			case FocusIn:
                break;
			case FocusOut:
                break;
			// keymap
			case KeymapNotify:
                break;
			// exposure
			case Expose:
                break;
			case GraphicsExpose:
                break;
			case NoExpose:
                break;
			// structure control
			case CirculateRequest:
                break;
			case ConfigureRequest:
				OnConfigureRequest(e.xconfigurerequest);
				break;
			case MapRequest:
				OnMapRequest(e.xmaprequest);
				break;
			case ResizeRequest:
                break;
			// colormap
			case ColormapNotify:
                break;
			// comms
			case ClientMessage:
                break;
			case PropertyNotify:
                break;
			case SelectionClear:
                break;
			case SelectionNotify:
                break;
			case SelectionRequest:
                break;
			default:
				LOG(WARNING) << "Ignored event";
		}
	}
}

int WindowManager::OnWMDetected(Display* display, XErrorEvent* e) {
	CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
	WindowManager::wm_detected_ = true;
	return 0;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) { 
	const int MAX_ERROR_TEXT_LENGTH = 1024;
	char error_text[MAX_ERROR_TEXT_LENGTH];
	XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
	LOG(ERROR) << "Received X error:\n"
			   << "		Request: " << int(e->request_code)
			   << " - " << e->request_code << "\n"
			   << "		Error code: " << int(e->error_code)
			   << " - " << error_text << "\n"
			   << " 	Resource ID: " << e->resourceid;
	return 0;
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
	XWindowChanges changes;
	changes.x = e.x;
	changes.y = e.y;
	changes.width = e.width;
	changes.height = e.height;
	changes.border_width = e.border_width;
	changes.sibling = e.above;
	changes.stack_mode = e.detail;

	if (clients_.count(e.window)) {
		const Window frame = clients_[e.window];
		XConfigureWindow(display_, frame, e.value_mask, &changes);
		LOG(INFO) << "Resize " << e.window << " to " << e.width << "x" << e.height;
	}
	
	XConfigureWindow(display_, e.window, e.value_mask, &changes);
	LOG(INFO) << "Resize " << e.window << " to " << e.width << "x" << e.height;
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
	Frame(e.window, false);
	XMapWindow(display_, e.window);
}

void WindowManager::Frame(Window w, bool was_created_before_window_manager) {
	const unsigned int BORDER_WIDTH  = 3;
	const unsigned long BORDER_COLOR = 0x6633bb;
	const unsigned long BG_COLOR 	 = 0x000000;

	XWindowAttributes x_window_attrs;
	CHECK(XGetWindowAttributes(display_, w, &x_window_attrs));

	if (was_created_before_window_manager) {
		if (x_window_attrs.override_redirect ||
			x_window_attrs.map_state != IsViewable) {
			return;
		}
	}

	const Window frame = XCreateSimpleWindow(
		display_,
		root_,
		x_window_attrs.x,
		x_window_attrs.y,
		1000,//x_window_attrs.width,
		x_window_attrs.height,
		BORDER_WIDTH,
		BORDER_COLOR,
		BG_COLOR);

	XSelectInput(
		display_,
		frame,
		SubstructureRedirectMask | SubstructureNotifyMask);

	XAddToSaveSet(display_, w);

	XReparentWindow(
		display_,
		w,
		frame,
		0, 0); // client window offset in frame	
	
	XMapWindow(display_, frame);

	clients_[w] = frame;

	// keymaps
	
	LOG(INFO) << "Framed window" << w << " [" << frame << "]";
}

void WindowManager::Unframe(Window w) {
    const Window frame = clients_[w];
    XUnmapWindow(display_, frame);
    XReparentWindow(
            display_,
            w,
            root_,
            0, 0);
    XRemoveFromSaveSet(display_, w);
    XDestroyWindow(display_, frame);
    clients_.erase(w);

    LOG(INFO) << "Unframed window " << w << " [" << frame << "]";
}

void WindowManager::OnReparentNotify(const XReparentEvent& e) {}

void WindowManager::OnMapNotify(const XMapEvent& e) {}

void WindowManager::OnUnmapNotify(const XUnmapEvent& e) {
    if (!clients_.count(e.window)) {
        LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
        return;
    }

	if (e.event == root_) {
		LOG(INFO) << "Ignore UnmapNotify for non-client window " << e.window;
		return;
	}

    Unframe(e.window);
}

void WindowManager::OnKeyPress(const XKeyPressedEvent& e) {
	switch(e.keycode) {
		default:
			Window win = XCreateSimpleWindow(display_, root_, 
					0, 0, 
					300, 300,
					0, 0, 0);
			Frame(win, false);
			XMapWindow(display_, win);
			break;
	}
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {}

void WindowManager::OnConfigureNotify(const XConfigureEvent& e) {}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {}

void WindowManager::OnCirculateNotify(const XCirculateEvent& e) {}
