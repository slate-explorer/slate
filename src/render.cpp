#include <string>
#include <windows.h>
#include "render.hpp"

struct WindowData {
    bool running = true;
};

WindowData winDat;

namespace { // private
    LRESULT windowTick(HWND window, UINT message, WPARAM wp, LPARAM lp) {
        LRESULT res = 0;

        switch (message) {
            case WM_CLOSE: // shut down slate
                winDat.running = false;
                break;
            
            default: // tell windows to deal with it
                res = DefWindowProc(window, message, wp, lp);
                break;
        }

        return res;
    }
}

namespace render { // public
    void openWindow(const std::string name, int width, int height) {
        WNDCLASS wc = {sizeof(WNDCLASS)};
        
        wc.hCursor = LoadCursor(0, IDC_ARROW); // the cursor
        wc.hInstance = GetModuleHandle(0); // current program instance
        wc.lpszClassName = "SlateClass"; // class name
        wc.style = CS_HREDRAW | CS_VREDRAW; // refresh on resize
        wc.lpfnWndProc = windowTick; // call back function

        if (!RegisterClass(&wc))
            return;

        HWND window = CreateWindowA(
            wc.lpszClassName, // name of group
            "Slate", // title
            WS_OVERLAPPEDWINDOW | // normal mode
            WS_VISIBLE, // ensure vibility
            CW_USEDEFAULT, // pos x
            CW_USEDEFAULT, // pos y
            CW_USEDEFAULT, // size x
            CW_USEDEFAULT, // size y
            0, // no parent
            0, // no menu
            GetModuleHandle(0), // current program instance
            0
        );
    }
} 
