
#ifndef AIR_CLI_LIB_H
#define AIR_CLI_LIB_H

#include <string>

namespace air
{
enum CommandType
{
    CMD_PID = 0,
    CMD_AIR_RUN,
    CMD_AIR_STREAM_INTERVAL,
    CMD_NODE_RUN,
    CMD_NODE_INIT,
    CMD_NODE_SAMPLE_RATIO,
    CMD_MAX_COUNT
};

enum ReturnCode
{
    SUCCESS = 0,
    ERR_INVALID_USAGE,
    ERR_OPT_PID,
    ERR_OPT_AIR_RUN,
    ERR_OPT_AIR_STREAM_INTERVAL,
    ERR_OPT_NODE_RUN,
    ERR_OPT_NODE_INIT,
    ERR_OPT_NODE_SAMPLE_RATIO,
    ERR_OUT_OF_RANGE,
    ERR_MISSING_PID_OPTION,
    ERR_AIR_NOT_FOUND,
    ERR_KERNEL_MSGQ_FAIL,
    ERR_DUPLICATION,
    ERR_TIMEOUT,
    ERR_MSG_Q_NOT_EXIST,
    ERR_UNKNOWN,
    RC_COUNT
};

static const std::string ReturnMsg[RC_COUNT]{
    "[SUCCESS]",
    "[ERR_INVALID_USAGE] Command not found.",
    "[ERR_OPT_PID]",
    "[ERR_OPT_AIR_RUN] ex) --pid=<target_pid> --air-run=<bool>",
    "[ERR_OPT_AIR_STREAM_INTERVAL] ex) --pid=<target_pid> --air-stream-interval=<1~60>",
    "[ERR_OPT_NODE_RUN] ex) --pid=<target_pid> --node-run=<node/range/all>",
    "[ERR_OPT_NODE_INIT] ex) --pid=<target_pid> --node-init=<node/range/all>",
    "[ERR_OPT_NODE_SAMPLE_RATIO] ex) --pid=<target_pid> --node-sample-ratio=<1~10000>",
    "[ERR_OUT_OF_RANGE]",
    "[ERR_MISSING_PID_OPTION]",
    "[ERR_AIR_NOT_FOUND]",
    "[ERR_KERNEL_MSGQ_FAIL]",
    "[ERR_DUPLICATION]",
    "[ERR_TIMEOUT]",
    "[ERR_MSG_Q_NOT_EXIST]",
    "[ERR_UNKNOWN]"};

} // namespace air

#endif // AIR_CLI_LIB_H
