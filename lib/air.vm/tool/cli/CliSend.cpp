
#include "tool/cli/CliSend.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

void
air::CliSend::_Initialize(void)
{
    for (int loop = 0; loop < CMD_MAX_COUNT; loop++)
    {
        cmd_duplicate_check[loop] = -1;
    }
}

void
air::CliSend::_Finalize(void)
{
}

int
air::CliSend::Send(int argc, char* argv[], int& target_pid)
{
    int num_send = 0;
    bool valid = _CheckRule(argc, argv);

    if (valid)
    {
        msg_q_key_id = msgget(target_pid_value, 0);

        if (-1 == msg_q_key_id)
        {
            cli_result->SetReturn(ReturnCode::ERR_MSG_Q_NOT_EXIST, "may use wrong pid");
            return 0;
        }

        if (1 == opt_air_run)
        {
            memset(&send_msg, 0, sizeof(struct msg_q_send_st));
            send_msg.pid = pid;
            send_msg.cmd_type = CMD_AIR_RUN;
            send_msg.cmd_int_value1 = (int)air_run_value;
            send_msg.cmd_order = cmd_duplicate_check[CMD_AIR_RUN];

            if (-1 == msgsnd(msg_q_key_id, (void*)&send_msg, sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT))
            {
                cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliSend::Send air-run");
            }
            else
            {
                num_send++;
            }
        }
        if (1 == opt_air_stream_interval)
        {
            memset(&send_msg, 0, sizeof(struct msg_q_send_st));
            send_msg.pid = pid;
            send_msg.cmd_type = CMD_AIR_STREAM_INTERVAL;
            send_msg.cmd_int_value1 = air_stream_interval_value;
            send_msg.cmd_order = cmd_duplicate_check[CMD_AIR_STREAM_INTERVAL];

            if (-1 == msgsnd(msg_q_key_id, &send_msg, sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT))
            {
                cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliSend::Send air-stream-interval");
            }
            else
            {
                num_send++;
            }
        }
        if (1 == opt_node_run)
        {
            memset(&send_msg, 0, sizeof(struct msg_q_send_st));
            send_msg.pid = pid;
            send_msg.cmd_type = CMD_NODE_RUN;
            memcpy(send_msg.cmd_str_value, node_run_type.c_str(), 10);
            send_msg.cmd_int_value2 = node_run_value1;      // start node
            send_msg.cmd_int_value3 = node_run_value2;      // end node
            send_msg.cmd_int_value1 = (int)node_run_enable; // bool
            send_msg.cmd_order = cmd_duplicate_check[CMD_NODE_RUN];

            if (-1 == msgsnd(msg_q_key_id, &send_msg, sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT))
            {
                cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliSend::Send node-run");
            }
            else
            {
                num_send++;
            }
        }
        if (1 == opt_node_init)
        {
            memset(&send_msg, 0, sizeof(struct msg_q_send_st));
            send_msg.pid = pid;
            send_msg.cmd_type = CMD_NODE_INIT;
            memcpy(send_msg.cmd_str_value, node_init_type.c_str(), 10);
            send_msg.cmd_int_value1 = node_init_value1; // node start
            send_msg.cmd_int_value2 = node_init_value2; // node end
            send_msg.cmd_order = cmd_duplicate_check[CMD_NODE_INIT];

            if (-1 == msgsnd(msg_q_key_id, &send_msg, sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT))
            {
                cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliSend::Send node-init");
            }
            else
            {
                num_send++;
            }
        }
        if (1 == opt_node_sample_ratio)
        {
            memset(&send_msg, 0, sizeof(struct msg_q_send_st));
            send_msg.pid = pid;
            send_msg.cmd_type = CMD_NODE_SAMPLE_RATIO;
            memcpy(send_msg.cmd_str_value, node_sample_ratio_type.c_str(), 10);
            send_msg.cmd_int_value1 = node_sample_ratio_value1; // ratio
            send_msg.cmd_int_value2 = node_sample_ratio_value2; // node start
            send_msg.cmd_int_value3 = node_sample_ratio_value3; // node end
            send_msg.cmd_order = cmd_duplicate_check[CMD_NODE_SAMPLE_RATIO];

            if (-1 == msgsnd(msg_q_key_id, &send_msg, sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT))
            {
                cli_result->SetReturn(ReturnCode::ERR_KERNEL_MSGQ_FAIL, "CliSend::Send node-sample-ratio");
            }
            else
            {
                num_send++;
            }
        }
    }

    target_pid = target_pid_value;

    return num_send;
}

