
#include "src/output/Out.h"

#include <iostream>

int
output::OutCommand::Send(int pid, int ret_code, int cmd_type, int cmd_order)
{
    int result = 0;
    struct SendMsgSt msg;
    msg.cmd_type = cmd_type;
    msg.ret_code = ret_code;
    msg.pid = pid;
    msg.cmd_order = cmd_order;

    msg_q_key_value = pid;
    msg_q_key_id = msgget(msg_q_key_value, IPC_CREAT | 0666);

    result = msgsnd(msg_q_key_id, (void*)&msg,
        sizeof(struct SendMsgSt) - sizeof(int64_t), IPC_NOWAIT);

    return result;
}
