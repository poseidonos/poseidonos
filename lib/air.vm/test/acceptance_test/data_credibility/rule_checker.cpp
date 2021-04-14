
#include "rule_checker.h"

#include <string.h>
#include <stdlib.h>
#include <iostream>

void
RuleChecker::PrintHelp()
{
    std::cout << "\n [Usage]: ./data_cred size:512/4k/8k/32k(default:4k) wl:sw/sr/rw/rr/rw30(default:rr) sleep:0/1us/10us/100us/1ms/10ms/100ms(default:1us) runtime:60(default:60s, <=60s) error_margin:3(default:3%, <=10%), confidence_interval:95(default:0%, <=100%), output:filename_##\n";
}

bool
RuleChecker::CheckRule(int argc, char **argv)
{
    bool result {true};

    for (int i = 1; i < argc; i++)
    {
        if (false == CheckSize(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }
        
        if (false == CheckWorkload(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }

        if (false == CheckSleep(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }

        if (false == CheckRuntime(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }

        if (false == CheckErrorMargin(argv[i]))
        {
            PrintHelp();
            result = false;
            break;
        }

        if (false == CheckConfidenceInterval(argv[i]))
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
RuleChecker::CheckSize(char *arg)
{
    char* ptr = strstr(arg, "size:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            if (0 == strcmp(result, "512"))
            {
                chunk_size = 512;
            }
            else if (0 == strcmp(result, "4k"))
            {
                chunk_size = 4096;
            }
            else if (0 == strcmp(result, "8k"))
            {
                chunk_size = 8192;
            }
            else if (0 == strcmp(result, "32k"))
            {
                chunk_size = 32768;
            }
            else
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
RuleChecker::CheckWorkload(char *arg)
{
    char* ptr = strstr(arg, "wl:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            if (0 == strcmp(result, "sw"))
            {
                mixed_ratio = 0;
                sequential_workload = true;
                write_pattern = true;
            }
            else if (0 == strcmp(result, "sr"))
            {
                mixed_ratio = 0;
                sequential_workload = true;
                write_pattern = false;
            }
            else if (0 == strcmp(result, "rw"))
            {
                mixed_ratio = 0;
                sequential_workload = false;
                write_pattern = true;
            }
            else if (0 == strcmp(result, "rr"))
            {
                mixed_ratio = 0;
                sequential_workload = false;
                write_pattern = false;
            }
            else if (0 == strcmp(result, "rw30"))
            {
                mixed_ratio = 30;
                sequential_workload = false;
                write_pattern = true;
            }
            else
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
RuleChecker::CheckSleep(char *arg)
{
    char* ptr = strstr(arg, "sleep:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            if (0 == strcmp(result, "0"))
            {
                sleep_cycle = 0;
            }
            else if (0 == strcmp(result, "1us"))
            {
                sleep_cycle = 1000;
            }
            else if (0 == strcmp(result, "10us"))
            {
                sleep_cycle = 10000;
            }
            else if (0 == strcmp(result, "100us"))
            {
                sleep_cycle = 100000;
            }
            else if (0 == strcmp(result, "1ms"))
            {
                sleep_cycle = 1000000;
            }
            else if (0 == strcmp(result, "10ms"))
            {
                sleep_cycle = 10000000;
            }
            else if (0 == strcmp(result, "100ms"))
            {
                sleep_cycle = 100000000;
            }
            else
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
RuleChecker::CheckErrorMargin(char *arg)
{
    char* ptr = strstr(arg, "error_margin:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            error_margin = (unsigned int)atoi(result);

            if (10 < error_margin || 0 == error_margin)
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
RuleChecker::CheckConfidenceInterval(char *arg)
{
    char* ptr = strstr(arg, "confidence_interval:");

    if (NULL != ptr)
    {
        char* result = strtok(ptr, ":");
        if (NULL != result)
        {
            result = strtok(NULL, ":");
            
            confidence_interval = (unsigned int)atoi(result);

            if (100 < confidence_interval)
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
