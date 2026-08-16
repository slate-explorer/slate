// input_unix.cpp

#if defined(__linux__) || defined(__APPLE__)

#include "input.hpp"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <string>

namespace input {

    namespace {
        termios originalTermios{};
        int originalFlags = -1;
        bool initialised = false;

        std::string escapeBuffer;

        // ------------------------------------------------------------
        // Queue helper
        // ------------------------------------------------------------

        void push(
            Key key,
            wchar_t character = L'\0',
            bool ctrl = false,
            bool alt = false,
            bool shift = false
        ) {
            input::queue.push({
                key,
                character,
                ctrl,
                alt,
                shift
                });
        }

        // ------------------------------------------------------------
        // UTF-8 decoder
        // ------------------------------------------------------------

        bool decodeUTF8(
            const std::string& data,
            std::size_t& index,
            wchar_t& output
        ) {
            if (index >= data.size())
                return false;

            const unsigned char c =
                static_cast<unsigned char>(data[index]);

            // ASCII
            if (c < 0x80) {
                output = static_cast<wchar_t>(c);
                ++index;
                return true;
            }

            std::uint32_t codepoint = 0;
            std::size_t length = 0;

            if ((c & 0xE0) == 0xC0) {
                codepoint = c & 0x1F;
                length = 2;
            }
            else if ((c & 0xF0) == 0xE0) {
                codepoint = c & 0x0F;
                length = 3;
            }
            else if ((c & 0xF8) == 0xF0) {
                codepoint = c & 0x07;
                length = 4;
            }
            else {
                ++index;
                return false;
            }

            if (index + length > data.size())
                return false;

            for (std::size_t i = 1; i < length; ++i) {

                const unsigned char next =
                    static_cast<unsigned char>(
                        data[index + i]
                        );

                if ((next & 0xC0) != 0x80) {
                    ++index;
                    return false;
                }

                codepoint =
                    (codepoint << 6) |
                    (next & 0x3F);
            }

            /*
             * wchar_t is 32-bit on Linux/macOS, so a Unicode
             * codepoint can be stored directly.
             *
             * This is not the case on Windows, where wchar_t
             * is 16-bit.
             */
            if (codepoint > 0x10FFFF ||
                (codepoint >= 0xD800 &&
                    codepoint <= 0xDFFF)) {

                ++index;
                return false;
            }

            output =
                static_cast<wchar_t>(codepoint);

            index += length;

            return true;
        }

        // ------------------------------------------------------------
        // ANSI escape sequence parser
        // ------------------------------------------------------------

