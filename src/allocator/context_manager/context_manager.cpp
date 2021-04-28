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

#include "src/allocator/context_manager/context_manager.h"

#include <vector>
#include <string>

#include "src/allocator/context_manager/io_ctx/allocator_context_io_ctx.h"
#include "src/allocator/context_manager/segment/segment_info.h"
#include "src/allocator/include/allocator_const.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/metafs/metafs_file_intf.h"

namespace pos
{
ContextManager::ContextManager(AllocatorAddressInfo* info, std::string arrayName)
: ctxFile(nullptr),
  userBlkAllocProhibited(false),
  ctxStoredVersion(INVALID_VERSION),
  ctxDirtyVersion(INVALID_VERSION),
  wbLsidBitmap(nullptr),
  flushInProgress(false),
  addrInfo(info),
  numAsyncIoIssued(0),
  arrayName(arrayName)
{
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        blkAllocProhibited[volumeId] = false;
    }

    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        activeStripeTail[asTailArrayIdx] = UNMAP_VSA;
    }

    segmentCtx = new SegmentCtx(info, arrayName);
    rebuildCtx = new RebuildCtx(segmentCtx, this, arrayName);
    segmentCtx->SetIRebuildCtxInternal(rebuildCtx);
}

ContextManager::~ContextManager(void)
{
    delete segmentCtx;
    delete rebuildCtx;
}

void
ContextManager::Init(void)
{
    segmentCtx->Init();
    rebuildCtx->Init(addrInfo);

    wbLsidBitmap = new BitMapMutex(addrInfo->GetnumWbStripes());

    _UpdateCtxList();

    if (ctxFile == nullptr)
    {
        ctxFile = new FILESTORE("AllocatorContexts", arrayName);
    }

    if (ctxFile->DoesFileExist() == false)
    {
        ctxFile->Create(ctxHeader.totalSize);
        ctxFile->Open();
        ctxDirtyVersion = 0; // For the first time,
        StoreAllocatorCtxs();
    }
    else
    {
        ctxFile->Open();
        _LoadSync();
        ctxDirtyVersion = ctxStoredVersion + 1;
    }
}

void
ContextManager::ReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    wbLsidBitmap->SetBit(wbLsid);
}

void
ContextManager::ReplayStripeFlushed(StripeId wbLsid)
{
    wbLsidBitmap->ClearBit(wbLsid);
}

std::vector<VirtualBlkAddr>
ContextManager::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> asTails;
    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        asTails.push_back(GetActiveStripeTail(asTailArrayIdx));
    }
    return asTails;
}

void
ContextManager::ResetActiveStripeTail(int index)
{
    SetActiveStripeTail(index, UNMAP_VSA);
}

void
ContextManager::Close(void)
{
    if (ctxFile != nullptr)
    {
        if (ctxFile->IsOpened() == true)
        {
            ctxFile->Close();
        }
        delete ctxFile;
        ctxFile = nullptr;
    }

    segmentCtx->Close();
    rebuildCtx->Close();

    delete wbLsidBitmap;
}

int
ContextManager::FlushAllocatorCtxs(EventSmartPtr callback)
{
    char* data;
    {
        std::lock_guard<std::mutex> lock(ctxLock);
        data = _GetCopiedCtxBuffer();
    }

    for (int index = 0; index < ACTIVE_STRIPE_TAIL_ARRAYLEN; index++)
    {
        std::unique_lock<std::mutex> volLock(activeStripeTailLock[index]);
        _CopyWbufTail(data, index);
    }

    return _Flush(data, callback);
}

int
ContextManager::StoreAllocatorCtxs(void)
{
    int ret = segmentCtx->StoreSegmentInfoSync();
    if (ret != 0)
    {
        return ret;
    }

    char* buffer = new char[ctxHeader.totalSize]();
    _PrepareCtxsStore(buffer);
    ret = ctxFile->IssueIO(MetaFsIoOpcode::Write, 0, ctxHeader.totalSize, buffer);
    if (ret == 0)
    {
        ctxStoredVersion = ctxHeader.ctxVersion;
    }
    delete[] buffer;
    return ret;
}

uint64_t
ContextManager::GetAllocatorCtxsStoredVersion(void)
{
    return ctxStoredVersion;
}

void
ContextManager::ResetAllocatorCtxsDirtyVersion(void)
{
    ctxDirtyVersion = 0;
}

bool
ContextManager::TurnOffVolumeBlkAllocation(uint32_t volumeId)
{
    return (blkAllocProhibited[volumeId].exchange(true) == false);
}

