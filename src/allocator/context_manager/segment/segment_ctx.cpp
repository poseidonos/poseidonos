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

#include <mutex>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/io_ctx/allocator_context_io_ctx.h"
#include "src/allocator/context_manager/segment/segment_ctx.h"
#include "src/allocator/context_manager/segment/segment_info.h"
#include "src/allocator/context_manager/segment/segment_states.h"
#include "src/include/pos_event_id.h"
#include "src/include/meta_const.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{

SegmentCtx::SegmentCtx(BitMapMutex* segmentBitmap_, StripeId prevSsdLsid_, StripeId currentSsdLsid_,
                       SegmentStates* segmentStates_, SegmentInfo* segmentInfo_, uint32_t versionSegInfo_,
                       uint32_t headerSize_, uint32_t writeBufferSize_, uint32_t thresholdSegments_,
                       uint32_t urgentSegments_, std::string arrayName_, AllocatorAddressInfo* addrInfo_,
                       MetaFileIntf* segInfoFile_, IRebuildCtxInternal* iRebuildCtxInternal_)
: segmentBitmap(segmentBitmap_),
  prevSsdLsid(prevSsdLsid_),
  currentSsdLsid(currentSsdLsid_),
  segmentStates(segmentStates_),
  segmentInfos(segmentInfo_),
  versionSegInfo(versionSegInfo_),
  headerSize(headerSize_),
  writeBufferSize(writeBufferSize_),
  thresholdSegments(thresholdSegments_),
  urgentSegments(urgentSegments_),
  arrayName(arrayName_),
  addrInfo(addrInfo_),
  segInfoFile(segInfoFile_),
  iRebuildCtxInternal(iRebuildCtxInternal_)
{
}

SegmentCtx::SegmentCtx(AllocatorAddressInfo* info, std::string arrayName_)
: SegmentCtx(nullptr, UNMAP_STRIPE, UNMAP_STRIPE, nullptr, nullptr, UINT32_MAX, sizeof(SegInfoHeader), UINT32_MAX, 20, 5, arrayName_, info, nullptr, nullptr)
{
}

SegmentCtx::~SegmentCtx(void)
{
}

void
SegmentCtx::Init(void)
{
    if (segmentBitmap != nullptr)   // DoCs already inserted
    {
        return;
    }
    uint32_t numSegment = addrInfo->GetnumUserAreaSegments();
    segmentBitmap = new BitMapMutex(numSegment);
    currentSsdLsid = STRIPES_PER_SEGMENT - 1;

    segmentStates = new SegmentStates[numSegment];
    for (uint32_t segmentId = 0; segmentId < numSegment; ++segmentId)
    {
        segmentStates[segmentId].SetSegmentId(segmentId);
    }

    segmentInfos = new SegmentInfo(numSegment);
    writeBufferSize = headerSize + (numSegment * sizeof(uint32_t) * 2);

    if (segInfoFile == nullptr)
    {
        segInfoFile = new FILESTORE("SegmentInfoContext", arrayName);
    }
    else
    {
        POS_TRACE_ERROR(EID(ERROR_REINIT_WITHOUT_DISPOSE), "failed to initalize segInfofile object!!!");
        assert(false);
    }

    if (segInfoFile->DoesFileExist() == false)
    {
        segInfoFile->Create(writeBufferSize);
        segInfoFile->Open();
        versionSegInfo = 0;
        StoreSegmentInfoSync();
    }
    else
    {
        segInfoFile->Open();
        _LoadSegmentInfoSync();
    }
}

void
SegmentCtx::Close(void)
{
    if (segInfoFile != nullptr)
    {
        if (segInfoFile->IsOpened() == true)
        {
            segInfoFile->Close();
        }
        delete segInfoFile;
        segInfoFile = nullptr;
    }

    delete[] segmentStates;
    segmentStates = nullptr;
    delete segmentBitmap;
    segmentBitmap = nullptr;
    delete segmentInfos;
    segmentInfos = nullptr;
}

int
SegmentCtx::StoreSegmentInfoSync(void)
{
    char* bufferInObj = new char[writeBufferSize];
    _HeaderUpdate(bufferInObj);
    segmentInfos->CopySegmentInfoToBuffer((bufferInObj + headerSize));
    int ret = segInfoFile->IssueIO(MetaFsIoOpcode::Write, 0, writeBufferSize, bufferInObj);
    if (ret != 0)
    {
        POS_TRACE_ERROR(POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_STORE, "Failed To Store SegmentInfoContext File:{}", ret);
    }

    delete[] bufferInObj;
    bufferInObj = nullptr;
    return ret;
}

void
SegmentCtx::_HeaderUpdate(char* pBuf)
{
    SegInfoHeader* pHeader = reinterpret_cast<SegInfoHeader*>(pBuf);
    pHeader->segInfoSig = SIG_SEGMENT_INFO;
    pHeader->segInfoVersion = versionSegInfo;
    versionSegInfo++;
}

