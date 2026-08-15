#pragma execution_character_set("utf-8")

#include <string>
#include <vector>
#include <iostream>

#pragma region MP-Imports
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#pragma endregion

#include "render.hpp"
#include "filesystem.hpp"

struct WindowData {
    unsigned int width = 0;
    unsigned int height = 0;

    std::vector<filesystem::Entry> entries;

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

    struct DrawEntries {
        unsigned int cursorPos;
        std::vector<filesystem::Entry> entries;
    };

    DrawEntries getEntriesToDraw(
        const std::vector<filesystem::Entry>& entries,
        unsigned int cursor,
        unsigned int rah
    ) {
        DrawEntries result{};

        if (entries.empty() || rah == 0)
            return result;

        // Work out where the visible section starts
        unsigned int offset = 0;

        if (entries.size() > rah) {
            const unsigned int half = rah / 2;

            if (cursor > half)
                offset = cursor - half;

            if (offset > entries.size() - rah)
                offset = entries.size() - rah;
        }

        // Relative cursor position
        result.cursorPos = cursor - offset;

        unsigned int entryAmt = static_cast<unsigned int>(entries.size());

        // Entries to draw
        const unsigned int end = std::min(
            offset + rah,
            entryAmt
        );

        result.entries.assign(
            entries.begin() + offset,
            entries.begin() + end
        );

        return result;
    }

    void drawEntries(unsigned int rah) {
        DrawEntries toDraw = getEntriesToDraw(
            filesystem::getEntries(),
            0, // input::cursor.pos
            rah
        );

        filesystem::sortEntries(toDraw.entries);

        for (unsigned int i = 0; i < rah; i++) {
            if (i >= toDraw.entries.size()) {
                if (i > 0)
                    std::cout << "\n";
                continue;
            }

            const auto& entry = toDraw.entries[i];

            std::cout << (i == toDraw.cursorPos ? "> " : "  ");
            std::cout << entry.filePath.filename().string();
            std::cout << (entry.isDir ? "\\" : ""); 

            if (i + 1 < rah)
                std::cout << "\n";
        }
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
        std::string cwd = filesystem::cwd.string();
        std::string name = "Slate";
        std::cout << name << rep(" ", winDat.width - (name.length() + cwd.length())) << cwd << "\n";
        hr();

        // draw files
        drawEntries(winDat.height - (4));
        std::cout << "\n";

        // search bar
        hr();
        std::cout << ">> ";
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
