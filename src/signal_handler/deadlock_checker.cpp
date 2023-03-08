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

#include "deadlock_checker.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/include/branch_prediction.h"

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace pos
{

thread_local bool DeadLockChecker::allocated = false;
thread_local uint32_t DeadLockChecker::allocatedId = 0;

DeadLockChecker::DeadLockChecker(void)
{
    timeoutMilliSec = DEFAULT_TIMEOUT_MILLISECOND;
    running = false;
    for (uint32_t index = 0; index < DeadLockChecker::MAX_HEARTBIT_COUNT; index++)
    {
        std::lock_guard<std::mutex> lock(freeListMutex);
        freeList.push(index);
        threadInfo[index].prevHeartBeatCount = 0;
        threadInfo[index].heartBeatCount = 0;
        threadInfo[index].tid = 0;
        threadInfo[index].alreadyHappened = false;
    }
    deadLockChecker = nullptr;
}

void
DeadLockChecker::RunDeadLockChecker(void)
{
    if (running == false)
    {
        running = true;
        deadLockChecker = new std::thread(&DeadLockChecker::Run, this);
    }
}

void
DeadLockChecker::RegisterOnceAndHeartBeat(void)
{
    if (unlikely(allocated == false))
    {
        allocatedId = AllocHeartBeatCount();
        allocated = true;
    }
    IncreaseHeartBeatCount(allocatedId);
}

void
DeadLockChecker::DeRegister(void)
{
    if (allocated == true)
    {
        FreeHeartBeatCount(allocatedId);
        allocated = false;
    }
}

void
DeadLockChecker::SetTimeout(uint32_t milisecond)
{
    timeoutMilliSec = milisecond;
}

uint32_t
DeadLockChecker::AllocHeartBeatCount(void)
{
    std::lock_guard<std::mutex> lock(freeListMutex);
    uint32_t ret = freeList.front();
    threadInfo[ret].prevHeartBeatCount = 0;
    threadInfo[ret].heartBeatCount = 0;
    threadInfo[ret].tid = pthread_self();
    threadInfo[ret].alreadyHappened = false;
    freeList.pop();
    return ret;
}

// Not necessary if pool is static
void
DeadLockChecker::FreeHeartBeatCount(uint32_t index)
{
    std::lock_guard<std::mutex> lock(freeListMutex);
    threadInfo[index].tid = 0;
    freeList.push(index);
}

void
DeadLockChecker::IncreaseHeartBeatCount(uint32_t index)
{
    threadInfo[index].heartBeatCount++;
}

void
DeadLockChecker::Run(void)
{
    cpu_set_t cpuSet = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::DEBUG);
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    pthread_setname_np(pthread_self(), "DeadLockChecker");
    while(running == true)
    {
        for (uint32_t index = 0; index < DeadLockChecker::MAX_HEARTBIT_COUNT; index++)
        {
            // check only allocated thread
            if (threadInfo[index].tid != 0 && threadInfo[index].alreadyHappened == false)
            {
                // Hang State
                if (threadInfo[index].prevHeartBeatCount == threadInfo[index].heartBeatCount)
                {
                    threadInfo[index].alreadyHappened = true;
                    pthread_kill(threadInfo[index].tid, SIGUSR1);
                }
                else
                {
                    threadInfo[index].alreadyHappened = false;
                }
                threadInfo[index].prevHeartBeatCount = threadInfo[index].heartBeatCount;
            }
        }
        usleep(timeoutMilliSec * 1000); // default 10 sec
    }
}

DeadLockChecker::~DeadLockChecker(void)
{
    running = false;
    if (deadLockChecker != nullptr)
    {
        deadLockChecker->join();
        delete deadLockChecker;
    }
}
} // namespace pos