void
SegmentCtx::_HeaderLoaded(char* pBuf)
{
    SegInfoHeader* pHeader = reinterpret_cast<SegInfoHeader*>(pBuf);
    assert(pHeader->segInfoSig == SIG_SEGMENT_INFO);
    versionSegInfo = pHeader->segInfoVersion;
    versionSegInfo++;
}

AllocatorContextIoCtx*
SegmentCtx::StoreSegmentInfoAsync(MetaIoCbPtr callback)
{
    char* bufferInObj = new char[writeBufferSize];
    _HeaderUpdate(bufferInObj);
    segmentInfos->CopySegmentInfoToBuffer((bufferInObj + headerSize));
    AllocatorContextIoCtx* flushRequest = new AllocatorContextIoCtx(MetaFsIoOpcode::Write,
        segInfoFile->GetFd(), 0, writeBufferSize, bufferInObj,
        callback);
    return flushRequest;
}

void
SegmentCtx::ReleaseRequestIo(AsyncMetaFileIoCtx* ctx)
{
    delete[] ctx->buffer;
    ctx->buffer = nullptr;
    delete ctx;
    ctx = nullptr;
}

bool
SegmentCtx::IsSegmentInfoRequestIo(char* pBuf)
{
    SegInfoHeader* pHeader = reinterpret_cast<SegInfoHeader*>(pBuf);
    if (pHeader->segInfoSig == SIG_SEGMENT_INFO)
    {
        return true;
    }
    return false;
}

int
SegmentCtx::_LoadSegmentInfoSync(void)
{
    char* bufferInObj = new char[writeBufferSize];
    int ret = segInfoFile->IssueIO(MetaFsIoOpcode::Read, 0, writeBufferSize, bufferInObj);
    if (ret == 0)
    {
        segmentInfos->CopySegmentInfoFromBuffer((bufferInObj + headerSize));
        _HeaderLoaded(bufferInObj);
    }
    else
    {
        POS_TRACE_ERROR(POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_LOAD, "Failed To Load Segment InvalidCountPool:{}", ret);
    }

    delete[] bufferInObj;
    bufferInObj = nullptr;
    return ret;
}

uint64_t
SegmentCtx::GetNumOfFreeUserDataSegment(void)
{
    return segmentBitmap->GetNumBits() - segmentBitmap->GetNumBitsSet();
}

SegmentId
SegmentCtx::GetGCVictimSegment(void)
{
    uint32_t numUserAreaSegments = addrInfo->GetnumUserAreaSegments();
    SegmentId victimSegment = UNMAP_SEGMENT;
    uint32_t minValidCount = addrInfo->GetblksPerSegment();

    for (SegmentId id = 0; id < numUserAreaSegments; ++id)
    {
        std::lock_guard<std::mutex> lock(segmentStates[id].GetSegmentLock());
        uint32_t cnt = segmentInfos->GetValidBlockCount(id);
        if ((segmentStates[id].Getstate() != SegmentState::SSD) || (cnt == 0))
        {
            continue;
        }

        if (cnt < minValidCount)
        {
            victimSegment = id;
            minValidCount = cnt;
        }
    }

    if (victimSegment != UNMAP_SEGMENT)
    {
        UsedSegmentStateChange(victimSegment, SegmentState::VICTIM);
    }
    return victimSegment;
}

void
SegmentCtx::ReplaySsdLsid(StripeId currentSsdLsid)
{
    SetCurrentSsdLsid(currentSsdLsid);
}

void
SegmentCtx::UpdateOccupiedStripeCount(StripeId lsid)
{
    SegmentId segId = lsid / addrInfo->GetstripesPerSegment();
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    if (segmentInfos->IncreaseOccupiedStripeCount(segId) == addrInfo->GetstripesPerSegment())
    {
        if (segmentInfos->GetValidBlockCount(segId) == 0)
        {
            SegmentState eState = segmentStates[segId].Getstate();
            if (eState != SegmentState::FREE)
            {
                _FreeSegment(segId);
            }
        }
        else
        {
            segmentStates[segId].Setstate(SegmentState::SSD);
        }
    }
}

void
SegmentCtx::FreeUserDataSegment(SegmentId segId)
{
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    SegmentState eState = segmentStates[segId].Getstate();
    if ((eState == SegmentState::SSD) || (eState == SegmentState::VICTIM))
    {
        assert(segmentInfos->GetOccupiedStripeCount(segId) == addrInfo->GetstripesPerSegment());
        _FreeSegment(segId);
    }
}

void
SegmentCtx::_FreeSegment(SegmentId segId)
{
    segmentInfos->SetOccupiedStripeCount(segId, 0 /* count */);
    segmentStates[segId].Setstate(SegmentState::FREE);
    segmentBitmap->ClearBit(segId);
    POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was freed by allocator", segId);
    iRebuildCtxInternal->FreeSegmentInRebuildTarget(segId);
}

