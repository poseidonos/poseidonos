
#include "rule_checker.h"

#include <string.h>
#include <stdlib.h>
#include <iostream>

void
RuleChecker::PrintHelp()
{
    std::cout << "\n [Usage]: ./data_stream runtime:60(default:60s, <=60s) output:filename_##\n";
}

bool
RuleChecker::CheckRule(int argc, char **argv)
{
    bool result {true};

    for (int i = 1; i < argc; i++)
    {
        if (false == CheckRuntime(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }

        if (false == CheckFilename(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }
    }

    return result;
}

bool
RuleChecker::CheckRuntime(char *arg)
{
    char* ptr = strstr(arg, "runtime:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            run_time = (unsigned int)atoi(result);

            if (60 < run_time || 0 == run_time)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool
RuleChecker::CheckFilename(char *arg)
{
    char* ptr = strstr(arg, "output:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            filename = result;
        }
        else
        {
            return false;
        }
    }

    return true;
}
