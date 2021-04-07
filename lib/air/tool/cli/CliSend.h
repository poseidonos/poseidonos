
#ifndef AIR_CLI_SEND_H
#define AIR_CLI_SEND_H

#include <getopt.h>
#include <sys/types.h>

#include <string>

#include "tool/cli/CliLib.h"
#include "tool/cli/CliResult.h"

namespace air
{
class CliSend
{
public:
    CliSend(CliResult* new_result, int new_pid)
    {
        cli_result = new_result;
        pid = new_pid;
        _Initialize();
    }
    virtual ~CliSend(void)
    {
        _Finalize();
    }
    virtual int Send(int argc, char* argv[], int& target_pid);

private:
    enum BoolOption
    {
        BOOL_OPT_AIR,
        BOOL_OPT_NODE
    };
    enum NodeOption
    {
        NODE_OPT_RUN,
        NODE_OPT_INIT,
        NODE_OPT_SAMPLE_RATIO
    };
    void _Initialize(void);
    void _Finalize(void);
    bool _CheckAirAlive(void);
    bool _CheckRule(int argc, char* argv[]);
    bool _CheckBoolOpt(BoolOption option, char* str);
    bool _CheckSetMgmtServer(char* str);
    bool _CheckStreamInterval(char* str);
    bool _CheckMultiOption(NodeOption option, char* str);
    void _SetMultiOptionType(NodeOption option, const char* str, int str_len);
    void _SetMultiOptionValue1(NodeOption option, int value1);
    void _SetMultiOptionValue2(NodeOption option, int value2);
    bool _IsNumber(const char* str);

    int opt_pid{0};
    int target_pid_value{-1};

    int opt_air_run{0};
    bool air_run_value{false};

    int opt_air_stream_interval{0};
    int air_stream_interval_value{0};

    int opt_node_run{0};
    std::string node_run_type{""};
    int node_run_value1{0};
    int node_run_value2{0};
    bool node_run_enable{false};

    int opt_node_init{0};
    std::string node_init_type{""};
    int node_init_value1{0};
    int node_init_value2{0};

    int opt_node_sample_ratio{0};
    std::string node_sample_ratio_type{""};
    int node_sample_ratio_value1{100};
    int node_sample_ratio_value2{0};
    int node_sample_ratio_value3{0};

    struct option options[10]{
        {"pid", 1, &opt_pid, 1},
        {"air-run", 1, &opt_air_run, 1},
        {"air-stream-interval", 1, &opt_air_stream_interval, 1},
        {"node-run", 1, &opt_node_run, 1},
        {"node-init", 1, &opt_node_init, 1},
        {"node-sample-ratio", 1, &opt_node_sample_ratio, 1},
        {0, 0, 0, 0}};
    int cmd_duplicate_check[CMD_MAX_COUNT];

    key_t msg_q_key_id{0};

    struct msg_q_send_st
    {
        int64_t pid;
        int cmd_int_value1;
        int cmd_int_value2;
        int cmd_int_value3;
        char cmd_str_value[10];
        int cmd_type;
        int cmd_order;
    };
    struct msg_q_send_st send_msg
    {
        0,
    };

    CliResult* cli_result{nullptr};
    int pid{0};
};

} // namespace air

#endif // AIR_CLI_SEND_H
