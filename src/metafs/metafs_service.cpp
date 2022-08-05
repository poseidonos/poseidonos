/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <numa.h>

#include <string>
#include <unordered_map>

#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/mim/metafs_io_scheduler.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
MetaFsService::MetaFsService(void)
: configManager_(new MetaFsConfigManager(ConfigManagerSingleton::Instance())),
  needToRemoveConfig_(true),
  tp_(nullptr)
{
}

MetaFsService::MetaFsService(MetaFsConfigManager* configManager)
: configManager_(configManager),
  needToRemoveConfig_(false),
  tp_(nullptr)
{
    fileSystems_.fill(nullptr);
}

MetaFsService::~MetaFsService(void)
{
    int count = 0;
    for (auto info : ioScheduler_)
    {
        auto scheduler = info.second;
        POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "MetaScheduler #{} is suspended", count);

        scheduler->ExitThread();

        delete scheduler;
        count++;
    }
    ioScheduler_.clear();

    if (tp_)
    {
        tp_->StopPublishing();
        TelemetryClientSingleton::Instance()->DeregisterPublisher(tp_->GetName());
        delete tp_;
        tp_ = nullptr;
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

    tp_ = tp;
    if (!tp_)
    {
        tp_ = new TelemetryPublisher{"meta_scheduler"};
        TelemetryClientSingleton::Instance()->RegisterPublisher(tp_);
    }

    _CreateScheduler(totalCoreCount, schedSet, mioSet);
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
    const cpu_set_t schedSet, const cpu_set_t mioSet)
{
    const std::string threadName = "MetaScheduler";
    int numScheduler = 0;
    for (uint32_t coreId = 0; coreId < totalCoreCount; ++coreId)
    {
        if (CPU_ISSET(coreId, &schedSet))
        {
            POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "MetaScheduler #{} is created, coreId: {}",
                numScheduler, coreId);

            MetaFsIoScheduler* scheduler = new MetaFsIoScheduler(0, coreId, totalCoreCount,
                threadName, mioSet, configManager_, tp_,
                new MetaFsTimeInterval(configManager_->GetTimeIntervalInMillisecondsForMetric()),
                configManager_->GetWrrWeight(), configManager_->IsSupportingNumaDedicatedScheduling());

            if (ioScheduler_.find(numa_node_of_cpu(coreId)) == ioScheduler_.end())
            {
                ioScheduler_.insert({numa_node_of_cpu(coreId), scheduler});
                scheduler->StartThread();
                numScheduler++;
            }
            else
            {
                delete scheduler;
            }
        }
    }

    if (numScheduler > MAX_SCHEDULER_COUNT)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::MFS_INVALID_PARAMETER,
            "It has exceeded the maximum number that can be generated, numScheduler:{}",
            numScheduler);
        assert(false);
    }
}
} // namespace pos
