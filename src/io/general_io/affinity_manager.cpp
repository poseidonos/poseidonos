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

#include "src/io/general_io/affinity_manager.h"

#include <numa.h>

#include <iomanip>

#include "affinity_config_parser.h"
#include "count_descripted_cpu_set_generator.h"
#include "poverty_cpu_set_generator.h"
#include "src/device/event_framework_api.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "string_descripted_cpu_set_generator.h"

namespace ibofos
{
const bool AffinityManager::PROHIBIT_CORE_MASK_OVERLAPPED = false;

AffinityManager::AffinityManager(void)
: totalNumaSystemCoreCount{
      0,
  },
  totalNumaDescriptedCoreCount{
      0,
  },
  NUMA_COUNT(numa_num_configured_nodes()),
  TOTAL_COUNT(numa_num_configured_cpus())
{
    assert(NUMA_COUNT <= MAX_NUMA_COUNT);

    AffinityConfigParser parser;
    const CoreDescriptionArray DESC_ARRAY = parser.GetDescriptions();
    useStringForParsing = parser.IsStringDescripted();

    _SetNumaInformation(DESC_ARRAY);

    try
    {
        if (useStringForParsing)
        {
            StringDescriptedCpuSetGenerator cpuSetGenerator(
                DESC_ARRAY, PROHIBIT_CORE_MASK_OVERLAPPED);
            cpuSetArray = cpuSetGenerator.GetCpuSetArray();

            return;
        }

        bool isCoreSufficient = _IsCoreSufficient();

        if (isCoreSufficient)
        {
            CountDescriptedCpuSetGenerator cpuSetGenerator(DESC_ARRAY);
            cpuSetArray = cpuSetGenerator.GetCpuSetArray();
            return;
        }
    }
    catch (...)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_FAIL_TO_PARSING_ERROR;
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
    }

    PovertyCpuSetGenerator cpuSetGenerator(DESC_ARRAY);
    cpuSetArray = cpuSetGenerator.GetCpuSetArray();
}

AffinityManager::~AffinityManager(void)
{
}

void
AffinityManager::_SetNumaInformation(const CoreDescriptionArray& descArray)
{
    for (uint32_t cpu = 0; cpu < TOTAL_COUNT; cpu++)
    {
        int32_t numa = numa_node_of_cpu(cpu);
        if (numa == INVALID_NUMA)
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_DISABLED_CORE;
            IBOF_TRACE_INFO(eventId, IbofEventId::GetString(eventId), cpu);
            continue;
        }

        totalNumaSystemCoreCount[numa]++;
    }

    for (auto& descIter : descArray)
    {
        uint32_t numa = 0;
        for (auto& countIter : descIter.countArray)
        {
            totalNumaDescriptedCoreCount[numa] += countIter;
            numa++;
        }
    }
}

bool
AffinityManager::_IsCoreSufficient(void)
{
    for (uint32_t numa = 0; numa < NUMA_COUNT; numa++)
    {
        uint32_t systemCoreCount = totalNumaSystemCoreCount[numa];
        uint32_t descriptedCoreCount = totalNumaDescriptedCoreCount[numa];
        if (systemCoreCount < descriptedCoreCount)
        {
            return false;
        }
    }

    return true;
}

uint32_t
AffinityManager::GetCoreCount(CoreType type)
{
    cpu_set_t cpuSet = GetCpuSet(type);
    return CPU_COUNT(&cpuSet);
}

uint32_t
AffinityManager::GetTotalCore(void)
{
    return TOTAL_COUNT;
}

uint32_t
AffinityManager::GetEventWorkerSocket(void)
{
    cpu_set_t eventCpuSet = GetCpuSet(CoreType::EVENT_WORKER);
    std::array<uint32_t, MAX_NUMA_COUNT> eventCoreCountNuma{};
    for (uint32_t cpu = 0; cpu < TOTAL_COUNT; cpu++)
    {
        if (CPU_ISSET(cpu, &eventCpuSet))
        {
            int32_t numa = numa_node_of_cpu(cpu);
            if (numa == INVALID_NUMA)
            {
                continue;
            }
            eventCoreCountNuma[numa]++;
        }
    }

    uint32_t maxEventCoreCount = 0;
    uint32_t maxEventCoreNuma = UINT32_MAX;
    uint32_t numa = 0;

    for (auto count : eventCoreCountNuma)
    {
        if (maxEventCoreCount < count)
        {
            maxEventCoreCount = count;
            maxEventCoreNuma = numa;
        }
        numa++;
    }

    if (unlikely(maxEventCoreNuma == UINT32_MAX))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_NO_EVENT_WORKER_ALLOCATED;
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
    }

    return maxEventCoreNuma;
}

std::string
AffinityManager::GetReactorCPUSetString(void)
{
    cpu_set_t reactorCpuSet = GetCpuSet(CoreType::REACTOR);
    string cpuString = _GetCPUSetString(reactorCpuSet);
    return cpuString;
}

std::string
AffinityManager::GetEventCPUSetString(void)
{
    cpu_set_t eventCpuSet = GetCpuSet(CoreType::EVENT_WORKER);
    string cpuString = _GetCPUSetString(eventCpuSet);
    return cpuString;
}

std::string
AffinityManager::_GetCPUSetString(cpu_set_t cpuSet)
{
    uint32_t totalCoreCount = GetTotalCore();
    uint32_t nibble = 0;
    const uint32_t NIBBLE_BIT_COUNT = 4;
    std::ostringstream cpuString;
    for (uint32_t cpu = 0; cpu < totalCoreCount; cpu++)
    {
        if (CPU_ISSET(cpu, &cpuSet))
        {
            nibble += 1UL << (cpu % NIBBLE_BIT_COUNT);
        }
        if (cpu % NIBBLE_BIT_COUNT == NIBBLE_BIT_COUNT - 1)
        {
            cpuString << std::hex << nibble;
            nibble = 0;
        }
    }
    cpuString << std::hex << nibble;
    string cpuSetString(cpuString.str());
    std::reverse(cpuSetString.begin(), cpuSetString.end());

    return cpuSetString;
}

uint32_t
AffinityManager::GetMasterReactorCore(void)
{
    cpu_set_t reactorCpuSet = GetCpuSet(CoreType::REACTOR);
    uint32_t totalCoreCount = GetTotalCore();
    for (uint32_t cpu = 0; cpu < totalCoreCount; cpu++)
    {
        if (CPU_ISSET(cpu, &reactorCpuSet))
        {
            return cpu;
        }
    }

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_FAIL_TO_FIND_MASTER_REACTOR;
    IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));

    return EventFrameworkApi::INVALID_CORE;
}

cpu_set_t
AffinityManager::GetCpuSet(CoreType type)
{
    return cpuSetArray[static_cast<uint32_t>(type)];
}

void
AffinityManager::SetGeneralAffinitySelf(void)
{
    cpu_set_t generalCPUSet = GetCpuSet(CoreType::GENERAL_USAGE);
    sched_setaffinity(0, sizeof(cpu_set_t), &generalCPUSet);
}

} // namespace ibofos
