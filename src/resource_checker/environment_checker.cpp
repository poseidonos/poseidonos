/*
 *   BSD LICENSE
 *   Copyright (c) 20` Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "environment_checker.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sstream>
#include <vector>

using namespace std;

namespace pos
{
const char EnvironmentChecker::SUPPORTED_OS_VERSION[20] = "Ubuntu 18.04";
const char EnvironmentChecker::SUPPORTED_KERNEL_VERSION[20] = "5.3.0-24-generic";
const char EnvironmentChecker::RULES_FILE_NAME[30] = "99-custom-nvme.rules";

const uint32_t EnvironmentChecker::MINIMUM_NUM_CORE = 4;
const uint32_t EnvironmentChecker::MINIMUM_MEMORY_SIZE_IN_KBYTE = 1000 * 1024;

EnvironmentChecker::EnvironmentChecker(void)
{
}

EnvironmentChecker::~EnvironmentChecker(void)
{
}

vector<string>
EnvironmentChecker::_GetcpuInfo(void)
{
    const string cmd = "cat /proc/cpuinfo -n";
    string cpuInfo;
    _GetSystemMsg_string(&cpuInfo, &cmd[0]);

    stringstream ss(cpuInfo);
    vector<string> systemMsg;
    string tmp;

    while (getline(ss, tmp, '\n'))
    {
        systemMsg.push_back(tmp);
    }

    return systemMsg;
}

vector<string>
EnvironmentChecker::_GetmemInfo(void)
{
    const string cmd = "cat /proc/meminfo";
    string memInfo;
    _GetSystemMsg_string(&memInfo, &cmd[0]);

    stringstream ss(memInfo);
    vector<string> systemMsg;
    string tmp;

    while (getline(ss, tmp, '\n'))
    {
        systemMsg.push_back(tmp);
    }

    return systemMsg;
}

uint32_t
EnvironmentChecker::GetNvmeDeviceCnt(void)
{
    const string cmd = "lspci -v";
    string pcieInfo;
    _GetSystemMsg_string(&pcieInfo, &cmd[0]);

    stringstream ss(pcieInfo);
    vector<string> systemMsg;
    string tmp;
    uint32_t nvmePcieCnt = 0;
    while (getline(ss, tmp, '\n'))
    {
        size_t found = tmp.find("Non-Volatile memory controller");
        if (found != string::npos)
        {
            systemMsg.push_back(tmp);
            nvmePcieCnt++;
        }

        found = tmp.find("nvme");
        if (found != string::npos)
        {
            systemMsg.push_back(tmp);
        }
    }

    return nvmePcieCnt;
}

bool
EnvironmentChecker::CheckSupportedOsVersion(void)
{
    bool isMatchedwithPOS = false;
    const string cmd = "cat /etc/issue";
    string userVersion;

    _GetSystemMsg_string(&userVersion, &cmd[0]);

    size_t found = userVersion.find(SUPPORTED_OS_VERSION);
    if (found != string::npos)
    {
        isMatchedwithPOS = true;
    }

    return isMatchedwithPOS;
}

bool
EnvironmentChecker::CheckSupportedKernelVersion(void)
{
    bool isMatchedwithPOS = false;
    const string cmd = "hostnamectl | grep Kernel";
    string userVersion;

    _GetSystemMsg_string(&userVersion, &cmd[0]);

    size_t found = userVersion.find(SUPPORTED_KERNEL_VERSION);
    if (found != string::npos)
    {
        isMatchedwithPOS = true;
    }

    return isMatchedwithPOS;
}

void
EnvironmentChecker::_GetSystemMsg_string(string* outMsg, const char* cmd)
{
    char buf[1024];
    string str;
    FILE* fp;
    if ((fp = popen(cmd, "r")) == NULL)
    {
        return;
    }

    while (fgets(buf, 1024, fp) != NULL)
    {
        outMsg->append(buf);
    }

    pclose(fp);
}

uint32_t
EnvironmentChecker::GetCpuNum(void)
{
    vector<string> cpuInfo = _GetcpuInfo();
    uint32_t cpuNum = 0;

    for (uint32_t i = 0; i < cpuInfo.size(); i++)
    {
        stringstream ss(cpuInfo[i]);
        string tmp;
        while (getline(ss, tmp, ':'))
        {
            size_t found = tmp.find("processor");
            if (found != string::npos)
            {
                cpuNum++;
            }
        }
    }

    return cpuNum;
}

uint32_t
EnvironmentChecker::GetCpuClock(void)
{
    vector<string> cpuInfo = _GetcpuInfo();
    uint32_t cpuClock = 0;

    for (uint32_t i = 0; i < cpuInfo.size(); i++)
    {
        size_t found = cpuInfo[i].find("cpu MHz");
        if (found != string::npos)
        {
            stringstream ss(cpuInfo[i]);
            string tmp = "";
            while (getline(ss, tmp, ' '))
            {
                if (isdigit(tmp[0]) != 0)
                    cpuClock = stoi(tmp);
            }

            return cpuClock;
        }
    }

    return cpuClock;
}

uint32_t
EnvironmentChecker::GetAvailableMemorySize(void)
{
    vector<string> memInfo = _GetmemInfo();
    uint32_t memSize = 0;

    for (uint32_t i = 0; i < memInfo.size(); i++)
    {
        size_t found = memInfo[i].find("MemAvailable");
        if (found != string::npos)
        {
            stringstream ss(memInfo[i]);
            string tmp = "";
            while (getline(ss, tmp, ' '))
            {
                if (isdigit(tmp[0]) != 0)
                {
                    memSize = stoi(tmp);
                    break;
                }
            }
            break;
        }
    }

    return memSize;
}

uint32_t
EnvironmentChecker::GetTotalMemorySize(void)
{
    vector<string> memInfo = _GetmemInfo();
    uint32_t memSize = 0;

    for (uint32_t i = 0; i < memInfo.size(); i++)
    {
        size_t found = memInfo[i].find("MemTotal");
        if (found != string::npos)
        {
            stringstream ss(memInfo[i]);
            string tmp = "";
            while (getline(ss, tmp, ' '))
            {
                if (isdigit(tmp[0]) != 0)
                {
                    memSize = stoi(tmp);
                    break;
                }
            }
            return memSize;
        }
    }

    return memSize;
}

bool
EnvironmentChecker::IsMeetMinimumCore(void)
{
    uint32_t hostCpuNum = GetCpuNum();
    return hostCpuNum >= MINIMUM_NUM_CORE;
}

bool
EnvironmentChecker::IsMeetMinimumMemory(void)
{
    uint32_t hostAvailableMemorySize = GetAvailableMemorySize();
    bool isSuccess = false;
    if (hostAvailableMemorySize >= MINIMUM_MEMORY_SIZE_IN_KBYTE)
    {
        isSuccess = true;
    }

    return isSuccess;
}

bool
EnvironmentChecker::CheckRulesFile(void)
{
    bool isSuccess = false;
    char* tmp = getcwd(NULL, 256);
    if (tmp == nullptr)
    {
        return false;
    }

    string directory = tmp;
    directory.append("/../tool/udev/");
    string filename = RULES_FILE_NAME;
    string targetFile = directory + filename;

    if (access(targetFile.c_str(), F_OK) == 0)
    {
        isSuccess = true;
    }

    return isSuccess;
}
} // namespace pos

extern "C"
{
    bool IsMeetMinimumCore(void)
    {
        return pos::EnvironmentChecker::IsMeetMinimumCore();
    }

    uint32_t GetHostCpuNum(void)
    {
        return pos::EnvironmentChecker::GetCpuNum();
    }

    uint32_t GetHostCpuClock(void)
    {
        return pos::EnvironmentChecker::GetCpuClock();
    }

    bool IsMeetMinimumMemorySize(void)
    {
        return pos::EnvironmentChecker::IsMeetMinimumMemory();
    }

    uint32_t GetTotalMemorySizeInfo(void)
    {
        return pos::EnvironmentChecker::GetTotalMemorySize();
    }

    uint32_t GetAvailableMemorySizeInfo(void)
    {
        return pos::EnvironmentChecker::GetAvailableMemorySize();
    }

    uint32_t PcieCountInfo(void)
    {
        return pos::EnvironmentChecker::GetNvmeDeviceCnt();
    }

    bool CheckSupportedOsVersion(void)
    {
        return pos::EnvironmentChecker::CheckSupportedOsVersion();
    }

    bool CheckSupportedKernelVersion(void)
    {
        return pos::EnvironmentChecker::CheckSupportedKernelVersion();
    }

    bool CheckRulesdFile(void)
    {
        return pos::EnvironmentChecker::CheckRulesFile();
    }
}
