#ifndef AIR_TUI_CLI_HANDLER_H
#define AIR_TUI_CLI_HANDLER_H

#include <string>

#include "tool/tui/ConfigTree.h"
#include "tool/tui/EventType.h"

namespace air
{
class CLIHandler
{
public:
    void HandleCLI(EventData data, AConfig& tree, int pid);

private:
    void _SendInit(AConfig& tree);
    void _SendRun(AConfig& tree);
    void _SendStop(AConfig& tree);
    void _SendInterval(int interval);
    void _SendCommandWithOption(AConfig& tree, std::string& cmd_str);
    void _AsyncSystemCall(std::string& cmd_str);

    int pid{-1};
};

} // namespace air

#endif // AIR_TUI_CLI_HANDLER_H
