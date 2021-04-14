
#ifndef AIR_OUTPUT_MANAGER_H
#define AIR_OUTPUT_MANAGER_H

#include <sys/file.h>
#include <unistd.h>

#include <queue>
#include <string>

#include "src/lib/Msg.h"
#include "src/output/Out.h"

namespace output
{
class OutputManager
{
public:
    explicit OutputManager(OutCommand* new_out_command)
    : out_command(new_out_command)
    {
    }
    ~OutputManager(void)
    {
    }
    void
    EnqueueMsg(uint32_t type1, uint32_t type2, uint32_t value1,
        uint32_t value2, int pid, int cmd_type, int cmd_order)
    {
        msg.push({type1, type2, value1, value2, pid, cmd_type, cmd_order});
    }
    int HandleMsg(void);

private:
    int _SendCompletionMsg(uint32_t value1, int pid, int cmd_type, int cmd_order);
    std::string _TimeStampToHReadble(int64_t timestamp);
    std::queue<lib::MsgEntry> msg;
    OutCommand* out_command{nullptr};
};

} // namespace output

#endif // AIR_OUTPUT_MANAGER_H
