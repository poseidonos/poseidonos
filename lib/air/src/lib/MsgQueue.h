
#ifndef AIR_MSG_QUEUE_H
#define AIR_MSG_QUEUE_H

#include <sys/ipc.h>
#include <sys/msg.h>

#include <cstring>
#include <iostream>

#include "src/lib/Msg.h"

namespace lib
{
const key_t FILE_MSG_Q_VALUE{8888};

class MsgQueue
{
public:
    explicit MsgQueue(key_t new_msg_q_key_value)
    {
        msg_q_key_value = new_msg_q_key_value;
        _Initialize();
    }
    ~MsgQueue(void)
    {
        if (nullptr != file_msg)
        {
            delete file_msg;
            file_msg = nullptr;
        }
    }
    bool
    Send(void)
    {
        if (msgsnd(msg_q_key_id, file_msg, sizeof(struct FileMsg) - sizeof(int64_t),
                IPC_NOWAIT) == -1)
        {
            return false;
        }
        return true;
    }
    FileMsg*
    GetCleanMsg(void)
    {
        file_msg->type = (int64_t)MsgType::FILE;
        memset(file_msg->name, 0, sizeof(file_msg->name));
        file_msg->air_pid = 0;
        file_msg->status = AirStatus::IDLE;
        memset(file_msg->time_stamp, 0, sizeof(file_msg->time_stamp));

        return file_msg;
    }
    FileMsg*
    Receive(void)
    {
        GetCleanMsg();
        int recv_bytes =
            msgrcv(msg_q_key_id, file_msg, sizeof(struct FileMsg) - sizeof(int64_t),
                (int64_t)MsgType::FILE, IPC_NOWAIT);
        if (-1 == recv_bytes)
        {
            return nullptr;
        }
        return file_msg;
    }

    void
    DeleteQueue(void)
    {
        // only air daemon can delete queue
        msgctl(msg_q_key_id, IPC_RMID, 0);
    }

private:
    void
    _Initialize(void)
    {
        file_msg = new FileMsg;
        msg_q_key_id = msgget(msg_q_key_value, IPC_CREAT | 0666);
        if (msg_q_key_id == -1)
        {
            std::cout << "msgget() failed!\n"
                      << std::endl;
            exit(1);
        }
    }

    key_t msg_q_key_id{0};
    key_t msg_q_key_value{0};
    struct FileMsg* file_msg{nullptr};
};

} // namespace lib

#endif // AIR_MSG_QUEUE_H
