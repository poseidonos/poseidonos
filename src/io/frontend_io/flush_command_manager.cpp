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

#include "src/io/frontend_io/flush_command_manager.h"

#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/io/frontend_io/flush_command_handler.h"

#include <memory>

namespace pos
{
static const uint64_t MAX_THRESHOLD = 100;

FlushCmdManager::FlushCmdManager(EventScheduler* eventSchedulerArg)
: metaFlushInProgress(false),
  eventScheduler(eventSchedulerArg)
{
    for (int i = 0; i< MAX_VOLUME_COUNT; i++)
    {
        flushInProgress[i] = false;
    }
}

FlushCmdManager::~FlushCmdManager(void)
{
}

bool
FlushCmdManager::IsFlushEnabled(void)
{
    return config.IsEnabled();
}

bool
FlushCmdManager::CanFlushMeta(FlushIoSmartPtr flushIo)
{
    std::unique_lock<std::mutex> lock(metaFlushLock);

    if (metaFlushInProgress == false)
    {
        metaFlushInProgress = true;
        return true;
    }

    flushEvents.push_back(flushIo);

    return false;
}

void
FlushCmdManager::FinishMetaFlush(void)
{
    std::unique_lock<std::mutex> lock(metaFlushLock);

    if (flushEvents.size() == 0)
    {
        metaFlushInProgress = false;
    }
    else
    {
        FlushIoSmartPtr flushIo = flushEvents.front();
        flushEvents.pop_front();

        EventSmartPtr flushCmdHandler =
                std::make_shared<FlushCmdHandler>(flushIo);

        if (flushIo->IsInternalFlush() == true)
        {
            if (nullptr == eventScheduler)
            {
                eventScheduler = EventSchedulerSingleton::Instance();
            }
            eventScheduler->EnqueueEvent(flushCmdHandler);
        }
        else
        {
            SpdkEventScheduler::SendSpdkEvent(flushIo->GetOriginCore(), flushCmdHandler);
        }
    }
}

void
FlushCmdManager::UpdateVSANewEntries(uint32_t volId, int arrayId)
{
    {
        std::unique_lock<std::mutex> lock(createAndExecFlushLock);
        if (backendFlushInProgress[volId] == true)
        {
            return;
        }
        backendFlushInProgress[volId] = true;
    }

    FlushIoSmartPtr flushIo(new FlushIo(arrayId));
    flushIo->SetVolumeId(volId);
    flushIo->SetInternalFlush(true);

    EventSmartPtr flushCmdHandler(new FlushCmdHandler(flushIo));
    if (nullptr == eventScheduler)
    {
         eventScheduler = EventSchedulerSingleton::Instance();
    }
    eventScheduler->EnqueueEvent(flushCmdHandler);
}

bool
FlushCmdManager::IsInternalFlushEnabled(void)
{
    return config.IsInternalFlushEnabled();
}

int
FlushCmdManager::GetInternalFlushThreshold(void)
{
    return config.GetInternalFlushThreshold();
}

bool
FlushCmdManager::TrySetFlushInProgress(uint32_t volId)
{
    return (flushInProgress[volId].exchange(true) == false);
}

void
FlushCmdManager::ResetFlushInProgress(uint32_t volId, bool isBackendFlush)
{
    flushInProgress[volId] = false;

    if (isBackendFlush == true)
    {
        backendFlushInProgress[volId] = false;
    }
}

} // namespace pos
