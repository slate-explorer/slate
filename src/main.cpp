#include <iostream>
#include "render.hpp"
#include "input.hpp"

int main() {
    if (!render::initialise()) {
        std::cerr << "Terminal init failed\n";
        return 1;
    }


    bool running = true;

    while (running) {
        if (render::needsRedraw())
            render::draw();

        input::process();
        
        input::Event event;
        while (input::poll(event)) {
            if (event.key == input::Key::Character) {
                input::commandBar += event.character;
                render::dirtyCMD();
            }
        }
    }

    render::shutdown();

    return 0;
} 