bool
air::CliSend::_CheckAirAlive(void)
{
    bool alive{false};

    std::string sys_cmd;
    sys_cmd = "ps -e | grep " + std::to_string(target_pid_value);

    FILE* fp;
    fp = popen(sys_cmd.c_str(), "r");
    if (nullptr == fp)
    {
        return false;
    }

    char sys_result[1024]{
        0,
    };
    if (NULL != fgets(sys_result, 1024, fp))
    {
        if (nullptr != strstr(sys_result, std::to_string(target_pid_value).c_str()))
        {
            alive = true;
        }
    }

    pclose(fp);
    return alive;
}

bool
air::CliSend::_CheckRule(int argc, char* argv[])
{
    int index{0};
    int num_cmd = 0;

    while (argc - 1 > num_cmd)
    {
        int opt = getopt_long(argc, argv, "", options, &index);
        if (0 != opt)
        {
            cli_result->SetReturn(ReturnCode::ERR_INVALID_USAGE, "option: pid/air-run/air-stream-interval/node-run/node-init/node-sample-ratio");
            return false;
        }

        if (CMD_MAX_COUNT <= index)
        {
            cli_result->SetReturn(ReturnCode::ERR_INVALID_USAGE, "unsupported option");
            return false;
        }

        if (0 <= cmd_duplicate_check[index])
        {
            cli_result->SetReturn(ReturnCode::ERR_DUPLICATION);
            return false;
        }
        cmd_duplicate_check[index] = num_cmd;

        if (0 == opt)
        {
            if (0 == strcmp(options[index].name, "pid"))
            {
                if (false == _IsNumber(optarg))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_PID, "pid value must be an integer");
                    return false;
                }
                target_pid_value = std::stoi(optarg);
            }
            else if (0 == strcmp(options[index].name, "air-run"))
            {
                if (false == _CheckBoolOpt(BoolOption::BOOL_OPT_AIR, optarg))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_AIR_RUN, "air-run value must be a boolean");
                    return false;
                }
            }
            else if (0 == strcmp(options[index].name, "air-stream-interval"))
            {
                if (false == _IsNumber(optarg))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_AIR_STREAM_INTERVAL, "air-stream-interval value must be an integer");
                    return false;
                }
                air_stream_interval_value = std::stoi(optarg);
            }
            else if (0 == strcmp(options[index].name, "node-run"))
            {
                char* tok = strtok(optarg, "_");
                if (nullptr == tok)
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_RUN, "missing '_'");
                    return false;
                }

                if (false == _CheckBoolOpt(BoolOption::BOOL_OPT_NODE, tok))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_RUN, "missing boolean");
                    return false;
                }

                tok = strtok(NULL, "_");
                if (false == _CheckMultiOption(NodeOption::NODE_OPT_RUN, tok))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_RUN, "invalid format");
                    return false;
                }
            }
            else if (0 == strcmp(options[index].name, "node-init"))
            {
                if (false == _CheckMultiOption(NodeOption::NODE_OPT_INIT, optarg))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_INIT, "invalid format");
                    return false;
                }
            }
            else if (0 == strcmp(options[index].name, "node-sample-ratio"))
            {
                char* tok = strtok(optarg, "_");
                if (nullptr == tok)
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_SAMPLE_RATIO, "missing '_'");
                    return false;
                }

                if (false == _IsNumber(tok))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_SAMPLE_RATIO, "missing integer");
                    return false;
                }
                node_sample_ratio_value1 = std::stoi(tok);

                tok = strtok(NULL, "_");
                if (false == _CheckMultiOption(NodeOption::NODE_OPT_SAMPLE_RATIO, tok))
                {
                    cli_result->SetReturn(ReturnCode::ERR_OPT_NODE_SAMPLE_RATIO, "invalid format");
                    return false;
                }
            }
        }
        num_cmd++;
    } // end while

    if (-1 == target_pid_value)
    {
        cli_result->SetReturn(ReturnCode::ERR_MISSING_PID_OPTION);
        return false;
    }

    if (!_CheckAirAlive())
    {
        cli_result->SetReturn(ReturnCode::ERR_AIR_NOT_FOUND);
        return false;
    }

    return num_cmd > 0 ? true : false;
}

