#pragma once

#include <string>
#include "tool/cli_client/command_list.h"

using namespace std;

class Request
{
public:
    string Make(int &argc, char *argv[]);
    string MountArray();
    string ListDevice();
    string SysState();
    string ExitSys();

private:
    CommandList cmdList;
};