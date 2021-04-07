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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "thread.h"

#include <unistd.h>

#include <thread>

namespace ibofos
{
int
Thread::SetAffinity(int coreId)
{
    int numCores = sysconf(_SC_NPROCESSORS_ONLN);
    if (coreId < 0 || coreId >= numCores)
    {
        return -1;
    }

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    return Thread::SetAffinity(cpuset);
}

int
Thread::SetAffinity(std::vector<int> coreIds)
{
    int numCores = sysconf(_SC_NPROCESSORS_ONLN);
    for (auto& v : coreIds)
    {
        if (v < 0 || v >= numCores)
        {
            return -1;
        }
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (auto& v : coreIds)
    {
        CPU_SET(v, &cpuset);
    }
    return Thread::SetAffinity(cpuset);
}

int
Thread::SetAffinity(cpu_set_t cpuset)
{
    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

int
Thread::GetAffinity(cpu_set_t* cpuset)
{
    return pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), cpuset);
}

int
Thread::SetName(std::string name)
{
    return pthread_setname_np(pthread_self(), name.c_str());
}

std::string
Thread::GetName(void)
{
    char name[16] = {
        0,
    };
    pthread_getname_np(pthread_self(), name, 16);
    return name;
}

} // namespace ibofos
