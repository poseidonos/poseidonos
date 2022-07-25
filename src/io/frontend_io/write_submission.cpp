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

#include <air/Air.h>

#include <mutex>

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
#include "src/include/array_config.h"
#include "src/include/branch_prediction.h"
#include "src/include/meta_const.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/block_map_update_request.h"
#include "src/io/frontend_io/read_completion_for_partial_write.h"
#include "src/io/frontend_io/write_for_parity.h"
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
      ArrayMgr()->GetInfo(volumeIo->GetArrayId())->arrayInfo,
      EventFrameworkApiSingleton::Instance()->IsReactorNow())
{
}

WriteSubmission::WriteSubmission(VolumeIoSmartPtr volumeIo, RBAStateManager* inputRbaStateManager,
    IBlockAllocator* inputIBlockAllocator, FlowControl* inputFlowControl, IArrayInfo* inputArrayInfo,
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
  iBlockAllocator(inputIBlockAllocator),
  flowControl(inputFlowControl),
  arrayInfo(inputArrayInfo)
{
    airlog("RequestedUserWrite", "user", GetEventType(), 1);
    if (nullptr == flowControl)
    {
        flowControl = FlowControlServiceSingleton::Instance()->GetFlowControl(arrayInfo->GetName());
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

VirtualBlkAddrInfo
WriteSubmission::_PopHeadVsa(void)
{
    VirtualBlks& firstVsaRange = allocatedVirtualBlks.front().first;
    VirtualBlkAddr headVsa = firstVsaRange.startVsa;
    StripeId stripeId = allocatedVirtualBlks.front().second;
    firstVsaRange.numBlks--;
    firstVsaRange.startVsa.offset++;

    if (firstVsaRange.numBlks == 0)
    {
        allocatedVirtualBlks.pop_front();
    }

    return {headVsa, stripeId};
}

VirtualBlkAddrInfo
WriteSubmission::_PopTailVsa(void)
{
    VirtualBlks& lastVsaRange = allocatedVirtualBlks.back().first;
    VirtualBlkAddr tailVsa = lastVsaRange.startVsa;
    StripeId stripeId = allocatedVirtualBlks.back().second;
    tailVsa.offset += lastVsaRange.numBlks - 1;
    lastVsaRange.numBlks--;
    if (lastVsaRange.numBlks == 0)
    {
        allocatedVirtualBlks.pop_back();
    }

    return {tailVsa, stripeId};
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
    bool isWTEnabled = arrayInfo->IsWriteThroughEnabled();

    if (false == isWTEnabled)
    {
        // If Read for partial write case, handling device failure is necessary.
        ioDispatcher->Submit(volumeIo, false, isRead);
    }
    else
    {
        if (false == isRead)
        {
            WriteForParity writeForParity(volumeIo);
            bool ret = writeForParity.Execute();
            if (ret == false)
            {
                POS_EVENT_ID eventId = EID(WRITE_FOR_PARITY_FAILED);
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    "Failed to copy user data to dram for parity");
            }
        }
        ioDispatcher->Submit(volumeIo, false, true);
    }
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
    VirtualBlksInfo& virtualBlksInfo = allocatedVirtualBlks.front();
    _PrepareSingleBlock(virtualBlksInfo);
    _SendVolumeIo(volumeIo);
}

void
WriteSubmission::_WriteMultipleBlocks(void)
{
    for (auto& virtualBlksInfo : allocatedVirtualBlks)
    {
        _WriteDataAccordingToVsaRange(virtualBlksInfo);
    }
    _SubmitVolumeIo();
}

void
WriteSubmission::_WriteDataAccordingToVsaRange(VirtualBlksInfo& virtualBlksInfo)
{
    VolumeIoSmartPtr newVolumeIo = _CreateVolumeIo(virtualBlksInfo);

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
    VirtualBlkAddrInfo vsa = _PopHeadVsa();

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
    VirtualBlkAddrInfo vsa = _PopTailVsa();

    _ReadOldBlock(tailRba, vsa, true);
}

void
WriteSubmission::_ReadOldBlock(BlkAddr rba, VirtualBlkAddrInfo& vsaInfo, bool isTail)
{
    VirtualBlkAddr vsa = vsaInfo.first;
    StripeId userLsid = vsaInfo.second;
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
    newVolumeIo->SetUserLsid(userLsid);
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
    bool isWTEnabled = arrayInfo->IsWriteThroughEnabled();
    int remainBlockCount = blockCount - allocatedBlockCount;

    if (!iBlockAllocator->TryRdLock(volumeId))
    {
        return;
    }

    while (remainBlockCount > 0)
    {
        VirtualBlks targetVsaRange;

        uint64_t key = reinterpret_cast<uint64_t>(this) + allocatedBlockCount;
        airlog("LAT_WrSb_AllocWriteBuf", "begin", 0, key);
        auto result = iBlockAllocator->AllocateWriteBufferBlks(volumeId, remainBlockCount);
        targetVsaRange = result.first;
        airlog("LAT_WrSb_AllocWriteBuf", "end", 0, key);

        if (IsUnMapVsa(targetVsaRange.startVsa))
        {
            POS_EVENT_ID eventId = EID(WRHDLR_NO_FREE_SPACE);
            POS_TRACE_DEBUG(eventId, "No free space in write buffer");

            IStateControl* stateControl =
                StateManagerSingleton::Instance()->GetStateControl(arrayInfo->GetName());
            if (unlikely(stateControl->GetState()->ToStateType() == StateEnum::STOP))
            {
                POS_EVENT_ID eventId =
                    EID(WRHDLR_FAIL_BY_SYSTEM_STOP);
                POS_TRACE_ERROR(eventId, "System Stop incurs write fail");
                if (!iBlockAllocator->Unlock(volumeId))
                {
                    POS_EVENT_ID eventId = EID(WRHDLR_FAIL_TO_UNLOCK);
                    POS_TRACE_DEBUG(eventId, "volumeId:{}", volumeId);
                }
                throw eventId;
            }
            break;
        }
        if (true == isWTEnabled)
        {
            VirtualBlkAddr startVsa = targetVsaRange.startVsa;
            uint32_t remainingBlks = targetVsaRange.numBlks;
            while (remainingBlks > 0)
            {
                VirtualBlksInfo info;
                VirtualBlks vsaInfo;
                uint32_t numBlks = ArrayConfig::BLOCKS_PER_CHUNK -
                    (startVsa.offset % ArrayConfig::BLOCKS_PER_CHUNK);
                if (numBlks > remainingBlks)
                {
                    numBlks = remainingBlks;
                }
                vsaInfo.startVsa = startVsa;
                vsaInfo.numBlks = numBlks;
                info.first = vsaInfo;
                info.second = result.second;
                _AddVirtualBlks(info);
                startVsa.offset += numBlks;
                remainingBlks -= numBlks;
            }
        }
        else
        {
            _AddVirtualBlks(result);
        }
        remainBlockCount -= targetVsaRange.numBlks;
    }

    if (!iBlockAllocator->Unlock(volumeId))
    {
        POS_EVENT_ID eventId = EID(WRHDLR_FAIL_TO_UNLOCK);
        POS_TRACE_DEBUG(eventId, "volumeId:{}", volumeId);
    }
}

void
WriteSubmission::_AddVirtualBlks(VirtualBlksInfo& virtualBlks)
{
    allocatedBlockCount += virtualBlks.first.numBlks;
    allocatedVirtualBlks.push_back(virtualBlks);
}

VolumeIoSmartPtr
WriteSubmission::_CreateVolumeIo(VirtualBlksInfo& virtualBlksInfo)
{
    VirtualBlks vsaRange = virtualBlksInfo.first;
    VolumeIoSmartPtr split =
        volumeIo->Split(ChangeBlockToSector(vsaRange.numBlks), false);
    _SetupVolumeIo(split, virtualBlksInfo, nullptr);

    return split;
}

void
WriteSubmission::_SetupVolumeIo(VolumeIoSmartPtr newVolumeIo,
    VirtualBlksInfo& virtualBlksInfo, CallbackSmartPtr callback)
{
    VirtualBlks vsaRange = virtualBlksInfo.first;
    StripeId userLsid = virtualBlksInfo.second;
    VirtualBlkAddr startVsa = vsaRange.startVsa;
    Translator translator(startVsa, volumeIo->GetArrayId(), userLsid);
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
    newVolumeIo->SetUserLsid(userLsid);

    CallbackSmartPtr blockMapUpdateRequest(
        new BlockMapUpdateRequest(newVolumeIo, callback));

    newVolumeIo->SetCallback(blockMapUpdateRequest);
    StripeAddr lsidEntry = translator.GetLsidEntry(0);
    newVolumeIo->SetLsidEntry(lsidEntry);

    pba.lba += ChangeBlockToSector(vsaRange.numBlks);
}

void
WriteSubmission::_PrepareSingleBlock(VirtualBlksInfo& virtualBlksInfo)
{
    _SetupVolumeIo(volumeIo, virtualBlksInfo, volumeIo->GetCallback());
}

} // namespace pos
