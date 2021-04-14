
#ifndef AIR_MSG_H
#define AIR_MSG_H

#include <cstdint>

namespace lib
{
struct MsgEntry
{
    uint32_t type1{0};
    uint32_t type2{0};
    uint32_t value1{0};
    uint32_t value2{0};
    int pid{0};
    int cmd_type{0};
    int cmd_order{0};
};

enum class MsgType : int64_t
{
    FILE = 1,
    CLI,
    NULLMsgType
};

enum class AirStatus : uint32_t
{
    RUN,
    DONE,
    ABNORMAL_DONE,
    IDLE
};

struct FileMsg
{
    int64_t type{(int64_t)MsgType::FILE};
    char name[50]{
        0,
    };
    uint32_t air_pid{0};
    AirStatus status{AirStatus::IDLE};
    char time_stamp[30]{
        0,
    };
};

enum class CmdType : uint32_t
{
    SetStreamingInterval = 1,
    RunNode,
    InitNode,
    GetConfig,
    NullCliCmd
};

struct CliMsg
{
    uint32_t msg_id{0};
    CmdType cmd{CmdType::NullCliCmd};
    char node_list[256]{
        0,
    };
    uint32_t value{0};
    uint32_t air_pid{0};
};

} // namespace lib

#endif // AIR_MSG_H
