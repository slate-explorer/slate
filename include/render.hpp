#pragma once

#include <string>

namespace render {
    void openWindow(const std::wstring name, int width, int height);
    void tickWindow();
    bool isRunning();
    void exit();
}