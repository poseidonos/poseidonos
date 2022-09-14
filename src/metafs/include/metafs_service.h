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

/* 
 * PoseidonOS - Meta Filesystem Layer
 * 
 * Meta Filesystem Manager (metaFsMgr)
 */

// A Meta Filesystem Layer instance accessible by upper modules
#pragma once

#include <numa.h>

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "mk/ibof_config.h"
#include "src/lib/singleton.h"
#include "src/metafs/metafs.h"

namespace pos
{
class MetaFsIoScheduler;
class MetaFsConfigManager;
class TelemetryPublisher;
class MetaFsIoSchedulerFactory;

using SchedulerMap = std::unordered_map<uint32_t, MetaFsIoScheduler*>;

class MetaFsService
{
public:
    MetaFsService(void);
    MetaFsService(MetaFsConfigManager* configManager, MetaFsIoSchedulerFactory* factory, const int numaCount = numa_num_configured_nodes());
    virtual ~MetaFsService(void);
    virtual void Initialize(const uint32_t totalCoreCount, const cpu_set_t schedSet,
        const cpu_set_t workSet, TelemetryPublisher* tp = nullptr);
    virtual void Register(const std::string& arrayName, const int arrayId, MetaFs* fileSystem);
    virtual void Deregister(const std::string& arrayName);
    virtual MetaFs* GetMetaFs(const std::string& arrayName) const;
    virtual MetaFs* GetMetaFs(int arrayId) const;
    virtual SchedulerMap GetScheduler(void) const
    {
        return ioScheduler_;
    }
    virtual MetaFsConfigManager* GetConfigManager(void) const
    {
        return configManager_;
    }

private:
    void _CreateScheduler(const uint32_t totalCount, const cpu_set_t schedSet,
        const cpu_set_t workSet);
    uint32_t _GetNumaIdConsideringNumaDedicatedScheduling(const uint32_t numaId) const;
    bool _CheckIfPossibleToCreateScheduler(const int numOfSchedulersCreated);
    bool _CheckSchedulerSettingFromConfig(const int countOfScheduler) const;
    virtual uint32_t _GetNumaId(const uint32_t coreId)
    {
        return numa_node_of_cpu(coreId);
    }

    const int MAX_SCHEDULER_COUNT;
    std::unordered_map<std::string, int> arrayNameToId_;
    std::array<MetaFs*, MetaFsConfig::MAX_ARRAY_CNT> fileSystems_;
    SchedulerMap ioScheduler_;
    MetaFsConfigManager* configManager_;
    TelemetryPublisher* tp_;
    MetaFsIoSchedulerFactory* factory_;
};

using MetaFsServiceSingleton = Singleton<MetaFsService>;

} // namespace pos
