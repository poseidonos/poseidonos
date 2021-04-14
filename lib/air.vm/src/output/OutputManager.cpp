
#include "src/output/OutputManager.h"

#include <string.h>
#include <time.h>

int
output::OutputManager::HandleMsg(void)
{
    int num_send = 0;
    bool retry = false;
    int num_try = 0;
    while (!msg.empty())
    {
        num_try++;
        lib::MsgEntry entry = msg.front();

        if (0 == _SendCompletionMsg(entry.value1, entry.pid, entry.cmd_type, entry.cmd_order))
        {
            msg.pop();
            num_send++;

            if (true == retry)
            {
                retry = false;
            }
        }
        else
        {
            if (false == retry)
            {
                retry = true;
            }
            else
            {
                retry = false;
                msg.pop();
            }
        }
    }

    return num_send;
}

int
output::OutputManager::_SendCompletionMsg(uint32_t value1, int pid,
    int cmd_type, int cmd_order)
{
    return out_command->Send(pid, static_cast<int>(value1) * -1, cmd_type, cmd_order);
}

std::string
output::OutputManager::_TimeStampToHReadble(int64_t timestamp)
{
    const time_t rawtime = (const time_t)timestamp;

    struct tm dt;
    char timestr[30];
    char buffer[30];

    if (NULL == localtime_r(&rawtime, &dt))
    {
        return "";
    }

    strftime(timestr, sizeof(timestr), "%y-%m-%d-%H:%M:%S", &dt);
    snprintf(buffer, sizeof(buffer), "%s", timestr);
    std::string stdBuffer(buffer);

    return stdBuffer;
}
