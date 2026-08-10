#include <string>
#include <windows.h>
#include "render.hpp"

struct WindowData {
    bool running = true;
};

WindowData winDat;

namespace { // private
    LRESULT windProc(HWND window, UINT message, WPARAM wp, LPARAM lp) {
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
        
    }
}
