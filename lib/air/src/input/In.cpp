
#include "src/input/In.h"

#include <cstring>
#include <iostream>

#include "src/lib/Protocol.h"

int
input::Subject::Notify(uint32_t index, uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2, int pid,
    int cmd_type, int cmd_order)
{
    if (index < to_dtype(pi::InSubject::COUNT))
    {
        arr_observer[index]->Update(type1, type2, value1, value2, pid, cmd_type,
            cmd_order);
        return 0;
    }

    return -1;
}

void
input::InCommand::_NotifyToPolicy(uint32_t type2, uint32_t value1,
    uint32_t value2, int pid, int64_t cmd_type, int cmd_order)
{
    subject->Notify(to_dtype(pi::InSubject::TO_POLICY),
        to_dtype(pi::Type1::INPUT_TO_POLICY), type2, value1, value2,
        pid, cmd_type, cmd_order);
}

void
input::InCommand::_CheckEnableCMD(void)
{
    _NotifyToPolicy(to_dtype(pi::Type2::ENABLE_AIR), recv_msg.cmd_int_value1, 0,
        static_cast<int>(recv_msg.pid), recv_msg.cmd_type,
        recv_msg.cmd_order);
}

void
input::InCommand::_CheckSetStreamingIntervalCMD(void)
{
    uint32_t type2 = to_dtype(pi::Type2::SET_STREAMING_INTERVAL);
    uint32_t value1 = recv_msg.cmd_int_value1; // interval
    uint32_t value2 = 0;
    _NotifyToPolicy(type2, value1, value2, recv_msg.pid, recv_msg.cmd_type,
        recv_msg.cmd_order);
}

void
input::InCommand::_SetEnableNodeValue(uint32_t* type2, uint32_t* value2)
{
    if (strcmp(recv_msg.cmd_str_value, "node") == 0)
    {
        (*type2) = to_dtype(pi::Type2::ENABLE_NODE);
        (*value2) = recv_msg.cmd_int_value2;
    }
    else if (strcmp(recv_msg.cmd_str_value, "range") == 0)
    {
        (*type2) = to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE);
        (*value2) = (recv_msg.cmd_int_value2 << 16) + recv_msg.cmd_int_value3;
    }
    else if (strcmp(recv_msg.cmd_str_value, "group") == 0)
    {
        (*type2) = to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP);
        (*value2) = recv_msg.cmd_int_value2;
    }
    else if (strcmp(recv_msg.cmd_str_value, "all") == 0)
    {
        (*type2) = to_dtype(pi::Type2::ENABLE_NODE_ALL);
    }
}

void
input::InCommand::_CheckEnableNodeCMD(void)
{
    uint32_t type2{0};                         // command
    uint32_t value1 = recv_msg.cmd_int_value1; // bool
    uint32_t value2 = recv_msg.cmd_int_value2; // node_id range
    _SetEnableNodeValue(&type2, &value2);
    _NotifyToPolicy(type2, value1, value2, recv_msg.pid, recv_msg.cmd_type,
        recv_msg.cmd_order);
}

void
input::InCommand::_CheckInitNodeCMD(void)
{
    uint32_t value1{0};
    uint32_t value2{0};
    uint32_t type2{0};

    if (strcmp(recv_msg.cmd_str_value, "node") == 0)
    {
        type2 = to_dtype(pi::Type2::INITIALIZE_NODE);
        value1 = recv_msg.cmd_int_value1;
    }
    else if (strcmp(recv_msg.cmd_str_value, "range") == 0)
    {
        type2 = to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE);
        value1 = (recv_msg.cmd_int_value1 << 16) + recv_msg.cmd_int_value2;
    }
    else if (strcmp(recv_msg.cmd_str_value, "group") == 0)
    {
        type2 = to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP);
        value1 = recv_msg.cmd_int_value1;
    }
    else if (strcmp(recv_msg.cmd_str_value, "all") == 0)
    {
        type2 = to_dtype(pi::Type2::INITIALIZE_NODE_ALL);
    }

    _NotifyToPolicy(type2, value1, value2, recv_msg.pid, recv_msg.cmd_type,
        recv_msg.cmd_order);
}

void
input::InCommand::_CheckSetSamplingRatioCMD(void)
{
    uint32_t type2{0};                         // command
    uint32_t value1 = recv_msg.cmd_int_value1; // ratio
    uint32_t value2 = {0};                     // node_id range

    if (strcmp(recv_msg.cmd_str_value, "node") == 0)
    {
        type2 = to_dtype(pi::Type2::SET_SAMPLING_RATE);
        value2 = recv_msg.cmd_int_value2;
    }
    else if (strcmp(recv_msg.cmd_str_value, "range") == 0)
    {
        type2 = to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE);
        value2 = (recv_msg.cmd_int_value2 << 16) + recv_msg.cmd_int_value3;
    }
    else if (strcmp(recv_msg.cmd_str_value, "group") == 0)
    {
        type2 = to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP);
        value2 = recv_msg.cmd_int_value2;
    }
    else if (strcmp(recv_msg.cmd_str_value, "all") == 0)
    {
        type2 = to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL);
    }

    _NotifyToPolicy(type2, value1, value2, recv_msg.pid, recv_msg.cmd_type,
        recv_msg.cmd_order);
}

int
input::InCommand::HandleKernelMsg(void)
{
    int num_enq = 0;
    memset(&recv_msg, 0, sizeof(struct RecvMsgSt));
    while (0 < msgrcv(msg_q_key_id, (void*)&recv_msg,
                   sizeof(struct RecvMsgSt) - sizeof(int64_t), 0, IPC_NOWAIT))
    {
        CommandType type = static_cast<CommandType>(recv_msg.cmd_type);
        switch (type)
        {
            case CMD_PID:
                break;
            case CMD_AIR_RUN:
                _CheckEnableCMD();
                num_enq++;
                break;
            case CMD_AIR_STREAM_INTERVAL:
                _CheckSetStreamingIntervalCMD();
                num_enq++;
                break;
            case CMD_NODE_RUN:
                _CheckEnableNodeCMD();
                num_enq++;
                break;
            case CMD_NODE_INIT:
                _CheckInitNodeCMD();
                num_enq++;
                break;
            case CMD_NODE_SAMPLE_RATIO:
                _CheckSetSamplingRatioCMD();
                num_enq++;
                break;
        }
        memset(&recv_msg, 0, sizeof(struct RecvMsgSt));
    }

    return num_enq;
}
