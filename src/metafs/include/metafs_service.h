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

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "mk/ibof_config.h"
#include "src/lib/singleton.h"
#include "src/metafs/include/meta_storage_info.h"
#include "src/metafs/include/metafs_return_code.h"
#include "src/metafs/metafs.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
class ScalableMetaIoWorker;
class MetaFsIoScheduler;
class MetaFsConfigManager;

class MetaFsService
{
public:
    MetaFsService(void);
    explicit MetaFsService(MetaFsConfigManager* configManager);
    ~MetaFsService(void);
    void Initialize(const uint32_t totalCount, const cpu_set_t schedSet,
        const cpu_set_t workSet, TelemetryPublisher* tp = nullptr);
    void Register(std::string& arrayName, int arrayId, MetaFs* fileSystem);
    void Deregister(std::string& arrayName);
    MetaFs* GetMetaFs(std::string& arrayName) const;
    MetaFs* GetMetaFs(int arrayId) const;
    MetaFsIoScheduler* GetScheduler(void) const
    {
        return ioScheduler;
    }
    MetaFsConfigManager* GetConfigManager(void) const
    {
        return configManager;
    }

private:
    void _PrepareThreads(const uint32_t totalCount, const cpu_set_t schedSet,
        const cpu_set_t workSet, TelemetryPublisher* tp);
    ScalableMetaIoWorker* _InitiateMioHandler(const int handlerId, const int coreId,
        const int coreCount, TelemetryPublisher* tp);

    std::unordered_map<std::string, int> arrayNameToId;
    std::array<MetaFs*, MetaFsConfig::MAX_ARRAY_CNT> fileSystems;
    MetaFsIoScheduler* ioScheduler;
    MetaFsConfigManager* configManager;
};

using MetaFsServiceSingleton = Singleton<MetaFsService>;

} // namespace pos
