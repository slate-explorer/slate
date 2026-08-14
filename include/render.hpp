#pragma once

#include <string>
#include <windows.h>


#define HEX_RGB(hex) RGB( \
        ((hex >> 16) & 0xFF), \
        ((hex >> 8) & 0xFF), \
        (hex & 0xFF) \
    )

namespace render {
    bool initialise();
    void clear();
    void draw();
    bool needsRedraw();
    void shutdown();
    inline void dirty();
}