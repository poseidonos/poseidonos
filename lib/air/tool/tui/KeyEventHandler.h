#ifndef AIR_TUI_KEY_EVENT_HANDLER_H
#define AIR_TUI_KEY_EVENT_HANDLER_H

#include <termios.h>

#include "tool/tui/EventType.h"

namespace air
{
enum KeyNormal
{
    SPECIAL = 27,
    NUM_1 = 49,
    NUM_9 = 57,
    KEY_I = 73,
    KEY_O = 79,
    KEY_Q = 81,
    KEY_X = 88,
    KEY_i = 105,
    KEY_o = 111,
    KEY_q = 113,
    KEY_x = 120,
};

enum KeySpecial
{
    KEY_ESC = -1,
    KEY_ARROW = 91,
};

enum KeyArrow
{
    KEY_ARROW_UP = 65,
    KEY_ARROW_DOWN = 66,
    KEY_ARROW_RIGHT = 67,
    KEY_ARROW_LEFT = 68,
};

class TerminalSetting
{
public:
    void SaveDefaultTermios(void);
    void RestoreDefaultTermios(void);
    struct termios default_termios;
};

class KeyListener
{
public:
    KeyListener()
    {
        setting.SaveDefaultTermios();
        _SetListenMode();
    }
    ~KeyListener()
    {
        setting.RestoreDefaultTermios();
    }
    EventData Listening(void);

private:
    void _SetListenMode(void);

    TerminalSetting setting;
};

} // namespace air

#endif // AIR_TUI_KEY_EVENT_HANDLER_H
