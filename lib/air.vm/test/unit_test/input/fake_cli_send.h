
#include <sys/msg.h>

#include "src/input/In.h"

class FakeCliSend
{
public:
    FakeCliSend()
    {
        msg_q_key_id = msgget(getpid(), IPC_CREAT | 0666);
        if (msg_q_key_id == -1)
            exit(1);
    }
    virtual ~FakeCliSend()
    {
    }

    int
    FakeSend(int argc, char argv[])
    {
        send_msg_.pid = 1;
        send_msg_.cmd_order = 0;

        if (strcmp(argv, "air-run") == 0)
        {
            send_msg_.cmd_type = input::CommandType::CMD_AIR_RUN;
            send_msg_.cmd_int_value1 = 1; // bool true
        }
        else if (strcmp(argv, "air-stream-interval") == 0)
        {
            send_msg_.cmd_type = input::CommandType::CMD_AIR_STREAM_INTERVAL;
            send_msg_.cmd_int_value1 = 3; // 3 seconds
        }
        else if (strcmp(argv, "node-run") == 0)
        {
            send_msg_.cmd_type = input::CommandType::CMD_NODE_RUN;
            send_msg_.cmd_int_value1 = 1; // bool
            send_msg_.cmd_int_value2 = 0; // start node
            send_msg_.cmd_int_value3 = 1; // end node
        }
        else if (strcmp(argv, "node-init") == 0)
        {
            send_msg_.cmd_type = input::CommandType::CMD_NODE_INIT;
            send_msg_.cmd_int_value1 = 0; // start node
            send_msg_.cmd_int_value2 = 1; // end node
        }
        else if (strcmp(argv, "node-sample-ratio") == 0)
        {
            send_msg_.cmd_type = input::CommandType::CMD_NODE_SAMPLE_RATIO;
            send_msg_.cmd_int_value1 = 100; // sampling ratio
            send_msg_.cmd_int_value2 = 0;   // start node
            send_msg_.cmd_int_value3 = 1;   // end nodea
        }
        else
        {
            send_msg_.cmd_type = 99;
        }

        switch (argc)
        {
            case 1:
                strcpy(send_msg_.cmd_str_value, "node");
                break;
            case 2:
                strcpy(send_msg_.cmd_str_value, "range");
                break;
            case 3:
                strcpy(send_msg_.cmd_str_value, "all");
                break;
            case 4:
                strcpy(send_msg_.cmd_str_value, "group");
                break;
        }

        msgsnd(msg_q_key_id, &send_msg_,
            sizeof(struct msg_q_send_st) - sizeof(long), IPC_NOWAIT);

        return 0;
    }

    key_t msg_q_key_id{0};
    key_t msg_q_key_value{647};

    struct msg_q_send_st
    {
        long pid;
        int cmd_int_value1;
        int cmd_int_value2;
        int cmd_int_value3;
        char cmd_str_value[10];
        int cmd_type;
        int cmd_order;
    };
    struct msg_q_send_st send_msg_;
};
