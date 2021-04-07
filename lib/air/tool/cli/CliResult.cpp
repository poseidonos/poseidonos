
#include "tool/cli/CliResult.h"

#include <stdio.h>

#include <cstring>
#include <iostream>

air::CliResult::~CliResult(void)
{
    result_q.clear();
}

void
air::CliResult::SetReturn(ReturnCode ret_code, std::string desc)
{
    result_q.push_back({ret_code, desc});
}

void
air::CliResult::PrintCliResult(void)
{
    for (auto i : result_q)
    {
        std::cout << ReturnMsg[i.ret_code] << std::endl;
        std::cout << i.desc << std::endl;
    }
}
