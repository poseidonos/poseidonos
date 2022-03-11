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

#include <string>
#include <unordered_map>
#include "src/metafs/include/metafs_service.h"
#include "src/metafs/mim/scalable_meta_io_worker.h"
#include "src/metafs/mim/metafs_io_scheduler.h"
#include "src/metafs/config/metafs_config_manager.h"

namespace pos
{
MetaFsService::MetaFsService(void)
: ioScheduler(nullptr),
  configManager(new MetaFsConfigManager(ConfigManagerSingleton::Instance()))
{
    fileSystems.fill(nullptr);
}

MetaFsService::MetaFsService(MetaFsConfigManager* configManager)
: ioScheduler(nullptr),
  configManager(configManager)
{
}

MetaFsService::~MetaFsService(void)
{
    if (nullptr != ioScheduler)
    {
        // exit mioHandler thread
        ioScheduler->ClearHandlerThread();

        // exit scheduler thread
        ioScheduler->ExitThread();

        // delete the scheduler
        delete ioScheduler;
    }
}

void
MetaFsService::Initialize(const uint32_t totalCount, const cpu_set_t schedSet,
    const cpu_set_t workSet, TelemetryPublisher* tp)
{
    if (!configManager->Init())
    {
        POS_TRACE_ERROR(static_cast<int>(POS_EVENT_ID::MFS_INVALID_CONFIG),
            "The config values are invalid.");
        assert(false);
    }
    _PrepareThreads(totalCount, schedSet, workSet, tp);
}

void
MetaFsService::Register(std::string& arrayName, int arrayId, MetaFs* fileSystem)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "New metafs instance registered. arrayId={}, arrayName={}",
            arrayId, arrayName);

    arrayNameToId.insert(std::pair<std::string, int>(arrayName, arrayId));
    fileSystems[arrayId] = fileSystem;
}

void
MetaFsService::Deregister(std::string& arrayName)
{
    int arrayId = arrayNameToId[arrayName];
    arrayNameToId.erase(arrayName);

    POS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
            "A metafs instance deregistered. arrayName={}", arrayName);

    fileSystems[arrayId] = nullptr;
}

MetaFs*
MetaFsService::GetMetaFs(std::string& arrayName) const
{
    auto iter = arrayNameToId.find(arrayName);
    if (iter == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return fileSystems[iter->second];
    }
}

MetaFs*
MetaFsService::GetMetaFs(int arrayId) const
{
    return fileSystems[arrayId];
}

void
MetaFsService::_PrepareThreads(const uint32_t totalCount, const cpu_set_t schedSet,
    const cpu_set_t workSet, TelemetryPublisher* tp)
{
    uint32_t availableMetaIoCoreCnt = CPU_COUNT(&workSet);
    uint32_t handlerId = 0;

    // meta io scheduler
    for (uint32_t coreId = 0; coreId < totalCount; ++coreId)
    {
        if (CPU_ISSET(coreId, &schedSet))
        {
            ioScheduler = new MetaFsIoScheduler(0, coreId, totalCount);
            ioScheduler->StartThread();
            break;
        }
    }

    // meta io handler
    for (uint32_t coreId = 0; coreId < totalCount; ++coreId)
    {
        if (CPU_ISSET(coreId, &workSet))
        {
            _InitiateMioHandler(handlerId++, coreId, totalCount, tp);
            availableMetaIoCoreCnt--;

            if (availableMetaIoCoreCnt == 0)
            {
                break;
            }
        }
    }
}

ScalableMetaIoWorker*
MetaFsService::_InitiateMioHandler(const int handlerId, const int coreId,
    const int coreCount, TelemetryPublisher* tp)
{
    ScalableMetaIoWorker* mioHandler =
        new ScalableMetaIoWorker(handlerId, coreId, coreCount, configManager, tp);
    mioHandler->StartThread();
    ioScheduler->RegisterMioHandler(mioHandler);

    return mioHandler;
}
} // namespace pos
