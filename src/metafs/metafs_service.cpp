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

#include "metafs_service.h"

#include <string>
#include <unordered_map>

#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/mim/metafs_io_scheduler.h"

namespace pos
{
MetaFsService::MetaFsService(void)
: MetaFsService(nullptr, new MetaFsConfigManager(ConfigManagerSingleton::Instance()))
{
    needToRemoveConfig_ = true;
}

MetaFsService::MetaFsService(MetaFsIoScheduler* ioScheduler, MetaFsConfigManager* configManager)
: ioScheduler_(ioScheduler),
  configManager_(configManager),
  needToRemoveConfig_(false)
{
    fileSystems_.fill(nullptr);
}

MetaFsService::~MetaFsService(void)
{
    if (ioScheduler_)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "MetaScheduler is suspended.");

        ioScheduler_->ExitThread();

        delete ioScheduler_;
        ioScheduler_ = nullptr;
    }

    if (needToRemoveConfig_)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Delete MetaFsConfigManager");

        delete configManager_;
        configManager_ = nullptr;
    }
}

void
MetaFsService::Initialize(const uint32_t totalCoreCount, const cpu_set_t schedSet,
    const cpu_set_t mioSet, TelemetryPublisher* tp)
{
    if (!configManager_->Init())
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MFS_INVALID_CONFIG),
            "The config values are invalid.");
        assert(false);
    }

    _CreateScheduler(totalCoreCount, schedSet, mioSet, tp);
}

void
MetaFsService::Register(const std::string& arrayName, const int arrayId, MetaFs* fileSystem)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "New metafs instance registered. arrayId: {}, arrayName: {}",
        arrayId, arrayName);

    arrayNameToId_.insert(std::pair<std::string, int>(arrayName, arrayId));
    fileSystems_[arrayId] = fileSystem;
}

void
MetaFsService::Deregister(const std::string& arrayName)
{
    const int arrayId = arrayNameToId_[arrayName];

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
        "A metafs instance deregistered. arrayName: {}",
        arrayName);

    arrayNameToId_.erase(arrayName);
    fileSystems_[arrayId] = nullptr;
}

MetaFs*
MetaFsService::GetMetaFs(const std::string& arrayName) const
{
    auto iter = arrayNameToId_.find(arrayName);
    if (iter == arrayNameToId_.end())
    {
        return nullptr;
    }
    else
    {
        return fileSystems_[iter->second];
    }
}

MetaFs*
MetaFsService::GetMetaFs(const int arrayId) const
{
    return fileSystems_[arrayId];
}

void
MetaFsService::_CreateScheduler(const uint32_t totalCoreCount,
    const cpu_set_t schedSet, const cpu_set_t mioSet, TelemetryPublisher* tp)
{
    const std::string threadName = "MetaScheduler";
    for (uint32_t coreId = 0; coreId < totalCoreCount; ++coreId)
    {
        if (CPU_ISSET(coreId, &schedSet))
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "MetaScheduler is created. count: {}, coreId: {}",
                CPU_COUNT(&schedSet), coreId);

            ioScheduler_ = new MetaFsIoScheduler(0, coreId, totalCoreCount,
                threadName, mioSet, configManager_, tp);
            ioScheduler_->StartThread();
            break;
        }
    }
}
} // namespace pos
