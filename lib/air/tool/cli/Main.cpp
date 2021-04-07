
#include <stdio.h>
#include <unistd.h>

#include "tool/cli/CliRecv.h"
#include "tool/cli/CliSend.h"

int
main(int argc, char* argv[])
{
    int pid = getpid();
    air::CliResult* cli_result = new air::CliResult{};
    air::CliSend* cli_send = new air::CliSend{cli_result, pid};
    air::CliRecv* cli_recv = new air::CliRecv{cli_result, pid};

    int num_cmd{0};
    int target_pid{-1};

    num_cmd = cli_send->Send(argc, argv, target_pid);

    if (0 < num_cmd)
    {
        cli_recv->Receive(num_cmd, target_pid);
    }

    cli_result->PrintCliResult();

    delete cli_send;
    delete cli_recv;
    delete cli_result;
}
