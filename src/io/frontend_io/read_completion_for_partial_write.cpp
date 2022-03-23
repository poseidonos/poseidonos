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

#include "src/io/frontend_io/read_completion_for_partial_write.h"

#include "src/allocator_service/allocator_service.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/translator.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/event_framework_api.h"

#include "src/array_mgmt/array_manager.h"
#include "src/io/frontend_io/write_for_parity.h"
namespace pos
{
ReadCompletionForPartialWrite::ReadCompletionForPartialWrite(
    VolumeIoSmartPtr volumeIo, uint32_t alignmentSize, uint32_t alignmentOffset,
    IWBStripeAllocator* inputIWBStripeAllocator, bool tested)
: Callback(true, CallbackType_ReadCompletionForPartialWrite),
  volumeIo(volumeIo),
  alignmentSize(alignmentSize),
  alignmentOffset(alignmentOffset),
  iWBStripeAllocator(inputIWBStripeAllocator),
  tested(tested)
{
    if (likely(iWBStripeAllocator == nullptr))
    {
        iWBStripeAllocator = AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(volumeIo->GetArrayId());
    }
}

ReadCompletionForPartialWrite::~ReadCompletionForPartialWrite(void)
{
}

void
ReadCompletionForPartialWrite::HandleCopyDone(void* argument)
{
    Translator* translator = nullptr;
    try
    {
        if (unlikely(nullptr == argument))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::BLKALGN_INVALID_UBIO;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Block aligning Ubio is null");
            throw eventId;
        }
        CopyParameter* copyParam = static_cast<CopyParameter*>(argument);
        VolumeIoSmartPtr volumeIo = copyParam->volumeIo;
        translator = copyParam->translator;

        // If UT is executed, translator will be input.
        // otherwise, translator will be nullptr
        if (likely(translator == nullptr))
        {
            Translator translatorLocal(volumeIo->GetVsa(), volumeIo->GetArrayId(), volumeIo->GetUserLsid());
            translator = &translatorLocal;
        }
        void* mem = volumeIo->GetBuffer();

        list<PhysicalEntry> physicalEntries = translator->GetPhysicalEntries(mem, 1);
        VolumeIoSmartPtr split = volumeIo->GetOriginVolumeIo();
        StripeAddr lsidEntry = translator->GetLsidEntry(0);
        split->SetLsidEntry(lsidEntry);

        // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
        assert(physicalEntries.size() == 1);

        for (auto& physicalEntry : physicalEntries)
        {
            PhysicalBlkAddr pba = physicalEntry.addr;
            {
                uint32_t blockCount = 1;
                assert(1 == blockCount);
                volumeIo->dir = UbioDir::Write;
                volumeIo->SetPba(pba);
                volumeIo->ClearCallback();
                volumeIo->SetLsidEntry(lsidEntry);
                CallbackSmartPtr splitCallback = split->GetCallback();
                volumeIo->SetCallback(splitCallback);
                if (likely(copyParam->tested == false))
                {
                    IArrayInfo *arrayInfo = ArrayMgr()->GetInfo(volumeIo->GetArrayId())->arrayInfo;
                    if (true == arrayInfo->IsWriteThroughEnabled())
                    {
                        WriteForParity writeForParity(volumeIo);
                        bool ret  = writeForParity.Execute();
                        if (ret == false)
                        {
                            POS_EVENT_ID eventId = POS_EVENT_ID::WRITE_FOR_PARITY_FAILED;
                            POS_TRACE_ERROR(static_cast<int>(eventId),
                                "Failed to copy user data to dram for parity");
                        }
                    }
                    _SendVolumeIo(volumeIo);
                }

                pba.lba += ChangeBlockToSector(blockCount);
            }
        }

        delete (static_cast<CopyParameter*>(argument));
    }
    catch (...)
    {
        if (nullptr != argument)
        {
            VolumeIoSmartPtr volumeIo =
                *(static_cast<VolumeIoSmartPtr*>(argument));
            UbioSmartPtr origin = volumeIo->GetOriginUbio();
            if (origin != nullptr)
            {
                IoCompleter ioCompleter(origin);
                ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::GENERIC_ERROR, true);
            }
            delete (static_cast<CopyParameter*>(argument));
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
    CopyParameter* copyParam = nullptr;
    if (tested == false)
    {
        copyParam = new CopyParameter(dstVolumeIo, nullptr);
    }
    AccelEngineApi::SubmitCopy(dst, src, size, HandleCopyDone, copyParam);
}

bool
ReadCompletionForPartialWrite::_DoSpecificJob(void)
{
    try
    {
        VolumeIoSmartPtr split = volumeIo->GetOriginVolumeIo();
        if (unlikely(nullptr == split))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::BLKALGN_INVALID_UBIO;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Block aligning Ubio is null");
            throw eventId;
        }
        StripeAddr oldLsidEntry = volumeIo->GetOldLsidEntry();

        if (false == IsUnMapStripe(oldLsidEntry.stripeId))
        {
            uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
            iWBStripeAllocator->DereferLsidCnt(oldLsidEntry, blockCount);
        }

        _Copy(split, 0, volumeIo, alignmentOffset, alignmentSize);
    }
    catch (...)
    {
        UbioSmartPtr origin = volumeIo->GetOriginUbio();
        if (origin != nullptr)
        {
            IoCompleter ioCompleter(origin);
            ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::GENERIC_ERROR, true);
        }
    }

    volumeIo = nullptr;

    return true;
}

CopyParameter::CopyParameter(VolumeIoSmartPtr volumeIo, Translator* translator, bool tested)
: volumeIo(volumeIo),
  translator(translator),
  tested(tested)
{
}
} // namespace pos