bool
air::CliSend::_CheckBoolOpt(BoolOption option, char* str)
{
    if (0 == strcmp(str, "false"))
    {
        if (BoolOption::BOOL_OPT_AIR == option)
        {
            air_run_value = false;
        }
        else if (BoolOption::BOOL_OPT_NODE == option)
        {
            node_run_enable = false;
        }
        return true;
    }
    else if (0 == strcmp(str, "true"))
    {
        if (BoolOption::BOOL_OPT_AIR == option)
        {
            air_run_value = true;
        }
        else if (BoolOption::BOOL_OPT_NODE == option)
        {
            node_run_enable = true;
        }
        return true;
    }

    return false;
}

bool
air::CliSend::_CheckMultiOption(NodeOption option, char* str)
{
    char* tok = strtok(str, ":");
    if (nullptr == tok)
    {
        return false;
    }

    if (0 == strcmp(tok, "node"))
    {
        _SetMultiOptionType(option, "node", 4);

        tok = strtok(NULL, ",");
        if (nullptr == tok)
        {
            return false;
        }

        if (false == _IsNumber(tok))
        {
            return false;
        }
        _SetMultiOptionValue1(option, std::stoi(tok));

        tok = strtok(NULL, ",");
        if (nullptr != tok)
        {
            return false;
        }
    }
    else if (0 == strcmp(tok, "range"))
    {
        _SetMultiOptionType(option, "range", 5);
        int value1, value2;

        tok = strtok(NULL, ",");
        if (nullptr == tok)
        {
            return false;
        }

        if (false == _IsNumber(tok))
        {
            return false;
        }
        value1 = std::stoi(tok);
        _SetMultiOptionValue1(option, value1);

        tok = strtok(NULL, ",");
        if (nullptr == tok)
        {
            return false;
        }
        if (false == _IsNumber(tok))
        {
            return false;
        }
        value2 = std::stoi(tok);
        _SetMultiOptionValue2(option, value2);

        tok = strtok(NULL, ",");
        if (nullptr != tok)
        {
            return false;
        }
    }
    else if (0 == strcmp(tok, "group"))
    {
        _SetMultiOptionType(option, "group", 5);

        tok = strtok(NULL, ",");
        if (nullptr == tok)
        {
            return false;
        }

        if (false == _IsNumber(tok))
        {
            return false;
        }
        _SetMultiOptionValue1(option, std::stoi(tok));

        tok = strtok(NULL, ",");
        if (nullptr != tok)
        {
            return false;
        }
    }
    else if (0 == strcmp(tok, "all"))
    {
        _SetMultiOptionType(option, "all", 3);

        tok = strtok(NULL, ",");
        if (nullptr != tok)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}

void
air::CliSend::_SetMultiOptionType(NodeOption option, const char* str, int str_len)
{
    if (NodeOption::NODE_OPT_RUN == option)
    {
        node_run_type.assign(str, str_len);
    }
    else if (NodeOption::NODE_OPT_INIT == option)
    {
        node_init_type.assign(str, str_len);
    }
    else if (NodeOption::NODE_OPT_SAMPLE_RATIO == option)
    {
        node_sample_ratio_type.assign(str, str_len);
    }
}

void
air::CliSend::_SetMultiOptionValue1(NodeOption option, int value1)
{
    if (NodeOption::NODE_OPT_RUN == option)
    {
        node_run_value1 = value1;
    }
    else if (NodeOption::NODE_OPT_INIT == option)
    {
        node_init_value1 = value1;
    }
    else if (NodeOption::NODE_OPT_SAMPLE_RATIO == option)
    {
        node_sample_ratio_value2 = value1;
    }
}

void
air::CliSend::_SetMultiOptionValue2(NodeOption option, int value2)
{
    if (NodeOption::NODE_OPT_RUN == option)
    {
        node_run_value2 = value2;
    }
    else if (NodeOption::NODE_OPT_INIT == option)
    {
        node_init_value2 = value2;
    }
    else if (NodeOption::NODE_OPT_SAMPLE_RATIO == option)
    {
        node_sample_ratio_value3 = value2;
    }
}

bool
air::CliSend::_IsNumber(const char* str)
{
    int len = strlen(str);
    for (int index = 0; index < len; index++)
    {
        if ('0' > str[index] || '9' < str[index])
        {
            return false;
        }
    }
    return true;
}
