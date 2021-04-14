
#ifndef AIR_CLI_RECV_H
#define AIR_CLI_RECV_H

#include <sys/types.h>

#include "tool/cli/CliResult.h"

namespace air
{
class CliRecv
{
public:
    CliRecv(CliResult* new_result, int new_pid)
    {
        cli_result = new_result;
        pid = new_pid;
        _Initialize();
    }
    ~CliRecv(void)
    {
        _Finalize();
    }
    void Receive(int num_cmd, int target_pid);
    void HandleTimeout(void);

private:
    void _Initialize(void);
    void _Finalize(void);

    key_t msg_q_key_id{0};

    struct msg_q_recv_st
    {
        long pid;
        int result;
        int cmd_type;
        int cmd_order;
    };

    struct msg_q_recv_st recv_msg
    {
        0,
    };
    CliResult* cli_result{nullptr};
    int pid{0};
    int target_pid_value{0};
    bool waiting{false};
};
} // end namespace air

#endif // AIR_CLI_RECV_H
