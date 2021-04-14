#include <unistd.h>

#include "tool/tui/CliHandler.h"
#include "tool/tui/ConfigTree.h"
#include "tool/tui/EventType.h"
#include "tool/tui/FileDetector.h"
#include "tool/tui/KeyEventHandler.h"
#include "tool/tui/MoveHandler.h"
#include "tool/tui/Viewer.h"

using namespace air;

int
main()
{
    FileDetector file_detector;
    int pid = file_detector.Detect();
    if (-1 == pid)
    {
        return -1;
    }

    EventData event;
    KeyListener key_listener;
    MoveHandler move_handler;
    CLIHandler cli_handler;
    Viewer viewer;
    AConfig tree;

    while (EventType::TUI_EXIT != event.type)
    {
        event = key_listener.Listening();
        move_handler.HandleMove(event, tree);
        cli_handler.HandleCLI(event, tree, pid);
        viewer.Render(event, tree, pid);

        usleep(1000); // 1ms sleep
    }

    return 0;
}
