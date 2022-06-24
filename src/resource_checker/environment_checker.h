/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
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

#ifndef ENVIRONMENT_CHECKER_H_
#define ENVIRONMENT_CHECKER_H_

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

namespace pos
{
class EnvironmentChecker
{
public:
    EnvironmentChecker(void);
    virtual ~EnvironmentChecker(void);

    static uint32_t GetCpuNum(void);
    static uint32_t GetCpuClock(void);
    static bool IsMeetMinimumCore(void);

    static uint32_t GetAvailableMemorySize(void);
    static uint32_t GetTotalMemorySize(void);
    static bool IsMeetMinimumMemory(void);

    static uint32_t GetNvmeDeviceCnt(void);
    static bool CheckSupportedOsVersion(void);
    static bool CheckSupportedKernelVersion(void);
    static bool CheckRulesFile(void);

private:
    static const char SUPPORTED_OS_VERSION[20];
    static const char SUPPORTED_KERNEL_VERSION[20];
    static const char RULES_FILE_NAME[30];
    static const uint32_t MINIMUM_NUM_CORE;
    static const uint32_t MINIMUM_MEMORY_SIZE_IN_KBYTE;

    static std::vector<string> _GetcpuInfo(void);
    static std::vector<string> _GetmemInfo(void);
    static void _GetSystemMsg_string(string* outMsg, const char* cmd);
};
} // namespace pos

#endif // ENVIRONMENT_CHECKER_H_
