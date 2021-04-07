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

#include "src/io/frontend_io/write_submission.h"

#include <mutex>

#include "src/allocator/allocator.h"
#include "src/device/event_framework_api.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/include/address_type.h"
#include "src/include/meta_const.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/block_map_update_request.h"
#include "src/io/frontend_io/read_completion_for_partial_write.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/translator.h"
#include "src/io/general_io/ubio.h"
#include "src/io/general_io/volume_io.h"
#include "src/logger/logger.h"
#include "src/state/state_manager.h"
#include "src/state/state_policy.h"

namespace ibofos
{
WriteSubmission::WriteSubmission(VolumeIoSmartPtr volumeIo)
: Event(EventFrameworkApi::IsReactorNow()),
  rbaStateManager(*RbaStateManagerSingleton::Instance()),
  allocator(AllocatorSingleton::Instance()),
  volumeIo(volumeIo),
  blockCount(0),
  volumeId(volumeIo->GetVolumeId()),
  volumeIoCount(0),
  remainSize(0),
  blockAlignment(ChangeSectorToByte(volumeIo->GetRba()), volumeIo->GetSize()),
  allocatedBlockCount(volumeIo->GetAllocatedBlockCount()),
  processedBlockCount(0)
{
}

WriteSubmission::~WriteSubmission(void)
{
}

uint32_t
WriteSubmission::GetOriginCore(void)
{
    uint32_t originCore = volumeIo->GetOriginCore();
    return originCore;
}

bool
WriteSubmission::Execute(void)
{
    try
    {
        BlkAddr startRba = ChangeSectorToBlock(volumeIo->GetRba());
        blockCount = blockAlignment.GetBlockCount();
        remainSize = ChangeBlockToByte(blockCount);

        bool ownershipAcquired = rbaStateManager.BulkAcquireOwnership(volumeId,
            startRba, blockCount);
        if (false == ownershipAcquired)
        {
            return false;
        }

        bool done = _ProcessOwnedWrite();

        if (unlikely(!done))
        {
            rbaStateManager.BulkReleaseOwnership(volumeId, startRba,
                blockCount);
        }
        else
        {
            volumeIo = nullptr;
        }

        return done;
    }
    catch (...)
    {
        if (nullptr != volumeIo)
        {
            volumeIo->CompleteWithoutRecovery(CallbackError::GENERIC_ERROR);
        }
        volumeIo = nullptr;
        return true;
    }
}

bool
WriteSubmission::_ProcessOwnedWrite(void)
{
    _AllocateFreeWriteBuffer();

    if (blockCount > allocatedBlockCount)
    {
        return false;
    }

    _PrepareBlockAlignment();

    if (processedBlockCount == 0 && allocatedBlockCount == 1)
    {
        _WriteSingleBlock();
        return true;
    }

    _WriteMultipleBlocks();
    _SubmitVolumeIo();

    return true;
}

void
WriteSubmission::_SubmitVolumeIo(void)
{
    uint32_t volumeIoCount = splitVolumeIoQueue.size();
    CallbackSmartPtr callback = volumeIo->GetCallback();
    callback->SetWaitingCount(volumeIoCount);

    while (splitVolumeIoQueue.empty() == false)
    {
        VolumeIoSmartPtr volumeIo = splitVolumeIoQueue.front();
        bool skipIoSubmission = (false == volumeIo->CheckPbaSet());
        if (skipIoSubmission)
        {
            volumeIo->CompleteWithoutRecovery(CallbackError::SUCCESS);
        }
        else
        {
            _SendVolumeIo(volumeIo);
        }
        splitVolumeIoQueue.pop();
    }
}

void
WriteSubmission::_WriteSingleBlock(void)
{
    uint32_t count = volumeIo->GetAllocatedVirtualBlksCount();
    for (uint32_t index = 0; index < count; index++)
    {
        VirtualBlks& vsaRange = volumeIo->GetAllocatedVirtualBlks(index);

        if (vsaRange.numBlks != 0)
        {
            _PrepareSingleBlock(vsaRange);
            _SendVolumeIo(volumeIo);
            return;
        }
    }
}

void
WriteSubmission::_WriteMultipleBlocks(void)
{
    uint32_t count = volumeIo->GetAllocatedVirtualBlksCount();
    for (uint32_t index = 0; index < count; index++)
    {
        VirtualBlks& virtualBlks = volumeIo->GetAllocatedVirtualBlks(index);
        _WriteDataAccordingToVsaRange(virtualBlks);
    }
}

void
WriteSubmission::_WriteDataAccordingToVsaRange(VirtualBlks& vsaRange)
{
    if (vsaRange.numBlks != 0)
    {
        VolumeIoSmartPtr newVolumeIo = _CreateVolumeIo(vsaRange);

        splitVolumeIoQueue.push(newVolumeIo);
    }
}

void
WriteSubmission::_PrepareBlockAlignment(void)
{
    if (blockAlignment.HasHead())
    {
        _ReadOldHeadBlock();
    }

    if (blockAlignment.HasTail())
    {
        _ReadOldTailBlock();
    }
}

void
WriteSubmission::_ReadOldHeadBlock(void)
{
    BlkAddr headRba = blockAlignment.GetHeadBlock();
    VirtualBlkAddr vsa = volumeIo->PopHeadVsa();

    _ReadOldBlock(headRba, vsa, false);
}

void
WriteSubmission::_ReadOldTailBlock(void)
{
    if (remainSize == 0)
    {
        return;
    }

    BlkAddr tailRba = blockAlignment.GetTailBlock();
    VirtualBlkAddr vsa = volumeIo->PopTailVsa();

    _ReadOldBlock(tailRba, vsa, true);
}

void
WriteSubmission::_ReadOldBlock(BlkAddr rba, VirtualBlkAddr& vsa, bool isTail)
{
    uint32_t alignmentSize;
    uint32_t alignmentOffset;

    if (isTail)
    {
        alignmentSize = blockAlignment.GetTailSize();
        alignmentOffset = 0;
    }
    else
    {
        alignmentSize = blockAlignment.GetHeadSize();
        alignmentOffset = blockAlignment.GetHeadPosition();
    }

    VolumeIoSmartPtr split =
        volumeIo->Split(ChangeByteToSector(alignmentSize), isTail);
    split->SetOriginUbio(volumeIo);
    split->SetVsa(vsa);
    CallbackSmartPtr callback(new BlockMapUpdateRequest(split));
    split->SetCallback(callback);

    VolumeIoSmartPtr newVolumeIo(new VolumeIo(nullptr, Ubio::UNITS_PER_BLOCK));
    if (unlikely(nullptr == newVolumeIo))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRHDLR_FAIL_TO_ALLOCATE_MEMORY;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
        throw eventId;
    }

