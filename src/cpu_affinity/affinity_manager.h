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

#pragma once

#include <string>

#include "cpu_set_generator.h"
#include "rte_config.h"
#include "src/lib/singleton.h"

namespace pos
{
class AffinityManager
{
public:
    AffinityManager(void);
    ~AffinityManager(void);

    void SetGeneralAffinitySelf(void);
    cpu_set_t GetCpuSet(CoreType type);
    std::string GetReactorCPUSetString(void);
    uint32_t GetMasterReactorCore(void);
    uint32_t GetEventWorkerSocket(void);
    uint32_t GetTotalCore(void);
    uint32_t GetCoreCount(CoreType type);

private:
    static const uint32_t MAX_NUMA_COUNT = RTE_MAX_NUMA_NODES;
    static const bool PROHIBIT_CORE_MASK_OVERLAPPED;

    uint32_t totalNumaSystemCoreCount[MAX_NUMA_COUNT];
    uint32_t totalNumaDescriptedCoreCount[MAX_NUMA_COUNT];
    const uint32_t NUMA_COUNT;
    const uint32_t TOTAL_COUNT;
    CpuSetArray cpuSetArray;
    bool useStringForParsing;

    void _SetNumaInformation(const CoreDescriptionArray& descArray);
    bool _IsCoreSufficient(void);
    std::string _GetCPUSetString(cpu_set_t cpuSet);
};

using AffinityManagerSingleton = Singleton<AffinityManager>;

} // namespace pos
