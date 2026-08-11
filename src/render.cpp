#include <string>
#include <windows.h>
#include "render.hpp"

#define HEX_RGB(hex) RGB( \
        ((hex >> 16) & 0xFF), \
        ((hex >> 8) & 0xFF), \
        (hex & 0xFF) \
    )

struct WindowData {
    bool running = true;
    HWND window = nullptr;
};

WindowData winDat;

namespace { // private

    LRESULT CALLBACK windowTick(HWND window, UINT message, WPARAM wp, LPARAM lp) {
        LRESULT res = 0;

        switch (message) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(window, &ps);

                const wchar_t* text = L"Hello from Slate";

                TextOutW(
                    dc,
                    20, 20,
                    text,
                    lstrlenW(text)
                );

                EndPaint(window, &ps);
                return 0;
            }


            case WM_CLOSE:
                DestroyWindow(window);
                return 0;

            case WM_DESTROY:
                winDat.running = false;
                PostQuitMessage(0);
                return 0;

            default: // tell windows to deal with it
                res = DefWindowProcW(window, message, wp, lp);
                break;
        }

        return res;
    }
}

namespace render { // public

    void openWindow(const std::wstring name, int width, int height) {
        WNDCLASSW wc = {sizeof(WNDCLASSW)};
        
        wc.hCursor = LoadCursorW(0, MAKEINTRESOURCEW(32512)); // the cursor
        wc.hInstance = GetModuleHandleW(0); // current program instance
        wc.hbrBackground = CreateSolidBrush(HEX_RGB(0x1e1e1e)); // background
        wc.lpszClassName = L"SlateClass"; // class name
        wc.style = CS_HREDRAW | CS_VREDRAW; // refresh on resize
        wc.lpfnWndProc = windowTick; // call back function

        if (!RegisterClassW(&wc)) {
            return;
        }


        HWND window = CreateWindowW(
            wc.lpszClassName, // name of group
            name.c_str(), // title
            WS_OVERLAPPEDWINDOW, // normal mode
            CW_USEDEFAULT, // pos x
            CW_USEDEFAULT, // pos y
            width, // size x
            height, // size y
            0, // no parent
            0, // no menu
            GetModuleHandleW(0), // current program instance
            0 // L-Params
        );

        // finished loading
        ShowWindow(window, SW_SHOW);
        UpdateWindow(window);

        winDat.window = window;
    }

    void tickWindow() {
        MSG message = {};
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE) > 0) { // remove all queued messages
            TranslateMessage(&message); // set to windows based messages
            DispatchMessageW(&message); // window callback
        }
    }

    bool isRunning() {
        return winDat.running;
    }

    void exit() {
        DestroyWindow(winDat.window);
    }

    void drawText(std::wstring text) {

    }
}