        void processEscapeSequence() {

            if (escapeBuffer.empty())
                return;

            /*
             * A lone ESC.
             *
             * Don't process it immediately because the next
             * byte could be the beginning of an ANSI sequence.
             */
            if (escapeBuffer == "\x1b")
                return;

            /*
             * Alt + character:
             *
             * ESC + x
             */
            if (escapeBuffer.size() == 2 &&
                escapeBuffer[0] == '\x1b' &&
                escapeBuffer[1] != '[' &&
                escapeBuffer[1] != 'O') {

                unsigned char c =
                    static_cast<unsigned char>(
                        escapeBuffer[1]
                        );

                push(
                    Key::Character,
                    static_cast<wchar_t>(c),
                    false,
                    true,
                    false
                );

                escapeBuffer.clear();
                return;
            }

            /*
             * CSI sequences:
             *
             * ESC [
             */
            if (escapeBuffer.size() >= 3 &&
                escapeBuffer[0] == '\x1b' &&
                escapeBuffer[1] == '[') {

                const char final =
                    escapeBuffer.back();

                /*
                 * Simple navigation keys.
                 */
                if (escapeBuffer.size() == 3) {

                    switch (final) {

                    case 'A':
                        push(Key::Up);
                        escapeBuffer.clear();
                        return;

                    case 'B':
                        push(Key::Down);
                        escapeBuffer.clear();
                        return;

                    case 'C':
                        push(Key::Right);
                        escapeBuffer.clear();
                        return;

                    case 'D':
                        push(Key::Left);
                        escapeBuffer.clear();
                        return;

                    case 'H':
                        push(Key::Home);
                        escapeBuffer.clear();
                        return;

                    case 'F':
                        push(Key::End);
                        escapeBuffer.clear();
                        return;

                    case 'Z':
                        push(
                            Key::Tab,
                            L'\0',
                            false,
                            false,
                            true
                        );

                        escapeBuffer.clear();
                        return;

                    default:
                        break;
                    }
                }

                /*
                 * Keys ending in ~:
                 *
                 * ESC [ 1 ~
                 * ESC [ 2 ~
                 * etc.
                 */
                if (final == '~') {

                    if (escapeBuffer == "\x1b[1~") {
                        push(Key::Home);
                    }
                    else if (escapeBuffer == "\x1b[2~") {
                        push(Key::Insert);
                    }
                    else if (escapeBuffer == "\x1b[3~") {
                        push(Key::Delete);
                    }
                    else if (escapeBuffer == "\x1b[4~") {
                        push(Key::End);
                    }
                    else if (escapeBuffer == "\x1b[5~") {
                        push(Key::PageUp);
                    }
                    else if (escapeBuffer == "\x1b[6~") {
                        push(Key::PageDown);
                    }
                    else if (escapeBuffer == "\x1b[15~") {
                        push(Key::F5);
                    }
                    else if (escapeBuffer == "\x1b[17~") {
                        push(Key::F6);
                    }
                    else if (escapeBuffer == "\x1b[18~") {
                        push(Key::F7);
                    }
                    else if (escapeBuffer == "\x1b[19~") {
                        push(Key::F8);
                    }
                    else if (escapeBuffer == "\x1b[20~") {
                        push(Key::F9);
                    }
                    else if (escapeBuffer == "\x1b[21~") {
                        push(Key::F10);
                    }
                    else if (escapeBuffer == "\x1b[23~") {
                        push(Key::F11);
                    }
                    else if (escapeBuffer == "\x1b[24~") {
                        push(Key::F12);
                    }
                    else {
                        escapeBuffer.clear();
                        return;
                    }

                    escapeBuffer.clear();
                    return;
                }

                /*
                 * Modified cursor keys:
                 *
                 * ESC [ 1 ; 2 A
                 * ESC [ 1 ; 3 A
                 * ESC [ 1 ; 4 A
                 * ESC [ 1 ; 5 A
                 * ESC [ 1 ; 6 A
                 * ESC [ 1 ; 7 A
                 * ESC [ 1 ; 8 A
                 *
                 * 2 = Shift
                 * 3 = Alt
                 * 4 = Shift+Alt
                 * 5 = Ctrl
                 * 6 = Ctrl+Shift
                 * 7 = Ctrl+Alt
                 * 8 = Ctrl+Alt+Shift
                 */

                if (escapeBuffer.size() >= 6) {

                    const std::size_t semicolon =
                        escapeBuffer.find(';');

                    if (semicolon != std::string::npos &&
                        semicolon + 1 < escapeBuffer.size()) {

                        int modifier =
                            escapeBuffer[semicolon + 1] - '0';

                        bool ctrl = false;
                        bool alt = false;
                        bool shift = false;

                        switch (modifier) {

                        case 2:
                            shift = true;
                            break;

                        case 3:
                            alt = true;
                            break;

                        case 4:
                            shift = true;
                            alt = true;
                            break;

                        case 5:
                            ctrl = true;
                            break;

                        case 6:
                            ctrl = true;
                            shift = true;
                            break;

                        case 7:
                            ctrl = true;
                            alt = true;
                            break;

                        case 8:
                            ctrl = true;
                            alt = true;
                            shift = true;
                            break;

                        default:
                            break;
                        }

                        Key key = Key::Character;
                        bool valid = true;

                        switch (final) {

                        case 'A':
                            key = Key::Up;
                            break;

                        case 'B':
                            key = Key::Down;
                            break;

                        case 'C':
                            key = Key::Right;
                            break;

                        case 'D':
                            key = Key::Left;
                            break;

                        case 'H':
                            key = Key::Home;
                            break;

                        case 'F':
                            key = Key::End;
                            break;

                        default:
                            valid = false;
                            break;
                        }

                        if (valid) {
                            push(
                                key,
                                L'\0',
                                ctrl,
                                alt,
                                shift
                            );
                        }

                        escapeBuffer.clear();
                        return;
                    }
                }
            }

            /*
             * SS3 sequences.
             *
             * Some terminals use:
             *
             * ESC O P = F1
             * ESC O Q = F2
             * ESC O R = F3
             * ESC O S = F4
             */
            if (escapeBuffer.size() == 3 &&
                escapeBuffer[0] == '\x1b' &&
                escapeBuffer[1] == 'O') {

                switch (escapeBuffer[2]) {

                case 'P':
                    push(Key::F1);
                    break;

                case 'Q':
                    push(Key::F2);
                    break;

                case 'R':
                    push(Key::F3);
                    break;

                case 'S':
                    push(Key::F4);
                    break;

                case 'A':
                    push(Key::Up);
                    break;

                case 'B':
                    push(Key::Down);
                    break;

                case 'C':
                    push(Key::Right);
                    break;

                case 'D':
                    push(Key::Left);
                    break;

                default:
                    break;
                }

                escapeBuffer.clear();
                return;
            }

            /*
             * Protect against an endlessly growing buffer if
             * something unexpected arrives.
             */
            if (escapeBuffer.size() > 32) {
                push(Key::Escape);
                escapeBuffer.clear();
            }
        }

    }

