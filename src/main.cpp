#include <iostream>
#include "render.hpp"

int main() {
    if (!render::initialise()) {
        std::cerr << "Terminal init failed\n";
        return 1;
    }

    while (true) {
        if (render::needsRedraw())
            render::draw();
    }

    render::shutdown();

    return 0;
} 