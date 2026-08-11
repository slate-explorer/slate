#include <iostream>
#include "render.hpp"

int main() {
    render::openWindow(L"slate", 500, 500);

    while (render::isRunning()) {
        render::tickWindow();
    }

    return 0;
} 