    newVolumeIo->SetVolumeId(volumeId);
    newVolumeIo->SetOriginUbio(split);
    CallbackSmartPtr event(new ReadCompletionForPartialWrite(newVolumeIo));
    newVolumeIo->SetCallback(event);
    newVolumeIo->SetAlignmentInformation({.offset = alignmentOffset,
        .size = alignmentSize});
    newVolumeIo->SetRba(ChangeBlockToSector(rba));
    newVolumeIo->SetVsa(vsa);

    const bool isRead = true;
    Translator oldDataTranslator(volumeId, rba, isRead);
    remainSize -= newVolumeIo->GetSize();
    StripeAddr oldLsidEntry = oldDataTranslator.GetLsidEntry(0);

    newVolumeIo->SetOldLsidEntry(oldLsidEntry);

    processedBlockCount++;
    if (oldDataTranslator.IsMapped())
    {
        newVolumeIo->dir = UbioDir::Read;
        PhysicalBlkAddr pba = oldDataTranslator.GetPba();
        newVolumeIo->SetPba(pba);
    }

    splitVolumeIoQueue.push(newVolumeIo);
}

void
WriteSubmission::_AllocateFreeWriteBuffer(void)
{
    int remainBlockCount = blockCount - allocatedBlockCount;
    if (remainBlockCount == 0)
    {
        return;
    }

    while (remainBlockCount > 0)
    {
        VirtualBlks targetVsaRange;

        if (volumeIo->IsGc())
        {
            targetVsaRange = allocator->AllocateGcBlk(volumeId,
                remainBlockCount);
        }
        else
        {
            targetVsaRange = allocator->AllocateWriteBufferBlks(volumeId,
                remainBlockCount);
        }

        if (IsUnMapVsa(targetVsaRange.startVsa))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::WRHDLR_NO_FREE_SPACE;
            IBOF_TRACE_DEBUG(eventId, IbofEventId::GetString(eventId));

            StateManager* stateManager = StateManagerSingleton::Instance();
            if (unlikely(stateManager->GetState() == State::STOP))
            {
                IBOF_EVENT_ID eventId =
                    IBOF_EVENT_ID::WRHDLR_FAIL_BY_SYSTEM_STOP;
                IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
                throw eventId;
            }
            break;
        }

        _AddVirtualBlks(targetVsaRange);
        remainBlockCount -= targetVsaRange.numBlks;
    }
}