void
ContextManager::TurnOnVolumeBlkAllocation(uint32_t volumeId)
{
    blkAllocProhibited[volumeId] = false;
}

void
ContextManager::TurnOffBlkAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        while (blkAllocProhibited[i].exchange(true) == true)
        {
            // Wait for flag to be reset
        }
    }
}

void
ContextManager::TurnOnBlkAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        blkAllocProhibited[i] = false;
    }
}

void
ContextManager::SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa)
{
    activeStripeTail[asTailArrayIdx] = vsa;
}

std::mutex&
ContextManager::GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTailLock[asTailArrayIdx];
}

VirtualBlkAddr
ContextManager::GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTail[asTailArrayIdx];
}

SegmentId
ContextManager::AllocateUserDataSegmentId(void)
{
    SegmentId segmentId = segmentCtx->GetSegmentBitmap()->SetNextZeroBit();

    // This 'segmentId' should not be a target of rebuilding
    while (segmentCtx->GetSegmentBitmap()->IsValidBit(segmentId) && (rebuildCtx->FindRebuildTargetSegment(segmentId) != rebuildCtx->RebuildTargetSegmentsEnd()))
    {
        POS_TRACE_DEBUG(EID(ALLOCATOR_REBUILDING_SEGMENT), "segmentId:{} is already rebuild target!", segmentId);
        segmentCtx->GetSegmentBitmap()->ClearBit(segmentId);
        ++segmentId;
        segmentId = segmentCtx->GetSegmentBitmap()->SetFirstZeroBit(segmentId);
    }

    // In case of all segments are used
    if (segmentCtx->GetSegmentBitmap()->IsValidBit(segmentId) == false)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT), "Free segmentId exhausted, segmentId:{}", segmentId);
        return UNMAP_SEGMENT;
    }

    POS_TRACE_INFO(EID(ALLOCATOR_START), "segmentId:{} @AllocateUserDataSegmentId", segmentId);
    return segmentId;
}

int
ContextManager::SetNextSsdLsid(void)
{
    std::unique_lock<std::mutex> lock(ctxLock);
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "@SetNextSsdLsid");

    SegmentId newSegmentId = AllocateUserDataSegmentId();
    if (newSegmentId == UNMAP_SEGMENT)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_NO_FREE_SEGMENT), "Free segmentId exhausted");
        return -EID(ALLOCATOR_NO_FREE_SEGMENT);
    }

    segmentCtx->SetPrevSsdLsid(segmentCtx->GetCurrentSsdLsid());
    segmentCtx->SetCurrentSsdLsid(newSegmentId * addrInfo->GetstripesPerSegment());
    segmentCtx->UsedSegmentStateChange(newSegmentId, SegmentState::NVRAM);

    return 0;
}
//----------------------------------------------------------------------------//
void
ContextManager::_LoadSync(void)
{
    char* buffer = new char[ctxHeader.totalSize]();
    ctxFile->IssueIO(MetaFsIoOpcode::Read, 0, ctxHeader.totalSize, buffer);
    _CtxLoaded(buffer);
    delete[] buffer;
}

void
ContextManager::_CtxLoaded(char* buffer)
{
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        memcpy(ctxsList[count].addr, buffer + ctxsList[count].offset, ctxsList[count].size);
    }
    segmentCtx->ResetExVictimSegment();
    _HeaderLoaded();
}

int
ContextManager::_HeaderLoaded(void)
{
    ctxStoredVersion = ctxHeader.ctxVersion;
    wbLsidBitmap->SetNumBitsSet(ctxHeader.numValidWbLsid);
    segmentCtx->GetSegmentBitmap()->SetNumBitsSet(ctxHeader.numValidSegment);
    return 0;
}

int
ContextManager::_HeaderUpdate(void)
{
    ctxHeader.ctxVersion = ctxDirtyVersion++;
    ctxHeader.numValidWbLsid = wbLsidBitmap->GetNumBitsSet();
    ctxHeader.numValidSegment = segmentCtx->GetSegmentBitmap()->GetNumBitsSet();
    return 0;
}

void
ContextManager::_PrepareCtxsStore(char* buffer)
{
    _HeaderUpdate();
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        assert(ctxHeader.totalSize > ctxsList[count].offset);
        memcpy(buffer + ctxsList[count].offset, ctxsList[count].addr, ctxsList[count].size);
    }
}

char*
ContextManager::_GetCopiedCtxBuffer(void)
{
    _HeaderUpdate();
    char* data = new char[ctxHeader.totalSize]();

    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        if (count == ACTIVE_STRIPE_TAIL)
        {
            // to update wbuf tail seperately with seperated lock
            continue;
        }
        else if (count == WB_LSID_BITMAP)
        {
            std::lock_guard<std::mutex> lock(wbLsidBitmap->GetLock());
            _FillBuffer(data, count);
        }
        else
        {
            _FillBuffer(data, count);
        }
    }

    return data;
}

