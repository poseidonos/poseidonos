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

#include "src/io/frontend_io/write_submission.h"

#include <mutex>

#include "Air.h"
#include "src/admin/smart_log_mgr.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/bio/volume_io.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/event_scheduler/io_completer.h"
#include "src/gc/flow_control/flow_control.h"
#include "src/gc/flow_control/flow_control_service.h"
#include "src/include/address_type.h"
#include "src/include/branch_prediction.h"
#include "src/include/meta_const.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/block_map_update_request.h"
#include "src/io/frontend_io/read_completion_for_partial_write.h"
#include "src/io/general_io/rba_state_service.h"
#include "src/io/general_io/translator.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/state/state_manager.h"

/*To do Remove after adding array Idx by Array*/
#include "src/array_mgmt/array_manager.h"

namespace pos
{
WriteSubmission::WriteSubmission(VolumeIoSmartPtr volumeIo)
: WriteSubmission(volumeIo, RBAStateServiceSingleton::Instance()->GetRBAStateManager(volumeIo->GetArrayId()),
      AllocatorServiceSingleton::Instance()->GetIBlockAllocator(volumeIo->GetArrayId()),
      nullptr,
      EventFrameworkApiSingleton::Instance()->IsReactorNow())
{
}

WriteSubmission::WriteSubmission(VolumeIoSmartPtr volumeIo, RBAStateManager* inputRbaStateManager, IBlockAllocator* inputIBlockAllocator, FlowControl* inputFlowControl,
    bool isReactorNow)
: Event(isReactorNow),
  volumeIo(volumeIo),
  volumeId(volumeIo->GetVolumeId()),
  blockAlignment(ChangeSectorToByte(volumeIo->GetSectorRba()),
      volumeIo->GetSize()),
  blockCount(blockAlignment.GetBlockCount()),
  allocatedBlockCount(0),
  processedBlockCount(0),
  rbaStateManager(inputRbaStateManager),
  iBlockAllocator(inputIBlockAllocator)
{
    flowControl = inputFlowControl;
    if (nullptr == flowControl)
    {
        /*To do Remove after adding array Idx by Array*/
        IArrayInfo* info = ArrayMgr()->GetInfo(volumeIo->GetArrayId())->arrayInfo;

        flowControl = FlowControlServiceSingleton::Instance()->GetFlowControl(info->GetName());
    }
}

WriteSubmission::~WriteSubmission(void)
{
}

bool
WriteSubmission::Execute(void)
{
    try
    {
        int token = flowControl->GetToken(FlowControlType::USER, blockCount);
        if (0 >= token)
        {
            return false;
        }

        BlkAddr startRba = blockAlignment.GetHeadBlock();
        bool ownershipAcquired = rbaStateManager->BulkAcquireOwnership(volumeId,
            startRba, blockCount);
        if (false == ownershipAcquired)
        {
            if (0 < token)
            {
                flowControl->ReturnToken(FlowControlType::USER, token);
            }
            return false;
        }
        bool done = _ProcessOwnedWrite();

        if (unlikely(!done))
        {
            rbaStateManager->BulkReleaseOwnership(volumeId, startRba,
                blockCount);
        }
        else
        {
            uint32_t arrayId = volumeIo->GetArrayId();
            SmartLogMgrSingleton::Instance()->IncreaseWriteBytes(blockCount, volumeId, arrayId);
            SmartLogMgrSingleton::Instance()->IncreaseWriteCmds(volumeId, arrayId);
            volumeIo = nullptr;
        }

        return done;
    }
    catch (...)
    {
        if (nullptr != volumeIo)
        {
            IoCompleter ioCompleter(volumeIo);
            ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::GENERIC_ERROR, true);
        }
        volumeIo = nullptr;
        return true;
    }
}

VirtualBlkAddr
WriteSubmission::_PopHeadVsa(void)
{
    VirtualBlks& firstVsaRange = allocatedVirtualBlks.front();
    VirtualBlkAddr headVsa = firstVsaRange.startVsa;
    firstVsaRange.numBlks--;
    firstVsaRange.startVsa.offset++;

    if (firstVsaRange.numBlks == 0)
    {
        allocatedVirtualBlks.pop_front();
    }

    return headVsa;
}
VirtualBlkAddr
WriteSubmission::_PopTailVsa(void)
{
    VirtualBlks& lastVsaRange = allocatedVirtualBlks.back();
    VirtualBlkAddr tailVsa = lastVsaRange.startVsa;
    tailVsa.offset += lastVsaRange.numBlks - 1;
    lastVsaRange.numBlks--;
    if (lastVsaRange.numBlks == 0)
    {
        allocatedVirtualBlks.pop_back();
    }

    return tailVsa;
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
    }
    else
    {
        _WriteMultipleBlocks();
    }

    return true;
}

void
WriteSubmission::_SendVolumeIo(VolumeIoSmartPtr volumeIo)
{
    bool isRead = (volumeIo->dir == UbioDir::Read);
    // If Read for partial write case, handling device failure is necessary.
    ioDispatcher->Submit(volumeIo, false, isRead);
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
            IoCompleter ioCompleter(volumeIo);
            ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
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
    VirtualBlks& virtualBlks = allocatedVirtualBlks.front();
    _PrepareSingleBlock(virtualBlks);
    _SendVolumeIo(volumeIo);
}

