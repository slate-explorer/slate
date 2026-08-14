#pragma execution_character_set("utf-8")

#include <string>
#include <vector>
#include <iostream>

#pragma region MP-Imports
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#pragma endregion

#include "render.hpp"

struct Entry {
    bool isDir = false;
    std::wstring filePath;

};

struct WindowData {
    unsigned int width = 0;
    unsigned int height = 0;

    std::vector<Entry> entries;

    bool dirty = true;
};

WindowData winDat;

namespace { // private
#ifdef _WIN32
    HANDLE output = nullptr;
    HANDLE input = nullptr;

    DWORD originalOutputMode = 0;
    DWORD originalInputMode = 0;
#endif

    bool getSize() {
        int width;
        int height;

#ifdef _WIN32

        CONSOLE_SCREEN_BUFFER_INFO info;
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

        if (!GetConsoleScreenBufferInfo(output, &info))
            return false;

        width = info.srWindow.Right - info.srWindow.Left + 1;
        height = info.srWindow.Bottom - info.srWindow.Top + 1;

#else

        struct winsize size;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1)
            return false;

        width = static_cast<int>(size.ws_col);
        height = static_cast<int>(size.ws_row);

#endif

        bool resized =
            width != winDat.width ||
            height != winDat.height;

        winDat.width = width;
        winDat.height = height;

        return resized;
    }

    void hr() {
        for (int i = 0; i < winDat.width; ++i)
            std::cout << "─";
    }

    std::string rep(const std::string& text, int amount) {
        std::string res = "";
        for (int i = 0; i < amount; ++i)
            res += text;
        return res;
    }

    void repp(const std::string& text, int amount) {
        for (int i = 0; i < amount; ++i)
            std::cout << text;
    }
}

namespace render { // public

    bool initialise() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        output = GetStdHandle(STD_OUTPUT_HANDLE);
        input = GetStdHandle(STD_INPUT_HANDLE);

        if (output == INVALID_HANDLE_VALUE ||
            input == INVALID_HANDLE_VALUE)
            return false;

        if (!GetConsoleMode(output, &originalOutputMode))
            return false;

        if (!GetConsoleMode(input, &originalInputMode))
            return false;

        // Enable ANSI/VT output
        DWORD outputMode = originalOutputMode;
        outputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        if (!SetConsoleMode(output, outputMode))
            return false;
#endif

        // Enter Slate's terminal state
        std::cout << "\x1b[?1049h"; // alternate screen
        std::cout << "\x1b[?25l";   // hide cursor
        std::cout << "\x1b[?2004h"; // bracketed paste
        std::cout << "\x1b[2J";     // clear
        std::cout << "\x1b[H";      // home

        std::cout << "\033]0;" << "slate" << "\007"; // set title

        std::cout.flush();

        return true;
    }

    void clear() {
        std::cout << "\x1b[2J\x1b[H";
    }

    void draw() {
        // Box Outlines: ┌ ┐ └ ┘ ─ │ ├ ┤ ┬ ┴ ┼
        // clear screen
        clear();

        // begin title
        std::string cwd = "C:\\Here";// filesystem::dat::cwd;
        std::string name = "Slate";
        std::cout << name << rep(" ", winDat.width - (name.length() + cwd.length())) << cwd << "\n";
        hr();
    }

    bool needsRedraw() {
        if (getSize())
            winDat.dirty = true;

        if (winDat.dirty) {
            winDat.dirty = false;
            return true;
        }
           
        return false;
    }

    void dirty() {
        winDat.dirty = true;
    }

    void shutdown() {
        std::cout << "\x1b[0m";       // reset colours/styles
        std::cout << "\x1b[?25h";    // show cursor
        std::cout << "\x1b[?2004l";  // disable bracketed paste
        std::cout << "\x1b[?1006l";  // disable SGR mouse
        std::cout << "\x1b[?1003l";  // disable mouse
        std::cout << "\x1b[?1004l";  // disable focus events
        std::cout << "\x1b[?1049l";  // leave alternate screen

        std::cout.flush();

#ifdef _WIN32
        // Restore Windows console state
        SetConsoleMode(output, originalOutputMode);
        SetConsoleMode(input, originalInputMode);
#endif
    }
}