    // ------------------------------------------------------------
    // Initialisation
    // ------------------------------------------------------------

    void init() {

        if (initialised)
            return;

        if (tcgetattr(
            STDIN_FILENO,
            &originalTermios
        ) == -1) {

            return;
        }

        originalFlags =
            fcntl(
                STDIN_FILENO,
                F_GETFL,
                0
            );

        if (originalFlags == -1)
            return;

        termios raw = originalTermios;

        /*
         * Disable:
         *
         * ICANON = line buffering
         * ECHO   = terminal echo
         *
         * ISIG   = terminal-generated signals
         */
        raw.c_lflag &= ~(
            ICANON |
            ECHO |
            ISIG
            );

        /*
         * Don't wait for input.
         */
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;

        if (tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &raw
        ) == -1) {

            return;
        }

        /*
         * Non-blocking stdin.
         */
        if (fcntl(
            STDIN_FILENO,
            F_SETFL,
            originalFlags | O_NONBLOCK
        ) == -1) {

            tcsetattr(
                STDIN_FILENO,
                TCSANOW,
                &originalTermios
            );

            return;
        }

        initialised = true;
    }

    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------

    void shutdown() {

        if (!initialised)
            return;

        tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &originalTermios
        );

        fcntl(
            STDIN_FILENO,
            F_SETFL,
            originalFlags
        );

        escapeBuffer.clear();

        initialised = false;
    }

    // ------------------------------------------------------------
    // Process input
    // ------------------------------------------------------------

    void process() {

        char buffer[256];

        while (true) {

            const ssize_t count =
                read(
                    STDIN_FILENO,
                    buffer,
                    sizeof(buffer)
                );

            if (count > 0) {

                for (
                    ssize_t i = 0;
                    i < count;
                    ) {

                    const unsigned char c =
                        static_cast<unsigned char>(
                            buffer[i]
                            );

                    /*
                     * Continue an existing escape sequence.
                     */
                    if (!escapeBuffer.empty()) {

                        escapeBuffer.push_back(
                            static_cast<char>(c)
                        );

                        ++i;

                        processEscapeSequence();

                        continue;
                    }

                    /*
                     * ESC starts an ANSI sequence.
                     */
                    if (c == 27) {

                        escapeBuffer = "\x1b";

                        ++i;

                        continue;
                    }

                    /*
                     * Ctrl+A through Ctrl+Z.
                     *
                     * 1  = Ctrl+A
                     * 2  = Ctrl+B
                     * ...
                     * 26 = Ctrl+Z
                     */
                    if (c >= 1 && c <= 26) {

                        const wchar_t character =
                            static_cast<wchar_t>(
                                L'a' + (c - 1)
                                );

                        push(
                            Key::Character,
                            character,
                            true,
                            false,
                            false
                        );

                        ++i;

                        continue;
                    }

                    /*
                     * Enter.
                     */
                    if (c == '\r' || c == '\n') {

                        push(Key::Enter);

                        ++i;

                        continue;
                    }

                    /*
                     * Backspace.
                     */
                    if (c == 8 || c == 127) {

                        push(Key::Backspace);

                        ++i;

                        continue;
                    }

                    /*
                     * Tab.
                     */
                    if (c == '\t') {

                        push(Key::Tab);

                        ++i;

                        continue;
                    }

                    /*
                     * Decode UTF-8.
                     */
                    std::string remaining(
                        buffer + i,
                        buffer + count
                    );

                    std::size_t offset = 0;
                    wchar_t character = L'\0';

                    if (decodeUTF8(
                        remaining,
                        offset,
                        character
                    )) {

                        push(
                            Key::Character,
                            character
                        );

                        i +=
                            static_cast<ssize_t>(
                                offset
                                );

                        continue;
                    }

                    /*
                     * Incomplete UTF-8 sequence.
                     *
                     * Keep it for the next read rather than
                     * corrupting it.
                     */
                    if (offset == 0 &&
                        (c & 0x80)) {

                        break;
                    }

                    ++i;
                }

                continue;
            }

            /*
             * No data available.
             *
             * O_NONBLOCK means EAGAIN/EWOULDBLOCK is normal.
             */
            if (count == -1 &&
                (errno == EAGAIN ||
                    errno == EWOULDBLOCK)) {

                break;
            }

            /*
             * Interrupted system call.
             */
            if (count == -1 &&
                errno == EINTR) {

                continue;
            }

            break;
        }
    }
}

#endif