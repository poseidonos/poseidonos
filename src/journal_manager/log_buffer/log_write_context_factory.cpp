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

#include "log_write_context_factory.h"

#include "../log/log_handler.h"
#include "src/allocator/active_stripe_index_info.h"
#include "src/allocator/stripe.h"

namespace ibofos
{
LogWriteContextFactory::LogWriteContextFactory(void)
: notifier(nullptr)
{
}

LogWriteContextFactory::~LogWriteContextFactory(void)
{
}

void
LogWriteContextFactory::Init(LogBufferWriteDoneNotifier* target)
{
    notifier = target;
}

LogWriteContext*
LogWriteContextFactory::CreateBlockMapLogWriteContext(VolumeIoSmartPtr volumeIo,
    MpageList dirty, EventSmartPtr callbackEvent)
{
    uint32_t volId = volumeIo->GetVolumeId();
    BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetRba());
    uint64_t numBlks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);

    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    ActiveStripeTailArrIdxInfo wbIndexFinder(volumeIo->GetVolumeId(), volumeIo->IsGc());
    int wbIndex = wbIndexFinder.GetActiveStripeTailArrIdx();
    StripeAddr writeBufferStripeAddress = volumeIo->GetLsidEntry(); // TODO(huijeong.kim): to only have wbLsid

    bool isGC = false;
    VirtualBlkAddr oldVsa = UNMAP_VSA;
    if (volumeIo->IsGc() == true)
    {
        isGC = true;
        oldVsa = volumeIo->GetOldVsa();
    }

    BlockWriteDoneLogHandler* log = new BlockWriteDoneLogHandler(volId, startRba,
        numBlks, startVsa, wbIndex, writeBufferStripeAddress, oldVsa, isGC);

    MapPageList dirtyMap;
    dirtyMap.emplace(volId, dirty);

    MapUpdateLogWriteContext* logWriteContext = new BlockMapUpdatedLogWriteContext(volumeIo, log, dirtyMap,
        callbackEvent, notifier);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateStripeMapLogWriteContext(Stripe* stripe,
    StripeAddr oldAddr, MpageList dirty, EventSmartPtr callbackEvent)
{
    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe->GetUserLsid()};

    StripeMapUpdatedLogHandler* log = new StripeMapUpdatedLogHandler(stripe->GetVsid(), oldAddr, newAddr);

    MapPageList dirtyMap;
    dirtyMap.emplace(STRIPE_MAP_ID, dirty);

    MapUpdateLogWriteContext* logWriteContext = new StripeMapUpdatedLogWriteContext(log, dirtyMap, callbackEvent,
        notifier);

    return logWriteContext;
}

LogWriteContext*
LogWriteContextFactory::CreateVolumeDeletedLogWriteContext(int volId,
    JournalInternalEventCallback callback)
{
    LogHandlerInterface* log = new VolumeDeletedLogEntry(volId);
    LogWriteContext* logWriteContext = new VolumeDeletedLogWriteContext(volId, log, callback);

    return logWriteContext;
}

} // namespace ibofos
