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

#ifndef RESOURCE_CHECKER_H_
#define RESOURCE_CHECKER_H_

#include <string.h>

#include <thread>
#include <vector>

#include "src/lib/singleton.h"

namespace pos
{
class AffinityManager;
class TelemetryPublisher;
class TelemetryClient;
class EnvironmentChecker;

class ResourceChecker
{
public:
    ResourceChecker(void);
    virtual ~ResourceChecker(void);
    void Execute(void);
    uint64_t GetIterationCount(void);
    void SetSleepTime(uint32_t time);
    void Enable(void);

private:
    void CollectSmartLogPage(void);
    bool enable;
    uint64_t runningCnt;
    uint32_t sleepSecTime;
    std::thread* th = nullptr;
    AffinityManager* affinityManager = nullptr; // Get singletone object
    TelemetryClient* telemetryClient = nullptr; // Get singletone object
    TelemetryPublisher* publisher = nullptr;    // new
    EnvironmentChecker* envChecker = nullptr;   // new

    const int SLEEP_TIME_SEC = 60;
};
using ResourceCheckerSingleton = Singleton<ResourceChecker>;
} // namespace pos

#endif // RESOURCE_CHECKER_H_