void
ContextManager::_CopyWbufTail(char* data, int index)
{
    int offset = sizeof(VirtualBlkAddr) * index;
    memcpy(data + ctxsList[ACTIVE_STRIPE_TAIL].offset + offset, ctxsList[ACTIVE_STRIPE_TAIL].addr + offset, sizeof(VirtualBlkAddr));
}

int
ContextManager::_Flush(char* data, EventSmartPtr callbackEvent)
{
    if (flushInProgress.exchange(true) == true)
    {
        return (int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS;
    }

    numAsyncIoIssued = 0;
    flushCallback = callbackEvent;
    AllocatorContextIoCtx* flushInvCntRequest = segmentCtx->StoreSegmentInfoAsync(
        std::bind(&ContextManager::_FlushCompletedThenCB, this, std::placeholders::_1));
    int ret = ctxFile->AsyncIO(flushInvCntRequest);
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(FAILED_TO_ISSUE_ASYNC_METAIO), "Failed to issue AsyncMetaIo(SegmentInfoContext):{}", ret);
        segmentCtx->ReleaseRequestIo(flushInvCntRequest);
        return ret;
    }
    numAsyncIoIssued++;

    AllocatorContextIoCtx* flushRequest = new AllocatorContextIoCtx(MetaFsIoOpcode::Write,
        ctxFile->GetFd(), 0, ctxHeader.totalSize, data,
        std::bind(&ContextManager::_FlushCompletedThenCB, this, std::placeholders::_1));
    ret = ctxFile->AsyncIO(flushRequest);
    numAsyncIoIssued++;
    return ret;
}

void
ContextManager::_UpdateCtxList(void)
{
    int size;
    ctxsList[HEADER].addr = (char*)&ctxHeader;
    ctxsList[HEADER].size = sizeof(ctxHeader);

    ctxsList[WB_LSID_BITMAP].addr = (char*)(wbLsidBitmap->GetMapAddr());
    ctxsList[WB_LSID_BITMAP].size = wbLsidBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE;

    ctxsList[SEGMENT_BITMAP].addr = (char*)segmentCtx->GetSegmentBitmap()->GetMapAddr();
    ctxsList[SEGMENT_BITMAP].size = segmentCtx->GetSegmentBitmap()->GetNumEntry() * BITMAP_ENTRY_SIZE;

    ctxsList[ACTIVE_STRIPE_TAIL].addr = (char*)activeStripeTail;
    ctxsList[ACTIVE_STRIPE_TAIL].size = sizeof(activeStripeTail);

    ctxsList[CURRENT_SSD_LSID].addr = segmentCtx->GetCtxSectionInfo(CURRENT_SSD_LSID, size);
    ctxsList[CURRENT_SSD_LSID].size = size;

    ctxsList[SEGMENT_STATES].addr = segmentCtx->GetCtxSectionInfo(SEGMENT_STATES, size);
    ctxsList[SEGMENT_STATES].size = size;

    int currentOffset = 0;
    ctxHeader.totalSize = 0;
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        ctxsList[count].offset = currentOffset;
        currentOffset += ctxsList[count].size;
        ctxHeader.totalSize += ctxsList[count].size;
    }
}

void
ContextManager::_FlushCompletedThenCB(AsyncMetaFileIoCtx* ctx)
{
    if (segmentCtx->IsSegmentInfoRequestIo(ctx->buffer) == true)
    {
        segmentCtx->ReleaseRequestIo(reinterpret_cast<AllocatorContextIoCtx*>(ctx));
    }
    else
    {
        assert(ctx->length == ctxHeader.totalSize);
        ctxStoredVersion = ctxHeader.ctxVersion;
        delete[] ctx->buffer;
        delete ctx;
    }

    assert(numAsyncIoIssued > 0);
    numAsyncIoIssued--;
    if (numAsyncIoIssued == 0)
    {
        POS_TRACE_DEBUG((int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_STORE, "allocatorCtx meta file Flushed");
        flushInProgress = false;
        EventSchedulerSingleton::Instance()->EnqueueEvent(flushCallback);
    }
}

void
ContextManager::_FillBuffer(char* buffer, int count)
{
    assert(ctxHeader.totalSize > ctxsList[count].offset);
    memcpy(buffer + ctxsList[count].offset, ctxsList[count].addr, ctxsList[count].size);
}

} // namespace pos
