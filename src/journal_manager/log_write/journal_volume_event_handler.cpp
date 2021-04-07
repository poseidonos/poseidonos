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

#include "journal_volume_event_handler.h"

#include <functional>

#include "../checkpoint/dirty_map_manager.h"
#include "../journal_configuration.h"
#include "../log_buffer/journal_log_buffer.h"
#include "../log_buffer/log_write_context_factory.h"
#include "log_write_handler.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
JournalVolumeEventHandler::JournalVolumeEventHandler(void)
: VolumeEvent("Journal"),
  isInitialized(false),
  isJournalEnabled(false),
  config(nullptr),
  logFactory(nullptr),
  dirtyPageManager(nullptr),
  logWriteHandler(nullptr)
{
    volumeDeleteInProgress.clear();
    this->RegisterToPublisher();
}

JournalVolumeEventHandler::~JournalVolumeEventHandler(void)
{
    this->RemoveFromPublisher();
}

void
JournalVolumeEventHandler::Init(LogWriteContextFactory* factory,
    DirtyMapManager* dirtyPages, LogWriteHandler* writter,
    JournalConfiguration* journalConfiguration)
{
    config = journalConfiguration;
    logFactory = factory;
    dirtyPageManager = dirtyPages;
    logWriteHandler = writter;

    isInitialized = true;
}

bool
JournalVolumeEventHandler::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte)
{
    int ret = 0;

    if (isInitialized == true && config->IsEnabled() == true)
    {
        auto status = volumeDeleteInProgress.find(volID);
        if (status == volumeDeleteInProgress.end() || (status->second == false))
        {
            volumeDeleteInProgress.emplace(volID, true);

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_VOLUME_DELETE_LOG_WRITE_STARTED,
                "Issue writes for volume {} deleted log event", volID);

            LogWriteContext* logWriteContext = logFactory->CreateVolumeDeletedLogWriteContext(volID,
                std::bind(&JournalVolumeEventHandler::VolumeDeletedLogWriteDone, this, std::placeholders::_1));
            logWriteHandler->AddLog(logWriteContext);
        }
        _WaitForLogWriteDone(volID);
        dirtyPageManager->DeleteDirtyList(volID);

        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_VOLUME_DELETE_LOG_WRITTEN,
            "Volume deleted log event is written");
    }

    return (ret == 0);
}

void
JournalVolumeEventHandler::_WaitForLogWriteDone(int volumeId)
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&] {
        return (volumeDeleteInProgress[volumeId] == false);
    });
}

void
JournalVolumeEventHandler::VolumeDeletedLogWriteDone(int volumeId)
{
    std::unique_lock<std::mutex> lock(mutex);
    volumeDeleteInProgress[volumeId] = false;
    cv.notify_all();
}

bool
JournalVolumeEventHandler::VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
JournalVolumeEventHandler::VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
JournalVolumeEventHandler::VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
JournalVolumeEventHandler::VolumeUnmounted(std::string volName, int volID)
{
    return true;
}

bool
JournalVolumeEventHandler::VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

void
JournalVolumeEventHandler::VolumeDetached(vector<int> volList)
{
}

} // namespace ibofos
