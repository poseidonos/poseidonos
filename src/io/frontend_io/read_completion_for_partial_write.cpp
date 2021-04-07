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

#include "src/io/frontend_io/read_completion_for_partial_write.h"

#include "src/device/event_framework_api.h"
#include "src/device/ioat_api.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/frontend_io/partial_write_completion.h"
#include "src/io/general_io/translator.h"
#include "src/io/general_io/volume_io.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
ReadCompletionForPartialWrite::ReadCompletionForPartialWrite(
    VolumeIoSmartPtr volumeIo)
: Callback(true),
  volumeIo(volumeIo)
{
}

ReadCompletionForPartialWrite::~ReadCompletionForPartialWrite(void)
{
}

void
ReadCompletionForPartialWrite::_HandleCopyDone(void* argument)
{
    try
    {
        if (unlikely(nullptr == argument))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::BLKALGN_INVALID_UBIO;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
            throw eventId;
        }

        VolumeIoSmartPtr volumeIo = *(static_cast<VolumeIoSmartPtr*>(argument));
        Translator translator(volumeIo->GetVsa());
        void* mem = volumeIo->GetBuffer();
        PhysicalEntries physicalEntries = translator.GetPhysicalEntries(mem, 1);
        VolumeIoSmartPtr split = volumeIo->GetOriginVolumeIo();
        StripeAddr lsidEntry = translator.GetLsidEntry(0);
        split->SetLsidEntry(lsidEntry);

        // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
        assert(physicalEntries.size() == 1);
        for (auto& physicalEntry : physicalEntries)
        {
            PhysicalBlkAddr pba = physicalEntry.addr;
            assert(physicalEntry.buffers.size() == 1);

            for (auto& buffer : physicalEntry.buffers)
            {
                uint32_t blockCount = buffer.GetBlkCnt();
                assert(1 == blockCount);
                volumeIo->dir = UbioDir::Write;
                volumeIo->SetPba(pba);
                volumeIo->ClearCallback();
                CallbackSmartPtr completion(
                    new PartialWriteCompletion(volumeIo));
                CallbackSmartPtr splitCallback = split->GetCallback();
                splitCallback->SetWaitingCount(1);
                completion->SetCallee(splitCallback);
                volumeIo->SetCallback(completion);

                _SendVolumeIo(volumeIo);

                pba.lba += ChangeBlockToSector(blockCount);
            }
        }

        delete (static_cast<VolumeIoSmartPtr*>(argument));
    }
    catch (...)
    {
        if (nullptr != argument)
        {
            VolumeIoSmartPtr volumeIo =
                *(static_cast<VolumeIoSmartPtr*>(argument));
            CallbackSmartPtr callback = volumeIo->GetCallback();
            if (callback)
            {
                callback->InformError(CallbackError::GENERIC_ERROR);
            }
            PartialWriteCompletion event(volumeIo);
            event.Execute();
            delete (static_cast<VolumeIoSmartPtr*>(argument));
        }
    }
}

void
ReadCompletionForPartialWrite::_Copy(VolumeIoSmartPtr srcVolumeIo,
    uint64_t srcOffset, VolumeIoSmartPtr dstVolumeIo, uint64_t dstOffset,
    uint64_t size)
{
    void* src = srcVolumeIo->GetBuffer(0, ChangeByteToSector(srcOffset));
    void* dst = dstVolumeIo->GetBuffer(0, ChangeByteToSector(dstOffset));
    VolumeIoSmartPtr* volumeIoSmartPtr = new VolumeIoSmartPtr(dstVolumeIo);
    IoatApi::SubmitCopy(dst, src, size, _HandleCopyDone, volumeIoSmartPtr);
}

bool
ReadCompletionForPartialWrite::_DoSpecificJob(void)
{
    try
    {
        AlignmentInformation alignment = volumeIo->GetAlignmentInformation();
        uint32_t alignmentOffset = alignment.offset;
        uint32_t alignmentSize = alignment.size;
        VolumeIoSmartPtr split = volumeIo->GetOriginVolumeIo();
        if (unlikely(nullptr == split))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::BLKALGN_INVALID_UBIO;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
            throw eventId;
        }
        StripeAddr oldLsidEntry = volumeIo->GetOldLsidEntry();

        if (false == IsUnMapStripe(oldLsidEntry.stripeId))
        {
            uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
            MapperSingleton::Instance()->DereferLsid(oldLsidEntry, blockCount);
        }

        _Copy(split, 0, volumeIo, alignmentOffset, alignmentSize);
    }
    catch (...)
    {
        InformError(CallbackError::GENERIC_ERROR);
        PartialWriteCompletion event(volumeIo);
        event.Execute();
    }

    volumeIo = nullptr;

    return true;
}
} // namespace ibofos
