
#ifndef AIR_IN_H
#define AIR_IN_H

#include <sys/msg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "src/lib/Design.h"

namespace input
{
enum CommandType
{
    CMD_PID = 0,
    CMD_AIR_RUN,
    CMD_AIR_STREAM_INTERVAL,
    CMD_NODE_RUN,
    CMD_NODE_INIT,
    CMD_NODE_SAMPLE_RATIO
};

class Subject : public lib_design::Subject
{
public:
    virtual ~Subject(void)
    {
    }
    Subject(void)
    {
    }
    virtual int Notify(uint32_t index, uint32_t type1, uint32_t type2,
        uint32_t value1, uint32_t value2, int pid, int cmd_type,
        int cmd_order);
};

class InCommand
{
public:
    explicit InCommand(input::Subject* new_subject)
    : subject(new_subject)
    {
        struct msqid_ds msq_stat
        {
            0,
        };

        msg_q_key_value = getpid();

        msg_q_key_id = msgget(msg_q_key_value, IPC_CREAT | 0666);

        msq_stat.msg_perm.uid = msg_q_key_value;
        msq_stat.msg_qbytes = 4096;               // set max number of bytes allowed in MsgQ
        msgctl(msg_q_key_id, IPC_SET, &msq_stat); // set AIR pid to kernel
    }
    ~InCommand(void)
    {
        msgctl(msg_q_key_id, IPC_RMID, 0);
    }
    int HandleKernelMsg(void);

private:
    void _CheckEnableCMD(void);
    void _CheckSetStreamingIntervalCMD(void);
    void _SetEnableNodeValue(uint32_t* type2, uint32_t* value1);
    void _CheckEnableNodeCMD(void);
    void _CheckDisableNodeCMD(void);
    void _CheckInitNodeCMD(void);
    void _CheckSetSamplingRatioCMD(void);
    void _NotifyToPolicy(uint32_t type2, uint32_t value1, uint32_t value2,
        int pid, int64_t cmd_type, int cmd_order);

    input::Subject* subject{nullptr};
    key_t msg_q_key_id{0};
    key_t msg_q_key_value{0};
    struct RecvMsgSt
    {
        int64_t pid;
        int cmd_int_value1;
        int cmd_int_value2;
        int cmd_int_value3;
        char cmd_str_value[10];
        int cmd_type;
        int cmd_order;
    };
    struct RecvMsgSt recv_msg
    {
        0,
    };
};

} // namespace input

#endif // AIR_IN_H
