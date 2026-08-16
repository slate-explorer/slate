#include <queue>
#include "input.hpp"

#pragma region MP-Imports
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#pragma endregion


namespace input {
    std::wstring commandBar;
    std::queue<input::Event> queue;

    bool poll(Event& event) {
        if (queue.empty())
            return false;

        event = queue.front();
        queue.pop();

        return true;
    }
}