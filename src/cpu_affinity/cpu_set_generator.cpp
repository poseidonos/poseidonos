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

#include "cpu_set_generator.h"

#include <numa.h>

#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos

{
CpuSetGenerator::CpuSetGenerator(void)
: NUMA_COUNT(numa_num_configured_nodes()),
  TOTAL_CORE_COUNT(numa_num_configured_cpus()),
  firstCore{
      0,
  }
{
    _SetNumaInformation();
    for (auto& iter : cpuSetArray)
    {
        CPU_ZERO(&iter);
    }
}

CpuSetGenerator::~CpuSetGenerator(void)
{
}

const CpuSetArray&
CpuSetGenerator::GetCpuSetArray(void)
{
    return cpuSetArray;
}

void
CpuSetGenerator::_SetNumaInformation(void)
{
    bool firstCoreFound[MAX_NUMA_COUNT] = {
        false,
    };

    for (uint32_t cpu = 0; cpu < TOTAL_CORE_COUNT; cpu++)
    {
        int32_t numa = numa_node_of_cpu(cpu);
        if (static_cast<uint32_t>(numa) == INVALID_NUMA)
        {
            continue;
        }

        if (firstCoreFound[numa] == false)
        {
            firstCore[numa] = cpu;
            firstCoreFound[numa] = true;
        }
    }
}

uint32_t
CpuSetGenerator::_GetNextCpuFromThisNuma(uint32_t cpu)
{
    int32_t numa = numa_node_of_cpu(cpu);
    if (static_cast<uint32_t>(numa) == INVALID_NUMA)
    {
        POS_EVENT_ID eventId = EID(AFTMGR_DISABLED_CORE);
        POS_TRACE_ERROR(eventId, "Core {} is disabled", cpu);
        return 0;
    }

    do
    {
        cpu++;
        if (cpu >= TOTAL_CORE_COUNT)
        {
            cpu = firstCore[numa];
        }
    } while (numa_node_of_cpu(cpu) != numa);

    return cpu;
}

uint32_t
CpuSetGenerator::_GetFirstCore(uint32_t numa)
{
    return firstCore[numa];
}

cpu_set_t
CpuSetGenerator::_CalculateCoreSet(uint32_t startCore, uint32_t coreCount,
    uint32_t* nextCore)
{
    *nextCore = startCore;
    cpu_set_t cpuSet;

    CPU_ZERO(&cpuSet);
    while (coreCount > 0)
    {
        CPU_SET(*nextCore, &cpuSet);
        *nextCore = _GetNextCpuFromThisNuma(*nextCore);
        coreCount--;
    }

    return cpuSet;
}

void
CpuSetGenerator::_AddCoreSet(uint32_t startCore, uint32_t coreCount,
    uint32_t* nextCore, CoreType type)
{
    cpu_set_t resultCpuSet;
    resultCpuSet = _CalculateCoreSet(startCore, coreCount, nextCore);

    cpu_set_t prevCpuSet = _GetCpuSet(type);
    CPU_OR(&resultCpuSet, &resultCpuSet, &prevCpuSet);
    _SetCpuSet(type, resultCpuSet);
}

void
CpuSetGenerator::_SetCpuSet(CoreType type, cpu_set_t cpuSet)
{
    cpuSetArray[static_cast<uint32_t>(type)] = cpuSet;
}

cpu_set_t
CpuSetGenerator::_GetCpuSet(CoreType type)
{
    return cpuSetArray[static_cast<uint32_t>(type)];
}

} // namespace pos
