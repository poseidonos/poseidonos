
#ifndef AIR_OUT_H
#define AIR_OUT_H

#include <sys/msg.h>
#include <sys/types.h>

#include "src/lib/Msg.h"

namespace output
{
class OutCommand
{
public:
    OutCommand(void)
    {
    }
    virtual ~OutCommand(void)
    {
    }
    virtual int Send(int pid, int ret_code, int cmd_type, int cmd_order);

private:
    struct SendMsgSt
    {
        int64_t pid;
        int ret_code;
        int cmd_type;
        int cmd_order;
    };

    struct SendMsgSt send_msg
    {
        0,
    };

    key_t msg_q_key_id{0};
    key_t msg_q_key_value{0};
};
} // namespace output

#endif // AIR_OUT_H
