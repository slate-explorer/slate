#pragma execution_character_set("utf-8")

#include <string>
#include <vector>
#include <windows.h>
#include "render.hpp"

constexpr const wchar_t SLATE_FONT[] = L"Atkinson Hyperlegible Mono";

struct Entry {
    bool isDir = false;
    std::wstring filePath;

};

struct TextSpan {
    std::wstring text;
    bool bold = false;
};

struct Line {
    std::vector<TextSpan> spans;
};

struct WindowData {
    bool running = true;
    HWND window = nullptr;

    unsigned int width = 0;
    unsigned int height = 0;

    PAINTSTRUCT ps = {};
    HDC dc = nullptr;

    HFONT fontReg = nullptr;
    HFONT fontBold = nullptr;
    unsigned int charWidth = NULL;
    unsigned int charHeight = NULL;

    std::vector<Entry> entries;

    std::vector<Line> lines;
};

WindowData winDat;

namespace { // private
    void loadFont() {
        AddFontResourceExW(
            L"resources/fonts/AtkinsonHyperlegibleMono-Regular.ttf",
            FR_PRIVATE,
            nullptr
        );

        AddFontResourceExW(
            L"resources/fonts/AtkinsonHyperlegibleMono-Bold.ttf",
            FR_PRIVATE,
            nullptr
        );

        winDat.fontReg = CreateFontW(
            18, 0, 0, 0,
            FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            SLATE_FONT
        );

        winDat.fontBold = CreateFontW(
            18, 0, 0, 0,
            FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH,
            SLATE_FONT
        );
    }

// Box Outlines: ┌ ┐ └ ┘ ─ │ ├ ┤ ┬ ┴ ┼

    void drawLines() {
        Line line;
        TextSpan span;
        unsigned int sizeX = 0; // how many letters across
        for (unsigned int i = 0; i < winDat.lines.size(); i++) {
            line = winDat.lines[i];
            for (unsigned int j = 0; j < line.spans.size(); j++) {
                span = line.spans[j];
                SelectObject(winDat.dc, span.bold ? winDat.fontBold : winDat.fontReg);
                SetTextColor(winDat.dc, render::textColor);
                SetBkColor(winDat.dc, render::backgroundColor);

                TextOutW(winDat.dc, sizeX*winDat.charWidth, i*winDat.charHeight, span.text.c_str(), span.text.length());
                
                sizeX += span.text.length();
            }
            sizeX = 0;
        }
    }

    void updateEntry(const Entry entry) {

    }

    void getCharacterWidthHeight() {
        HDC dc = GetDC(winDat.window);

        HFONT oldFont = (HFONT)SelectObject(dc, winDat.fontReg);

        SIZE size;
        GetTextExtentPoint32W(dc, L"─", 1, &size);

        SelectObject(dc, oldFont);
        ReleaseDC(winDat.window, dc);

        winDat.charWidth = size.cx;
        winDat.charHeight = size.cy;
    }

    void updateLines() {
        std::wstring cwd = L"C:\\Here";  // TODO - add filesystem::cwd

        SelectObject(winDat.dc, winDat.fontReg);
        winDat.lines.clear();

        winDat.lines.push_back(Line{ { {L"┌", false}, { std::wstring(winDat.width/winDat.charWidth-2, L'─'), false }, {L"┐", false} } });
        winDat.lines.push_back(Line{ { {L"│ ", false}, {L"Slate", true}, {L" │ ", false}, {std::wstring(winDat.width / winDat.charWidth - (12+cwd.length()), L' '), false}, {cwd, false}, {L" │", false}}});
        for (const Entry entry : winDat.entries) {
            updateEntry(entry);
        }
    }

    LRESULT CALLBACK windowTick(HWND window, UINT message, WPARAM wp, LPARAM lp) {
        LRESULT res = 0;

        switch (message) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(window, &ps);
                winDat.dc = dc;

                drawLines();

                EndPaint(window, &ps);
                return 0;
            }
            
            case WM_CREATE:
            case WM_SIZE:
                winDat.width = LOWORD(lp);
                winDat.height = HIWORD(lp);

                updateLines();

                InvalidateRect(window, nullptr, FALSE);
                return 0;
                break;

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
    COLORREF backgroundColor = HEX_RGB(0x1e1e1e);
    COLORREF textColor = HEX_RGB(0xffffff);

    void setup() {
        loadFont();
        getCharacterWidthHeight();
    }

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
        if (GetMessage(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    bool isRunning() {
        return winDat.running;
    }

    void exit() {
        DestroyWindow(winDat.window);
    }

    void drawText(std::wstring text, unsigned int line) {
        
    }
}
