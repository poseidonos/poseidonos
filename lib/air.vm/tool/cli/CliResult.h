
#ifndef AIR_CLI_RESULT_H
#define AIR_CLI_RESULT_H

#include <deque>
#include <string>

#include "tool/cli/CliLib.h"

namespace air
{
class CliResult
{
public:
    ~CliResult(void);
    void SetReturn(ReturnCode ret_code, std::string desc = "");
    void PrintCliResult(void);

private:
    void _PrintResultQ(void);

    struct CmdResult
    {
        ReturnCode ret_code;
        std::string desc;
    };

    std::deque<CmdResult> result_q;
};

} // namespace air

#endif // AIR_CLI_RESULT_H
