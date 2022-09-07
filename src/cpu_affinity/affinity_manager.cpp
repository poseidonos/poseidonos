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

#include "src/cpu_affinity/affinity_manager.h"

#include <numa.h>
#include <sched.h>

#include <iomanip>

#include "count_descripted_cpu_set_generator.h"
#include "poverty_cpu_set_generator.h"
#include "src/include/branch_prediction.h"
#include "src/include/core_const.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/qos/qos_common.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "string_descripted_cpu_set_generator.h"

namespace pos
{
const bool AffinityManager::PROHIBIT_CORE_MASK_OVERLAPPED = false;
thread_local uint32_t AffinityManager::numaId = INVALID_NUMA;

AffinityManager::AffinityManager(void)
: AffinityManager::AffinityManager(new AffinityConfigParser())
{
}

AffinityManager::AffinityManager(uint32_t totalCount, CpuSetArray& cpuSetArray)
: NUMA_COUNT(numa_num_configured_nodes()),
  TOTAL_COUNT(totalCount),
  cpuSetArray(cpuSetArray),
  useStringForParsing(false),
  parser(nullptr)
{
}

AffinityManager::AffinityManager(AffinityConfigParser* parser_)
: totalNumaSystemCoreCount{
      0,
  },
  totalNumaDescriptedCoreCount{
      0,
  },
  NUMA_COUNT(numa_num_configured_nodes()),
  TOTAL_COUNT(numa_num_configured_cpus()),
  parser(parser_)
{
    assert(NUMA_COUNT <= MAX_NUMA_COUNT);

    const CoreDescriptionArray DESC_ARRAY = parser->GetDescriptions();
    useStringForParsing = parser->IsStringDescripted();

    _SetNumaInformation(DESC_ARRAY);

    try
    {
        if (useStringForParsing)
        {
            StringDescriptedCpuSetGenerator cpuSetGenerator(
                DESC_ARRAY, PROHIBIT_CORE_MASK_OVERLAPPED);
            cpuSetArray = cpuSetGenerator.GetCpuSetArray();
        }
        else
        {
            bool isCoreSufficient = _IsCoreSufficient();
            if (isCoreSufficient)
            {
                CountDescriptedCpuSetGenerator cpuSetGenerator(DESC_ARRAY);
                cpuSetArray = cpuSetGenerator.GetCpuSetArray();
            }
            else
            {
                POS_EVENT_ID eventId = EID(AFTMGR_CORE_NOT_SUFFICIENT);
                POS_TRACE_ERROR(eventId, "Cpu is not sufficient");
                PovertyCpuSetGenerator cpuSetGenerator(DESC_ARRAY);
                cpuSetArray = cpuSetGenerator.GetCpuSetArray();
            }
        }

        if (parser->UseEventReactor())
        {
            SpdkNvmfCaller spdkNvmfCaller;
            spdkNvmfCaller.SpdkNvmfSetUseEventReactor(cpuSetArray[(static_cast<uint32_t>(CoreType::EVENT_REACTOR))]);
        }
    }
    catch (...)
    {
        POS_EVENT_ID eventId = EID(AFTMGR_FAIL_TO_PARSING_ERROR);
        POS_TRACE_ERROR(eventId, "Cpu allowed list is wrongly set");
        PovertyCpuSetGenerator cpuSetGenerator(DESC_ARRAY);
        cpuSetArray = cpuSetGenerator.GetCpuSetArray();
    }
}

AffinityManager::~AffinityManager(void)
{
    if (nullptr != parser)
    {
        delete parser;
        parser = nullptr;
    }
}

void
AffinityManager::_SetNumaInformation(const CoreDescriptionArray& descArray)
{
    for (uint32_t cpu = 0; cpu < TOTAL_COUNT; cpu++)
    {
        int32_t numa = numa_node_of_cpu(cpu);
        if (static_cast<uint32_t>(numa) == INVALID_NUMA)
        {
            POS_EVENT_ID eventId = EID(AFTMGR_DISABLED_CORE);
            POS_TRACE_INFO(eventId, "Core {} is disabled", cpu);
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
            if (static_cast<uint32_t>(numa) == INVALID_NUMA)
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
        POS_EVENT_ID eventId = EID(AFTMGR_NO_EVENT_WORKER_ALLOCATED);
        POS_TRACE_ERROR(eventId, "Cannot find event worker at any NUMA");
    }

    return maxEventCoreNuma;
}

std::string
AffinityManager::GetReactorCPUSetString(void)
{
    cpu_set_t reactorCpuSet = GetCpuSet(CoreType::REACTOR);
    cpu_set_t totalReactorCpuSet;
    CPU_ZERO(&totalReactorCpuSet);
    CPU_OR(&totalReactorCpuSet, &reactorCpuSet, &totalReactorCpuSet);
    if (true == UseEventReactor())
    {
        cpu_set_t eventReactorCpuSet = GetCpuSet(CoreType::EVENT_REACTOR);
        CPU_OR(&totalReactorCpuSet, &eventReactorCpuSet, &totalReactorCpuSet);
    }
    string cpuString = _GetCPUSetString(totalReactorCpuSet);
    return cpuString;
}

uint32_t
AffinityManager::GetNumaIdFromCurrentThread(void)
{
    if (unlikely(numaId == INVALID_NUMA))
    {
        numaId = numa_node_of_cpu(sched_getcpu());
    }
    return numaId;
}

uint32_t
AffinityManager::GetNumaIdFromCoreId(uint32_t coreId)
{
    numaId = numa_node_of_cpu(coreId);
    return numaId;
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

    POS_EVENT_ID eventId = EID(AFTMGR_FAIL_TO_FIND_MASTER_REACTOR);
    POS_TRACE_ERROR(eventId, "Fail to find master reactor");

    return INVALID_CORE;
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

uint32_t
AffinityManager::GetNumaCount(void)
{
    return NUMA_COUNT;
}

bool
AffinityManager::UseEventReactor(void)
{
    if (parser != nullptr)
    {
        return parser->UseEventReactor();
    }
    return false;
}

bool
AffinityManager::IsEventReactor(uint32_t reactor)
{
    cpu_set_t eventReactorCpuSet = GetCpuSet(CoreType::EVENT_REACTOR);
    if (CPU_ISSET(reactor, &eventReactorCpuSet))
    {
        return true;
    }
    return false;
}

bool
AffinityManager::IsIoReactor(uint32_t reactor)
{
    cpu_set_t ioReactorCpuSet = GetCpuSet(CoreType::REACTOR);
    if (CPU_ISSET(reactor, &ioReactorCpuSet))
    {
        return true;
    }
    return false;
}

} // namespace pos
