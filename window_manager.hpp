#ifndef SCWM_WINDOW_MANAGER_HPP
#define SCWM_WINDOW_MANAGER_HPP

extern "C" {
#include <X11/Xlib.h>
}
#include <memory>
#include <string>
#include<unordered_map>

class WindowManager {
    public:
        // Establish connection to server
        static ::std::unique_ptr<WindowManager> Create();
        // Disconnect froms server
        ~WindowManager();
        void Run();
	private:
		WindowManager(Display* display);
		Display* display_;
		const Window root_;
		::std::unordered_map<Window, Window> clients_;

		static int OnXError(Display* display, XErrorEvent* e);
		static int OnWMDetected(Display* display, XErrorEvent* e);
		static bool wm_detected_;

		void Frame(Window w, bool was_created_before_window_manager);
		void Unframe(Window w);

		void OnKeyPress(const XKeyPressedEvent& e);
		void OnCreateNotify(const XCreateWindowEvent& e);
		void OnDestroyNotify(const XDestroyWindowEvent& e);
		void OnMapNotify(const XMapEvent& e);
		void OnUnmapNotify(const XUnmapEvent& e);
		void OnReparentNotify(const XReparentEvent& e);
		void OnCirculateNotify(const XCirculateEvent& e);
		void OnConfigureNotify(const XConfigureEvent& e);
		void OnConfigureRequest(const XConfigureRequestEvent& e);
		void OnMapRequest(const XMapRequestEvent& e);

};

#endif