void
WriteSubmission::_AddVirtualBlks(VirtualBlks& virtualBlks)
{
    allocatedBlockCount += virtualBlks.numBlks;
    volumeIo->AddAllocatedVirtualBlks(virtualBlks);
}

VolumeIoSmartPtr
WriteSubmission::_CreateVolumeIo(VirtualBlks& vsaRange)
{
    VirtualBlkAddr startVsa = vsaRange.startVsa;
    Translator translator(startVsa);
    void* mem = volumeIo->GetBuffer();
    uint32_t blockCount = vsaRange.numBlks;
    PhysicalEntries physicalEntries =
        translator.GetPhysicalEntries(mem, blockCount);

    // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
    assert(physicalEntries.size() == 1);

    VolumeIoSmartPtr split =
        volumeIo->Split(ChangeBlockToSector(blockCount), false);
    split->SetVsa(startVsa);

    for (auto& physicalEntry : physicalEntries)
    {
        PhysicalBlkAddr pba = physicalEntry.addr;
        assert(physicalEntry.buffers.size() == 1);

        for (auto& buffer : physicalEntry.buffers)
        {
            split->SetOriginUbio(volumeIo);
            split->SetVsa(startVsa);
            split->SetPba(pba);
            CallbackSmartPtr callback(new BlockMapUpdateRequest(split));
            split->SetCallback(callback);
            StripeAddr lsidEntry = translator.GetLsidEntry(0);
            split->SetLsidEntry(lsidEntry);

            pba.lba += ChangeBlockToSector(buffer.GetBlkCnt());
        }
    }

    return split;
}

void
WriteSubmission::_PrepareSingleBlock(VirtualBlks& vsaRange)
{
    VirtualBlkAddr startVsa = vsaRange.startVsa;
    Translator translator(startVsa);
    void* mem = volumeIo->GetBuffer();
    uint32_t blockCount = vsaRange.numBlks;
    PhysicalEntries physicalEntries =
        translator.GetPhysicalEntries(mem, blockCount);

    // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
    assert(physicalEntries.size() == 1);

    volumeIo->SetVsa(startVsa);

    for (auto& physicalEntry : physicalEntries)
    {
        PhysicalBlkAddr pba = physicalEntry.addr;
        assert(physicalEntry.buffers.size() == 1);

        for (auto& buffer : physicalEntry.buffers)
        {
            volumeIo->SetVsa(startVsa);
            volumeIo->SetPba(pba);

            CallbackSmartPtr callback(new BlockMapUpdateRequest(volumeIo, volumeIo->GetCallback()));

            volumeIo->SetCallback(callback);
            StripeAddr lsidEntry = translator.GetLsidEntry(0);
            volumeIo->SetLsidEntry(lsidEntry);

            pba.lba += ChangeBlockToSector(buffer.GetBlkCnt());
        }
    }
}

} // namespace ibofos
