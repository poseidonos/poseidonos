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

#pragma once

#include <sched.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "rte_config.h"
#include "src/include/core_const.h"

namespace pos
{
enum class CoreType
{
    REACTOR = 0,
    UDD_IO_WORKER,
    EVENT_SCHEDULER,
    EVENT_WORKER,
    GENERAL_USAGE,
    QOS,
    META_SCHEDULER,
    META_IO,
    AIR,
    EVENT_REACTOR,
    DEBUG,
    COUNT,
};

static const uint32_t MAX_NUMA_COUNT = RTE_MAX_NUMA_NODES;

using CpuSetArray =
    std::array<cpu_set_t, static_cast<uint32_t>(CoreType::COUNT)>;
using CoreCountArray = std::array<uint32_t, MAX_NUMA_COUNT>;

struct CoreDescription
{
    CoreType type;
    CoreCountArray countArray;
    std::string coreRange;
};

using CoreDescriptionArray =
    std::array<CoreDescription, static_cast<uint32_t>(CoreType::COUNT)>;

class CpuSetGenerator
{
public:
    CpuSetGenerator(void);
    virtual ~CpuSetGenerator(void);

    const CpuSetArray& GetCpuSetArray(void);

protected:
    void _AddCoreSet(uint32_t startCore, uint32_t coreCount, uint32_t* nextCore,
        CoreType type);
    void _SetCpuSet(CoreType coreType, cpu_set_t cpuSet);
    uint32_t _GetFirstCore(uint32_t numa);

    const uint32_t NUMA_COUNT;
    const uint32_t TOTAL_CORE_COUNT;

private:
    void _SetNumaInformation(void);
    uint32_t _GetNextCpuFromThisNuma(uint32_t cpu);
    cpu_set_t _CalculateCoreSet(uint32_t startCore, uint32_t coreCount,
        uint32_t* nextCore);
    cpu_set_t _GetCpuSet(CoreType coreType);

    CpuSetArray cpuSetArray;
    uint32_t firstCore[MAX_NUMA_COUNT];
};

} // namespace pos