void
WriteSubmission::_WriteMultipleBlocks(void)
{
    for (auto& virtualBlks : allocatedVirtualBlks)
    {
        _WriteDataAccordingToVsaRange(virtualBlks);
    }
    _SubmitVolumeIo();
}

void
WriteSubmission::_WriteDataAccordingToVsaRange(VirtualBlks& vsaRange)
{
    VolumeIoSmartPtr newVolumeIo = _CreateVolumeIo(vsaRange);

    splitVolumeIoQueue.push(newVolumeIo);
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
    VirtualBlkAddr vsa = _PopHeadVsa();

    _ReadOldBlock(headRba, vsa, false);
}

void
WriteSubmission::_ReadOldTailBlock(void)
{
    if (processedBlockCount == blockCount)
    {
        return;
    }

    BlkAddr tailRba = blockAlignment.GetTailBlock();
    VirtualBlkAddr vsa = _PopTailVsa();

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

    VolumeIoSmartPtr newVolumeIo(new VolumeIo(nullptr, Ubio::UNITS_PER_BLOCK, volumeIo->GetArrayId()));

    newVolumeIo->SetVolumeId(volumeId);
    newVolumeIo->SetOriginUbio(split);
    CallbackSmartPtr event(new ReadCompletionForPartialWrite(newVolumeIo,
        alignmentSize, alignmentOffset));
    newVolumeIo->SetCallback(event);
    uint64_t sectorRba = ChangeBlockToSector(rba);
    newVolumeIo->SetSectorRba(sectorRba);
    newVolumeIo->SetVsa(vsa);

    const bool isRead = true;
    Translator oldDataTranslator(volumeId, rba, volumeIo->GetArrayId(), isRead);
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

    while (remainBlockCount > 0)
    {
        VirtualBlks targetVsaRange;

        uint64_t key = reinterpret_cast<uint64_t>(this) + allocatedBlockCount;
        airlog("LAT_WrSb_AllocWriteBuf", "AIR_BEGIN", 0, key);
        targetVsaRange = iBlockAllocator->AllocateWriteBufferBlks(volumeId,
            remainBlockCount);
        airlog("LAT_WrSb_AllocWriteBuf", "AIR_END", 0, key);

        if (IsUnMapVsa(targetVsaRange.startVsa))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::WRHDLR_NO_FREE_SPACE;
            POS_TRACE_DEBUG(eventId, "No free space in write buffer");

            /*To do Remove after adding array Idx by Array*/
            IArrayInfo* info = ArrayMgr()->GetInfo(volumeIo->GetArrayId())->arrayInfo;

            IStateControl* stateControl =
                StateManagerSingleton::Instance()->GetStateControl(info->GetName());
            if (unlikely(stateControl->GetState()->ToStateType() == StateEnum::STOP))
            {
                POS_EVENT_ID eventId =
                    POS_EVENT_ID::WRHDLR_FAIL_BY_SYSTEM_STOP;
                POS_TRACE_ERROR(eventId, "System Stop incurs write fail");
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
    allocatedVirtualBlks.push_back(virtualBlks);
}

VolumeIoSmartPtr
WriteSubmission::_CreateVolumeIo(VirtualBlks& vsaRange)
{
    VolumeIoSmartPtr split =
        volumeIo->Split(ChangeBlockToSector(vsaRange.numBlks), false);
    _SetupVolumeIo(split, vsaRange, nullptr);

    return split;
}

void
WriteSubmission::_SetupVolumeIo(VolumeIoSmartPtr newVolumeIo,
    VirtualBlks& vsaRange, CallbackSmartPtr callback)
{
    VirtualBlkAddr startVsa = vsaRange.startVsa;
    Translator translator(startVsa, volumeIo->GetArrayId());
    void* mem = newVolumeIo->GetBuffer();
    list<PhysicalEntry> physicalEntries =
        translator.GetPhysicalEntries(mem, vsaRange.numBlks);

    // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
    assert(physicalEntries.size() == 1);

    newVolumeIo->SetVsa(startVsa);
    PhysicalBlkAddr pba = physicalEntries.front().addr;
    if (volumeIo != newVolumeIo)
    {
        newVolumeIo->SetOriginUbio(volumeIo);
    }
    newVolumeIo->SetVsa(startVsa);
    newVolumeIo->SetPba(pba);

    CallbackSmartPtr blockMapUpdateRequest(
        new BlockMapUpdateRequest(newVolumeIo, callback));

    newVolumeIo->SetCallback(blockMapUpdateRequest);
    StripeAddr lsidEntry = translator.GetLsidEntry(0);
    newVolumeIo->SetLsidEntry(lsidEntry);

    pba.lba += ChangeBlockToSector(vsaRange.numBlks);
}

void
WriteSubmission::_PrepareSingleBlock(VirtualBlks& vsaRange)
{
    _SetupVolumeIo(volumeIo, vsaRange, volumeIo->GetCallback());
}

} // namespace pos
