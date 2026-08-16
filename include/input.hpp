#include <queue>
#include <string>

namespace input {
    
    extern std::wstring commandBar;

    enum class Key {
        Unknown,
        Character,
        Up,
        Down,
        Left,
        Right,
        Enter,
        Escape,
        Backspace,
        Tab,
        Home,
        End,
        PageUp,
        PageDown,
        Insert,
        Delete,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12
    };

    struct Event {
        Key key = Key::Unknown;
        wchar_t character = L'\0';

        bool ctrl = false;
        bool alt = false;
        bool shift = false;
    };
    
    extern std::queue<Event> queue;

    void init();
    void process();
    bool poll(Event& event);
    void shutdown();
}