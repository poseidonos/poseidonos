#include "tool/tui/KeyEventHandler.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

void
air::TerminalSetting::SaveDefaultTermios(void)
{
    tcgetattr(STDIN_FILENO, &default_termios);
}

void
air::TerminalSetting::RestoreDefaultTermios(void)
{
    int fd;

    tcsetattr(STDIN_FILENO, TCSANOW, &default_termios);

    fd = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (-1 != fd)
    {
        fd = fcntl(STDIN_FILENO, F_SETFL, fd);
        if (-1 == fd)
        {
            std::cout << "TerminalSetting::RestoreDefaultTermios failed\n";
        }
    }
}

void
air::KeyListener::_SetListenMode(void)
{
    int fd;
    struct termios instant_termios;

    instant_termios = setting.default_termios;
    instant_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &instant_termios);

    fd = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (-1 != fd)
    {
        fd = fcntl(STDIN_FILENO, F_SETFL, fd | O_NONBLOCK);
        if (-1 == fd)
        {
            std::cout << "KeyListener::_SetListenMode failed\n";
        }
    }
}

air::EventData
air::KeyListener::Listening(void)
{
    EventData event;

    int getchar_result;
    getchar_result = getchar();

    if (EOF != getchar_result)
    {
        switch (getchar_result)
        {
            case KeyNormal::SPECIAL:
            {
                getchar_result = getchar();
                if (KeySpecial::KEY_ESC == getchar_result)
                {
                    event.type = air::EventType::TUI_EXIT;
                }
                else if (KeySpecial::KEY_ARROW == getchar_result)
                {
                    getchar_result = getchar();
                    switch (getchar_result)
                    {
                        case KeyArrow::KEY_ARROW_UP:
                        {
                            event.type = air::EventType::MOVE_UP;
                            break;
                        }
                        case KeyArrow::KEY_ARROW_DOWN:
                        {
                            event.type = air::EventType::MOVE_DOWN;
                            break;
                        }
                        case KeyArrow::KEY_ARROW_RIGHT:
                        {
                            event.type = air::EventType::MOVE_RIGHT;
                            break;
                        }
                        case KeyArrow::KEY_ARROW_LEFT:
                        {
                            event.type = air::EventType::MOVE_LEFT;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }

                break;
            }
            case KeyNormal::KEY_Q:
            case KeyNormal::KEY_q:
            {
                event.type = air::EventType::TUI_EXIT;
                break;
            }
            case KeyNormal::KEY_O:
            case KeyNormal::KEY_o:
            {
                event.type = air::EventType::CLI_RUN;
                break;
            }
            case KeyNormal::KEY_X:
            case KeyNormal::KEY_x:
            {
                event.type = air::EventType::CLI_STOP;
                break;
            }
            case KeyNormal::KEY_I:
            case KeyNormal::KEY_i:
            {
                event.type = air::EventType::CLI_INIT;
                break;
            }
            case KeyNormal::NUM_1... KeyNormal::NUM_9:
            {
                event.type = air::EventType::CLI_INTERVAL;
                event.value = getchar_result - 48;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return event;
}
