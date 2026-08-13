#pragma once

#include <string>
#include <windows.h>


#define HEX_RGB(hex) RGB( \
        ((hex >> 16) & 0xFF), \
        ((hex >> 8) & 0xFF), \
        (hex & 0xFF) \
    )

namespace render {
    extern COLORREF backgroundColor;
    extern COLORREF textColor;
     
    void setup();
    void openWindow(const std::wstring name, int width, int height);
    void tickWindow();
    bool isRunning();
    void exit();
    void drawText(std::wstring text, unsigned int line);
}