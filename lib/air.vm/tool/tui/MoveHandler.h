#ifndef AIR_TUI_MOVE_HANDLER_H
#define AIR_TUI_MOVE_HANDLER_H

#include "tool/tui/ConfigTree.h"
#include "tool/tui/EventType.h"

namespace air
{
class MoveHandler
{
public:
    void HandleMove(EventData data, AConfig& tree);

private:
    void _MoveUp(AConfig& tree);
    void _MoveDown(AConfig& tree);
    void _MoveRight(AConfig& tree);
    void _MoveLeft(AConfig& tree);
};

} // namespace air

#endif // AIR_TUI_MOVE_HANDLER_H
