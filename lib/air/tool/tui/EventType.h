#ifndef AIR_TUI_EVENT_TYPE_H
#define AIR_TUI_EVENT_TYPE_H

namespace air
{
enum class EventType : int
{
    DEFAULT,
    TUI_EXIT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT,
    CLI_INIT,
    CLI_RUN,
    CLI_STOP,
    CLI_INTERVAL,
};

struct EventData
{
    int value{0};
    EventType type{EventType::DEFAULT};
};

} // namespace air

#endif // AIR_TUI_EVENT_TYPE_H
