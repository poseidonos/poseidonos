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

#include "src/gc/copier_read_completion.h"

#include "Air.h"
#include "src/gc/copier_write_completion.h"
#include "src/io/frontend_io/write_submission.h"
#include "src/io/general_io/volume_io.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
CopierReadCompletion::CopierReadCompletion(VictimStripe* victimStripe,
    uint32_t listIndex,
    void* buffer,
    CopierMeta* meta,
    StripeId stripeId)
: Callback(false),
  victimStripe(victimStripe),
  listIndex(listIndex),
  buffer(buffer),
  meta(meta),
  stripeId(stripeId)
{
}

CopierReadCompletion::~CopierReadCompletion(void)
{
}

bool
CopierReadCompletion::_DoSpecificJob(void)
{
    list<BlkInfo> blkInfoList = victimStripe->GetBlkInfoList(listIndex);
    uint32_t blksCnt = blkInfoList.size();

    CallbackSmartPtr gcChunkWriteCompletion(
        new GcChunkWriteCompletion(buffer,
            blksCnt,
            meta,
            stripeId));
    gcChunkWriteCompletion->SetWaitingCount(blksCnt);

    auto blkInfo = blkInfoList.begin();
    uint32_t baseOffset = (blkInfo->vsa.offset / BLOCKS_IN_CHUNK) * BLOCKS_IN_CHUNK;

    for (auto blkInfo : blkInfoList)
    {
        VolumeIoSmartPtr volumeIo(new VolumeIo((char*)buffer +
            ((blkInfo.vsa.offset - baseOffset) * BLOCK_SIZE),
            VolumeIo::UNITS_PER_BLOCK));

        volumeIo->dir = UbioDir::Write;
        volumeIo->SetVolumeId(blkInfo.volID);
        volumeIo->SetRba(blkInfo.rba * VolumeIo::UNITS_PER_BLOCK);

        CallbackSmartPtr callback(new GcBlkWriteCompletion(volumeIo));
        callback->SetCallee(gcChunkWriteCompletion);
        volumeIo->SetCallback(callback);
        volumeIo->SetGc(blkInfo.vsa);

        AIRLOG(PERF_COPY, 0, AIR_WRITE, volumeIo->GetSize());
        bool done = false;
        WriteSubmission write(volumeIo);
        done = write.Execute();

        if (unlikely(false == done))
        {
            EventSmartPtr writeEvent(new WriteSubmission(volumeIo));
#if defined QOS_ENABLED_BE
            writeEvent->SetEventType(BackendEvent_GC);
#endif
            EventArgument::GetEventScheduler()->EnqueueEvent(writeEvent);
        }
    }

    return true;
}

} // namespace ibofos
