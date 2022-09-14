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

#include <string>
#include <unordered_map>

#include "src/metafs/config/metafs_config_manager.h"
#include "src/metafs/mim/metafs_io_scheduler.h"
#include "src/metafs/mim/metafs_io_scheduler_factory.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
MetaFsService::MetaFsService(void)
: MetaFsService(new MetaFsConfigManager(ConfigManagerSingleton::Instance()), new MetaFsIoSchedulerFactory())
{
}

MetaFsService::MetaFsService(MetaFsConfigManager* configManager, MetaFsIoSchedulerFactory* factory, const int numaCount)
: MAX_SCHEDULER_COUNT(numaCount),
  configManager_(configManager),
  tp_(nullptr),
  factory_(factory)
{
    fileSystems_.fill(nullptr);
}

MetaFsService::~MetaFsService(void)
{
    int count = 0;
    for (auto info : ioScheduler_)
    {
        auto scheduler = info.second;
        POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
            "MetaScheduler #{} is terminated", count);

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

    if (configManager_)
    {
        POS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
            "Delete MetaFsConfigManager");

        delete configManager_;
        configManager_ = nullptr;
    }

    if (factory_)
    {
        delete factory_;
        factory_ = nullptr;
    }
}

void
MetaFsService::Initialize(const uint32_t totalCoreCount, const cpu_set_t schedSet,
    const cpu_set_t mioSet, TelemetryPublisher* tp)
{
    if (!configManager_->Init())
    {
        POS_TRACE_ERROR(static_cast<int>(EID(MFS_INVALID_CONFIG)),
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
    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
        "New metafs instance registered. arrayId: {}, arrayName: {}",
        arrayId, arrayName);

    arrayNameToId_.insert(std::pair<std::string, int>(arrayName, arrayId));
    fileSystems_[arrayId] = fileSystem;
}

void
MetaFsService::Deregister(const std::string& arrayName)
{
    const int arrayId = arrayNameToId_[arrayName];

    POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
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
    int countOfScheduler = CPU_COUNT(&schedSet);
    if (!_CheckSchedulerSettingFromConfig(countOfScheduler))
    {
        POS_TRACE_ERROR(EID(MFS_NEED_TO_CHECK_SCHEDULER_SETTING),
            "A scheduler needs to be created at least, countOfScheduler:{}",
            countOfScheduler);
        assert(false);
    }

    const std::string threadName = "MetaScheduler";
    int numOfSchedulersCreated = 0;
    bool needToIgnoreNuma = (countOfScheduler < MAX_SCHEDULER_COUNT) ? true : false;

    configManager_->SetIgnoreNumaDedicatedScheduling(needToIgnoreNuma);

    for (uint32_t coreId = 0; coreId < totalCoreCount; ++coreId)
    {
        if (CPU_ISSET(coreId, &schedSet))
        {
            MetaFsIoScheduler* scheduler = factory_->CreateMetaFsIoScheduler(0, coreId, totalCoreCount,
                threadName, mioSet, configManager_, tp_);

            int numaId = needToIgnoreNuma ? 0 : _GetNumaId(coreId);
            POS_TRACE_INFO(EID(MFS_INFO_MESSAGE),
                "MetaScheduler #{} is created, coreId: {}, numaId: [}",
                numOfSchedulersCreated, coreId, numaId);

            if (ioScheduler_.find(numaId) == ioScheduler_.end())
            {
                ioScheduler_.insert({numaId, scheduler});
                scheduler->StartThread();
                numOfSchedulersCreated++;
            }
            else
            {
                POS_TRACE_ERROR(EID(MFS_TRY_TO_CREATE_SCHEDULER_IN_THE_SAME_NUMA),
                    "Only one scheduler can be created for each NUMA");
                delete scheduler;
                assert(false);
            }
        }
    }
}

bool
MetaFsService::_CheckSchedulerSettingFromConfig(const int countOfScheduler) const
{
    if (countOfScheduler == 0)
    {
        POS_TRACE_ERROR(EID(MFS_SCHEDULER_NONE),
            "A scheduler needs to be created at least, countOfScheduler:{}",
            countOfScheduler);
        return false;
    }
    else if (countOfScheduler > MAX_SCHEDULER_COUNT)
    {
        POS_TRACE_ERROR(EID(MFS_MAX_SCHEDULER_EXCEEDED),
            "It has exceeded the maximum number that can be generated, countOfScheduler:{}, MAX_SCHEDULER_COUNT: {}",
            countOfScheduler, MAX_SCHEDULER_COUNT);
        return false;
    }

    if (configManager_->IsSupportingNumaDedicatedScheduling())
    {
        if (countOfScheduler < MAX_SCHEDULER_COUNT)
        {
            POS_TRACE_WARN(EID(MFS_SCHEDULER_COUNT_SMALLER_THAN_EXPECTED),
                "Numa_dedicated is set but the count of metafs io scheduler is not more than one");
            return true;
        }
    }
    else
    {
        if (countOfScheduler > 1)
        {
            POS_TRACE_ERROR(EID(MFS_UNNECESSARY_SCHEDULER_SET),
                "This meta scheduler will not be created, when the numa_dedicated setting is turned on");
            return false;
        }
    }

    return true;
}

uint32_t
MetaFsService::_GetNumaIdConsideringNumaDedicatedScheduling(const uint32_t numaId) const
{
    return configManager_->IsSupportingNumaDedicatedScheduling() ? numaId : 0;
}

bool
MetaFsService::_CheckIfPossibleToCreateScheduler(const int numOfSchedulersCreated)
{
    if (configManager_->IsSupportingNumaDedicatedScheduling())
    {
        return true;
    }

    if (numOfSchedulersCreated == 1)
    {
        return false;
    }

    return true;
}
} // namespace pos
