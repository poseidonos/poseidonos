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

#include <string>

#include "rte_config.h"
#include "src/cpu_affinity/affinity_config_parser.h"
#include "src/cpu_affinity/cpu_set_generator.h"
#include "src/lib/singleton.h"

namespace pos
{
class AffinityManager
{
public:
    AffinityManager(void);
    AffinityManager(uint32_t totalCount, CpuSetArray& cpuSetArray);
    explicit AffinityManager(AffinityConfigParser* parser_);
    virtual ~AffinityManager(void);

    void SetGeneralAffinitySelf(void);
    cpu_set_t GetCpuSet(CoreType type);
    std::string GetReactorCPUSetString(void);
    uint32_t GetMasterReactorCore(void);
    virtual uint32_t GetEventWorkerSocket(void);
    virtual uint32_t GetTotalCore(void);
    virtual uint32_t GetNumaIdFromCurrentThread(void);
    virtual uint32_t GetNumaIdFromCoreId(uint32_t coreId);
    uint32_t GetCoreCount(CoreType type);
    virtual uint32_t GetNumaCount(void);
    virtual bool UseEventReactor();
    virtual bool IsEventReactor(uint32_t reactor);
    virtual bool IsIoReactor(uint32_t reactor);

private:
    static const uint32_t MAX_NUMA_COUNT = RTE_MAX_NUMA_NODES;
    static const bool PROHIBIT_CORE_MASK_OVERLAPPED;

    uint32_t totalNumaSystemCoreCount[MAX_NUMA_COUNT];
    uint32_t totalNumaDescriptedCoreCount[MAX_NUMA_COUNT];
    const uint32_t NUMA_COUNT;
    const uint32_t TOTAL_COUNT;
    CpuSetArray cpuSetArray;
    bool useStringForParsing;
    AffinityConfigParser* parser;

    void _SetNumaInformation(const CoreDescriptionArray& descArray);
    bool _IsCoreSufficient(void);
    std::string _GetCPUSetString(cpu_set_t cpuSet);
    static thread_local uint32_t numaId;
};

using AffinityManagerSingleton = Singleton<AffinityManager>;

} // namespace pos
