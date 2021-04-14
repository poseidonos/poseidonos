#include <iostream>

#include "request.h"
#include "tool/cli_client/cli_client.h"

string Request::Make(int &argc, char *argv[])
{
    shared_ptr<Command> command = cmdList.GetCommand(argv[1]);

    if (command.get() != nullptr && argc >= 2)
    {
        if (command->Decode(argc, argv) == true)
        {
            return command->Serialize();
        }
        else
        {
            cout << "missing operand" << endl;
        }
    }
    else
    {
        cout << "command not found" << endl;
    }

    return "";
}

string Request::MountArray()
{
    int argc = 16;
    char *argv[argc];
    argv[0] = "cli_client";
    argv[1] = "mount_arr";
    argv[2] = "-ft";
    argv[3] = "1";
    argv[4] = "-b";
    argv[5] = "1";
    argv[6] = "b1";
    argv[7] = "-d";
    argv[8] = "4";
    argv[9] = "d1";
    argv[10] = "d2";
    argv[11] = "d3";
    argv[12] = "d4";
    argv[13] = "-s";
    argv[14] = "1";
    argv[15] = "s1";
    return Make(argc, argv);
}

string Request::ListDevice()
{
    int argc = 2;
    char *argv[argc];
    argv[0] = "cli_client";
    argv[1] = "list_dev";
    return Make(argc, argv);
}

string Request::SysState()
{
    int argc = 2;
    char *argv[argc];
    argv[0] = "cli_client";
    argv[1] = "sys_state";
    return Make(argc, argv);
}

string Request::ExitSys()
{
    int argc = 2;
    char *argv[argc];
    argv[0] = "cli_client";
    argv[1] = "exit_sys";
    return Make(argc, argv);
}