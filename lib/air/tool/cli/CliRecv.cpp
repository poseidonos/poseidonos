
#include "tool/cli/CliRecv.h"

#include <sys/ipc.h>
#include <sys/msg.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

void
air::CliRecv::_Initialize(void)
{
    msg_q_key_id = msgget(pid, IPC_CREAT | 0666);
    if (-1 == msg_q_key_id)
    {
        std::cout << "msgget() failed!\n\n\n";
        exit(1);
    }
}

void
air::CliRecv::_Finalize(void)
{
    if (-1 != msg_q_key_id)
    {
        msgctl(msg_q_key_id, IPC_RMID, 0);
    }
}

void
air::CliRecv::Receive(int num_cmd, int target_pid)
{
    waiting = true;
    target_pid_value = target_pid;

    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        this->HandleTimeout();
    }).detach();

    for (int loop = 0; loop < num_cmd; loop++)
    {
        memset(&recv_msg, 0, sizeof(struct msg_q_recv_st));
        int recv_bytes = msgrcv(msg_q_key_id, (void*)&recv_msg,
            sizeof(struct msg_q_recv_st) - sizeof(long), pid, 0);
        if (0 < recv_bytes)
        {
            cli_result->SetReturn(ReturnCode::SUCCESS);
        }
        else
        {
            cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliRecv::Receive");
        }
    }

    waiting = false;
}

void
air::CliRecv::HandleTimeout(void)
{
    if (waiting)
    {
        cli_result->SetReturn(ReturnCode::ERR_TIMEOUT, "may use wrong pid");
        msgctl(msg_q_key_id, IPC_RMID, 0);
        key_t msg_q_key_target = msgget(target_pid_value, IPC_CREAT | 0666);
        msgctl(msg_q_key_target, IPC_RMID, 0);
    }
}