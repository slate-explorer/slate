// input_win.cpp

#ifdef _WIN32

#include "input.hpp"

#include <windows.h>

namespace input {

    void init() {
        HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

        if (handle == INVALID_HANDLE_VALUE ||
            handle == nullptr)
            return;

        DWORD mode = 0;

        if (!GetConsoleMode(handle, &mode))
            return;

        /*
         * Enable window and mouse input if you eventually
         * want Slate to support them.
         *
         * For now we mainly want keyboard events.
         */
        mode |= ENABLE_WINDOW_INPUT;
        mode &= ~ENABLE_PROCESSED_INPUT;

        SetConsoleMode(handle, mode);
    }


    void process() {

        HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);

        if (handle == INVALID_HANDLE_VALUE ||
            handle == nullptr)
            return;

        DWORD available = 0;

        if (!GetNumberOfConsoleInputEvents(
            handle,
            &available))
            return;

        while (available > 0) {

            INPUT_RECORD record;
            DWORD read = 0;

            if (!ReadConsoleInputW(
                handle,
                &record,
                1,
                &read))
                break;

            --available;

            if (record.EventType != KEY_EVENT)
                continue;

            KEY_EVENT_RECORD& key =
                record.Event.KeyEvent;

            /*
             * We only want key-down events.
             */
            if (!key.bKeyDown)
                continue;

            DWORD state =
                key.dwControlKeyState;

            bool ctrl =
                (state & LEFT_CTRL_PRESSED) ||
                (state & RIGHT_CTRL_PRESSED);

            bool alt =
                (state & LEFT_ALT_PRESSED) ||
                (state & RIGHT_ALT_PRESSED);

            bool shift =
                (state & SHIFT_PRESSED);

            /*
             * Convert Windows virtual keys into
             * Slate's platform-independent keys.
             */
            switch (key.wVirtualKeyCode) {

            case VK_UP:

                input::queue.push({
                    Key::Up,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_DOWN:

                input::queue.push({
                    Key::Down,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_LEFT:

                input::queue.push({
                    Key::Left,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_RIGHT:

                input::queue.push({
                    Key::Right,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_HOME:

                input::queue.push({
                    Key::Home,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_END:

                input::queue.push({
                    Key::End,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_PRIOR:

                input::queue.push({
                    Key::PageUp,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_NEXT:

                input::queue.push({
                    Key::PageDown,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_INSERT:

                input::queue.push({
                    Key::Insert,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_DELETE:

                input::queue.push({
                    Key::Delete,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_RETURN:

                input::queue.push({
                    Key::Enter,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_ESCAPE:

                input::queue.push({
                    Key::Escape,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_BACK:

                input::queue.push({
                    Key::Backspace,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_TAB:

                input::queue.push({
                    Key::Tab,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F1:

                input::queue.push({
                    Key::F1,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F2:

                input::queue.push({
                    Key::F2,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F3:

                input::queue.push({
                    Key::F3,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F4:

                input::queue.push({
                    Key::F4,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F5:

                input::queue.push({
                    Key::F5,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F6:

                input::queue.push({
                    Key::F6,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F7:

                input::queue.push({
                    Key::F7,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F8:

                input::queue.push({
                    Key::F8,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F9:

                input::queue.push({
                    Key::F9,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F10:

                input::queue.push({
                    Key::F10,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F11:

                input::queue.push({
                    Key::F11,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            case VK_F12:

                input::queue.push({
                    Key::F12,
                    L'\0',
                    ctrl,
                    alt,
                    shift
                    });

                break;


            default:

                /*
                 * UnicodeChar contains the actual character
                 * generated by the key press.
                 *
                 * This is preferable to converting the
                 * virtual-key code ourselves.
                 */
                if (key.uChar.UnicodeChar != L'\0') {

                    input::queue.push({
                        Key::Character,
                        key.uChar.UnicodeChar,
                        ctrl,
                        alt,
                        shift
                        });
                }

                break;
            }
        }
    }


    void shutdown() {
        /*
         * Nothing required here for the current
         * Windows implementation.
         *
         * If init() later changes more console
         * state, save the original mode and restore
         * it here.
         */
    }

}

#endif