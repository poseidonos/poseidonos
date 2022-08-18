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

#include "log_write_context_factory.h"

#include "src/allocator/stripe/stripe.h"
#include "src/journal_manager/config/journal_configuration.h"
#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/journal_manager/log_buffer/log_group_reset_context.h"

namespace pos
{
LogWriteContextFactory::LogWriteContextFactory(void)
: config(nullptr),
  notifier(nullptr)
{
}

LogWriteContextFactory::~LogWriteContextFactory(void)
{
}

void
LogWriteContextFactory::Init(JournalConfiguration* journalConfig, LogBufferWriteDoneNotifier* target)
{
    config = journalConfig;
    notifier = target;
}

LogWriteContext*
LogWriteContextFactory::CreateBlockMapLogWriteContext(VolumeIoSmartPtr volumeIo, EventSmartPtr callbackEvent)
{
    uint32_t volId = volumeIo->GetVolumeId();
    BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint64_t numBlks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);

    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    int wbIndex = volumeIo->GetVolumeId();
    StripeAddr writeBufferStripeAddress = volumeIo->GetLsidEntry(); // TODO(huijeong.kim): to only have wbLsid

    BlockWriteDoneLogHandler* log = new BlockWriteDoneLogHandler(volId, startRba,
        numBlks, startVsa, wbIndex, writeBufferStripeAddress);

    MapList dirtyMap;
    dirtyMap.emplace(volId);

    MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log, dirtyMap,
        callbackEvent, notifier);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateStripeMapLogWriteContext(Stripe* stripe,
    StripeAddr oldAddr, EventSmartPtr callbackEvent)
{
    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe->GetUserLsid()};

    StripeMapUpdatedLogHandler* log = new StripeMapUpdatedLogHandler(stripe->GetVsid(), oldAddr, newAddr);

    MapList dirtyMap;
    dirtyMap.emplace(STRIPE_MAP_ID);

    MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log, dirtyMap,
        callbackEvent, notifier);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateGcBlockMapLogWriteContext(GcStripeMapUpdateList mapUpdates,
    EventSmartPtr callbackEvent)
{
    GcBlockWriteDoneLogHandler* log = new GcBlockWriteDoneLogHandler(mapUpdates.volumeId,
        mapUpdates.vsid, mapUpdates.blockMapUpdateList);

    MapList dummyDirty;
    MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log,
        dummyDirty, callbackEvent, notifier);

    return logWriteContext;
}

std::vector<LogWriteContext*>
LogWriteContextFactory::CreateGcBlockMapLogWriteContexts(GcStripeMapUpdateList mapUpdates,
    EventSmartPtr callbackEvent)
{
    std::vector<LogWriteContext*> returnList;

    uint64_t maxNumLogsInAContext = _GetMaxNumGcBlockMapUpdateInAContext();
    uint64_t totalNumBlocks = mapUpdates.blockMapUpdateList.size();
    uint64_t remainingBlocks = totalNumBlocks;

    while (remainingBlocks != 0)
    {
        uint64_t numBlocks = (remainingBlocks > maxNumLogsInAContext) ? (maxNumLogsInAContext) : (remainingBlocks);

        GcBlockMapUpdateList list;
        int startOffset = totalNumBlocks - remainingBlocks;
        for (uint64_t offset = 0; offset < numBlocks; offset++)
        {
            list.push_back(mapUpdates.blockMapUpdateList[startOffset + offset]);
        }

        GcBlockWriteDoneLogHandler* log = new GcBlockWriteDoneLogHandler(mapUpdates.volumeId,
            mapUpdates.vsid, list);

        MapList dummyDirty;
        MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log,
            dummyDirty, callbackEvent, notifier);

        returnList.push_back(logWriteContext);

        remainingBlocks -= numBlocks;
    }

    return returnList;
}

uint64_t
LogWriteContextFactory::_GetMaxNumGcBlockMapUpdateInAContext(void)
{
    uint64_t logHeaderSize = sizeof(GcBlockWriteDoneLog);

    uint64_t maxLogSize = config->GetMetaPageSize();
    uint64_t maxNumLogsInAContext = (maxLogSize - logHeaderSize) / sizeof(GcBlockMapUpdate);

    return maxNumLogsInAContext;
}

LogWriteContext*
LogWriteContextFactory::CreateGcStripeFlushedLogWriteContext(GcStripeMapUpdateList mapUpdates,
    EventSmartPtr callbackEvent)
{
    GcStripeFlushedLogHandler* log = new GcStripeFlushedLogHandler(mapUpdates.volumeId, mapUpdates.vsid,
        mapUpdates.wbLsid, mapUpdates.userLsid, mapUpdates.blockMapUpdateList.size());

    MapList dirtyMap;
    dirtyMap.emplace(STRIPE_MAP_ID);
    dirtyMap.emplace(mapUpdates.volumeId);
    MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext(log,
        dirtyMap, callbackEvent, notifier);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateVolumeDeletedLogWriteContext(int volId,
    uint64_t contextVersion, EventSmartPtr callback)
{
    LogHandlerInterface* log = new VolumeDeletedLogEntry(volId, contextVersion);
    LogWriteContext* logWriteContext = new LogWriteContext(log, callback, notifier);

    return logWriteContext;
}

LogGroupResetContext*
LogWriteContextFactory::CreateLogGroupResetContext(uint64_t offset, int id, uint64_t groupSize, EventSmartPtr callbackEvent, char* dataBuffer)
{
    LogGroupResetContext* resetRequest = new LogGroupResetContext(id, callbackEvent);
    resetRequest->SetIoRequest(offset, groupSize, dataBuffer);
    return resetRequest;
}
} // namespace pos