void
SegmentCtx::ReplaySegmentAllocation(StripeId userLsid)
{
    if (userLsid % addrInfo->GetstripesPerSegment() == 0)
    {
        SegmentId segmentId = userLsid / addrInfo->GetstripesPerSegment();
        std::lock_guard<std::mutex> lock(segmentStates[segmentId].GetSegmentLock());
        if (segmentStates[segmentId].Getstate() == SegmentState::FREE)
        {
            segmentStates[segmentId].Setstate(SegmentState::NVRAM);
            segmentBitmap->SetBit(segmentId);
            POS_TRACE_DEBUG((int)POS_EVENT_ID::JOURNAL_REPLAY_STATUS, "SegmentId:{} is allocated", segmentId);
        }
    }
}

StripeId
SegmentCtx::GetPrevSsdLsid(void)
{
    return prevSsdLsid;
}

void
SegmentCtx::SetPrevSsdLsid(StripeId stripeId)
{
    prevSsdLsid = stripeId;
}

StripeId
SegmentCtx::GetCurrentSsdLsid(void)
{
    return currentSsdLsid;
}

void
SegmentCtx::SetCurrentSsdLsid(StripeId stripeId)
{
    currentSsdLsid = stripeId;
}

SegmentStates&
SegmentCtx::GetSegmentState(SegmentId segmentId)
{
    return segmentStates[segmentId];
}

void
SegmentCtx::UsedSegmentStateChange(SegmentId segmentId, SegmentState state)
{
    std::lock_guard<std::mutex> lock(segmentStates[segmentId].GetSegmentLock());
    segmentStates[segmentId].Setstate(state);
}

uint32_t
SegmentCtx::IncreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    uint32_t blksPerSegment = addrInfo->GetblksPerSegment();
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    uint32_t validCount = segmentInfos->IncreaseValidBlockCount(segId, cnt);
    if (validCount > blksPerSegment)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_OVERFLOWED), "segmentId:{} increasedCount:{} total validCount:{} : OVERFLOWED", segId, cnt, validCount);
        assert(false);
    }
    return validCount;
}

int32_t
SegmentCtx::DecreaseValidBlockCount(SegmentId segId, uint32_t cnt)
{
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    int32_t validCount = segmentInfos->DecreaseValidBlockCount(segId, cnt);
    if (validCount < 0)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED), "segmentId:{} decreasedCount:{} total validCount:{} : UNDERFLOWED", segId, cnt, validCount);
        assert(false);
    }
    return validCount;
}

uint32_t
SegmentCtx::GetValidBlockCount(SegmentId segId)
{
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    return segmentInfos->GetValidBlockCount(segId);
}

bool
SegmentCtx::CheckSegmentState(SegmentId segId, SegmentState state)
{
    std::lock_guard<std::mutex> lock(segmentStates[segId].GetSegmentLock());
    return (segmentStates[segId].Getstate() == state);
}

void
SegmentCtx::ResetExVictimSegment(void)
{
    for (uint32_t segmentId = 0; segmentId < addrInfo->GetnumUserAreaSegments(); ++segmentId)
    {
        if (CheckSegmentState(segmentId, SegmentState::VICTIM))
        {
            UsedSegmentStateChange(segmentId, SegmentState::SSD);
            POS_TRACE_INFO(EID(SEGMENT_WAS_VICTIM), "segmentId:{} was VICTIM, so changed to SSD", segmentId);
        }
    }
}

void
SegmentCtx::FreeAllInvalidatedSegment(void)
{
    for (uint32_t segmentId = 0; segmentId < addrInfo->GetnumUserAreaSegments(); ++segmentId)
    {
        std::lock_guard<std::mutex> lock(segmentStates[segmentId].GetSegmentLock());
        if ((GetSegmentState(segmentId).Getstate() == SegmentState::SSD) && (segmentInfos->GetValidBlockCount(segmentId) == 0))
        {
            segmentStates[segmentId].Setstate(SegmentState::FREE);
            POS_TRACE_INFO(EID(ALLOCATOR_SEGMENT_FREED), "segmentId:{} was All Invalidated, so changed to FREE", segmentId);
        }
    }
}

uint32_t
SegmentCtx::GetNumSegment(void)
{
    return segmentInfos->GetNumSegment();
}

char*
SegmentCtx::GetCtxSectionInfo(AllocatorCtxType type, int& sectionSize)
{
    char* ret = nullptr;
    switch (type)
    {
        case CURRENT_SSD_LSID:
            sectionSize = sizeof(currentSsdLsid);
            ret = reinterpret_cast<char*>(&currentSsdLsid);
            break;
        case SEGMENT_STATES:
            sectionSize = sizeof(SegmentStates) * segmentInfos->GetNumSegment();
            ret = reinterpret_cast<char*>(segmentStates);
            break;
        default:
            assert(false);
            break;
    }
    return ret;
}

void
SegmentCtx::SetIRebuildCtxInternal(IRebuildCtxInternal* irebuildCtxInternal)
{
    iRebuildCtxInternal = irebuildCtxInternal;
}

} // namespace pos
