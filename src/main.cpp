#include <iostream>
#include "render.hpp"

int main() {
    std::cout << "Opening Slate" << std::endl;

    render::openWindow("Slate", 500, 500);

    return 0;
}