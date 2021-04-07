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

#include "allocator_meta_archive.h"

#include "segment_info.h"
#include "src/include/meta_const.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"
#ifndef IBOF_CONFIG_USE_MOCK_FS
#include "src/metafs/mfs_file_intf.h"
#endif
#include <string>
#include <utility>

#include "src/scheduler/event_argument.h"

namespace ibofos
{
AllocatorMetaArchive::AllocatorMetaArchive(AllocatorAddressInfo& info)
: wbLsidBitmap(nullptr),
  segmentBitmap(nullptr),
  bufferInObj(nullptr),
  flushInProgress(false),
  prevSsdLsid(UINT32_MAX),
  needRebuildCont(false)
{
    _InitMetadata(info);
    _UpdateMetaList(info);

    allocatorMetaFile = new FILESTORE("AllocatorMeta");
    if (allocatorMetaFile->DoesFileExist() == false)
    {
        allocatorMetaFile->Create(allocatorMetaHeader.totalSize);
        allocatorMetaFile->Open();
        StoreSync();
    }
    else
    {
        allocatorMetaFile->Open();
        LoadSync();
    }

    rebuildSegmentsFile = new FILESTORE("AllocatorRebuildSegments");
    if (rebuildSegmentsFile->DoesFileExist() == false)
    {
        rebuildSegmentsFile->Create(sizeof(targetSegmentCnt) + sizeof(SegmentId) * targetSegmentCnt);
        rebuildSegmentsFile->Open();
        StoreRebuildSegmentSync();
    }
    else
    {
        rebuildSegmentsFile->Open();
        LoadRebuildSegmentSync();
        if (targetSegmentCnt != 0)
        {
            needRebuildCont = true;
        }
    }
}

AllocatorMetaArchive::~AllocatorMetaArchive(void)
{
    delete wbLsidBitmap;
    delete segmentBitmap;
    delete[] segmentInfos;
    delete[] bufferInObj;

    allocatorMetaFile->Close();
    delete allocatorMetaFile;
    rebuildSegmentsFile->Close();
    delete rebuildSegmentsFile;
}

void
AllocatorMetaArchive::_InitMetadata(AllocatorAddressInfo& info)
{
    wbLsidBitmap = new BitMapMutex(info.GetnumWbStripes());
    segmentBitmap = new BitMapMutex(info.GetnumUserAreaSegments());

    currentSsdLsid = STRIPES_PER_SEGMENT - 1;

    for (ASTailArrayIdx asTailArrayIdx = 0; asTailArrayIdx < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++asTailArrayIdx)
    {
        activeStripeTail[asTailArrayIdx] = UNMAP_VSA;
    }

    segmentInfos = new SegmentInfo[info.GetnumUserAreaSegments()];
    for (uint32_t segmentId = 0; segmentId < info.GetnumUserAreaSegments(); ++segmentId)
    {
        segmentInfos[segmentId].SetSegmentId(segmentId);
    }

    targetSegmentCnt = info.GetnumUserAreaSegments();
    bufferInObj = new char[sizeof(targetSegmentCnt) + targetSegmentCnt * sizeof(SegmentId)];
}

void
AllocatorMetaArchive::_UpdateMetaList(AllocatorAddressInfo& info)
{
    metaList[HEADER].addr = (char*)&allocatorMetaHeader;
    metaList[HEADER].size = sizeof(allocatorMetaHeader);

    metaList[WB_LSID_BITMAP].addr = (char*)(wbLsidBitmap->GetMapAddr());
    metaList[WB_LSID_BITMAP].size = wbLsidBitmap->GetNumEntry() * BITMAP_ENTRY_SIZE;

    metaList[SEGMENT_BITMAP].addr = (char*)segmentBitmap->GetMapAddr();
    metaList[SEGMENT_BITMAP].size = segmentBitmap->GetNumEntry() *
        BITMAP_ENTRY_SIZE;

    metaList[ACTIVE_STRIPE_TAIL].addr = (char*)activeStripeTail;
    metaList[ACTIVE_STRIPE_TAIL].size = sizeof(activeStripeTail);

    metaList[CURRENT_SSD_LSID].addr = (char*)&currentSsdLsid;
    metaList[CURRENT_SSD_LSID].size = sizeof(currentSsdLsid);

    metaList[SEGMENT_INFO].addr = (char*)segmentInfos;
    metaList[SEGMENT_INFO].size = sizeof(SegmentInfo) *
        info.GetnumUserAreaSegments();

    int currentOffset = 0;
    allocatorMetaHeader.totalSize = 0;
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        metaList[count].offset = currentOffset;
        currentOffset += metaList[count].size;
        allocatorMetaHeader.totalSize += metaList[count].size;
    }
}

void
AllocatorMetaArchive::_StoreCompleted(AsyncMetaFileIoCtx* ctx)
{
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ALLOCATOR_META_ARCHIVE_STORE,
        "allocator meta file Stored");
    delete[] ctx->buffer;
    delete ctx;
}

void
AllocatorMetaArchive::_StoreRebuildSegmentCompleted(AsyncMetaFileIoCtx* ctx)
{
    int segmentCount = ((AllocatorMetaIoContext*)ctx)->segmentCnt;
    IBOF_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_STORE_REBUILD_SEGMENT),
        "Rebuild Segment File Stored, segmentCount:{}", segmentCount);
    delete ctx;
}

void
AllocatorMetaArchive::_LoadCompleted(AsyncMetaFileIoCtx* ctx)
{
    _MetaLoaded(ctx->buffer);

    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ALLOCATOR_META_ARCHIVE_LOAD,
        "allocator meta file Loaded");
    delete[] ctx->buffer;
    delete ctx;
}

void
AllocatorMetaArchive::_MetaLoaded(char* buffer)
{
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        memcpy(metaList[count].addr, buffer + metaList[count].offset,
            metaList[count].size);
    }
    _HeaderLoaded();
}

void
AllocatorMetaArchive::_LoadRebuildSegmentCompleted(AsyncMetaFileIoCtx* ctx)
{
    _RebuildMetaLoaded();
    delete ctx;
}

void
AllocatorMetaArchive::_RebuildMetaLoaded(void)
{
    int buffer_offset = 0;

    memcpy(&targetSegmentCnt, bufferInObj, sizeof(targetSegmentCnt));
    buffer_offset += sizeof(targetSegmentCnt);

    for (uint32_t cnt = 0; cnt < targetSegmentCnt; ++cnt)
    {
        SegmentId segmentId;
        memcpy(&segmentId, bufferInObj + buffer_offset, sizeof(SegmentId));
        buffer_offset += sizeof(SegmentId);

        auto pr = rebuildTargetSegments.emplace(segmentId);
        if (pr.second == false)
        {
            IBOF_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET_FAILURE),
                "segmentId:{} is already in set", segmentId);
            assert(false);
        }
    }

    IBOF_TRACE_DEBUG(EID(ALLOCATOR_META_ARCHIVE_LOAD_REBUILD_SEGMENT),
        "Rebuild segment file loaded, segmentCount:{}", targetSegmentCnt);
}

void
AllocatorMetaArchive::_PrintMetaInfo(void)
{
    printf("size: %d\n", allocatorMetaHeader.totalSize);
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        printf("addr: %p, offset:%x, size: %d\n", metaList[count].addr,
            metaList[count].offset, metaList[count].size);
    }
}

void
AllocatorMetaArchive::_PrepareMetaStore(char* buffer)
{
    _HeaderUpdate();

    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        assert(allocatorMetaHeader.totalSize > metaList[count].offset);
        memcpy(buffer + metaList[count].offset, metaList[count].addr,
            metaList[count].size);
    }
}

void
AllocatorMetaArchive::Store(void)
{
    char* buffer = new char[allocatorMetaHeader.totalSize]();
    _PrepareMetaStore(buffer);

    AllocatorMetaIoContext* storeRequest = new AllocatorMetaIoContext();
    storeRequest->opcode = MetaFsIoOpcode::Write;
    storeRequest->fd = allocatorMetaFile->GetFd();
    storeRequest->fileOffset = 0;
    storeRequest->length = allocatorMetaHeader.totalSize;
    storeRequest->buffer = buffer;
    storeRequest->callback = std::bind(&AllocatorMetaArchive::_StoreCompleted, this,
        std::placeholders::_1);

    allocatorMetaFile->AsyncIO(storeRequest);
}

int
AllocatorMetaArchive::StoreSync(void)
{
    char* buffer = new char[allocatorMetaHeader.totalSize]();
    _PrepareMetaStore(buffer);

    int ret = allocatorMetaFile->IssueIO(MetaFsIoOpcode::Write, 0,
        allocatorMetaHeader.totalSize, buffer);

    delete[] buffer;
    return ret;
}

int
AllocatorMetaArchive::_PrepareRebuildMeta(void)
{
    targetSegmentCnt = rebuildTargetSegments.size();
    int lenToWrite = sizeof(targetSegmentCnt) + sizeof(SegmentId) * targetSegmentCnt;
    int buffer_offset = 0;

    memcpy(bufferInObj, &targetSegmentCnt, sizeof(targetSegmentCnt));
    buffer_offset += sizeof(targetSegmentCnt);

    for (const auto targetSegment : rebuildTargetSegments)
    {
        memcpy(bufferInObj + buffer_offset, &targetSegment, sizeof(SegmentId));
        buffer_offset += sizeof(SegmentId);
    }
    return lenToWrite;
}

void
AllocatorMetaArchive::StoreRebuildSegment(void)
{
    int lenToWrite = _PrepareRebuildMeta();

    AllocatorMetaIoContext* rebuildStoreRequest = new AllocatorMetaIoContext();
    rebuildStoreRequest->opcode = MetaFsIoOpcode::Write;
    rebuildStoreRequest->fd = rebuildSegmentsFile->GetFd();
    rebuildStoreRequest->fileOffset = 0;
    rebuildStoreRequest->length = lenToWrite;
    rebuildStoreRequest->buffer = bufferInObj;
    rebuildStoreRequest->callback = std::bind(
        &AllocatorMetaArchive::_StoreRebuildSegmentCompleted, this,
        std::placeholders::_1);
    rebuildStoreRequest->segmentCnt = targetSegmentCnt;

    rebuildSegmentsFile->AsyncIO(rebuildStoreRequest);
}

void
AllocatorMetaArchive::StoreRebuildSegmentSync(void)
{
    int lenToWrite = _PrepareRebuildMeta();
    rebuildSegmentsFile->IssueIO(MetaFsIoOpcode::Write, 0, lenToWrite, bufferInObj);
}

void
AllocatorMetaArchive::Load(void)
{
    char* buffer = new char[allocatorMetaHeader.totalSize]();

    AllocatorMetaIoContext* loadRequest = new AllocatorMetaIoContext();
    loadRequest->opcode = MetaFsIoOpcode::Read;
    loadRequest->fd = allocatorMetaFile->GetFd();
    loadRequest->fileOffset = 0;
    loadRequest->length = allocatorMetaHeader.totalSize;
    loadRequest->buffer = buffer;
    loadRequest->callback = std::bind(&AllocatorMetaArchive::_LoadCompleted, this,
        std::placeholders::_1);

    allocatorMetaFile->AsyncIO(loadRequest);
}

void
AllocatorMetaArchive::LoadSync(void)
{
    char* buffer = new char[allocatorMetaHeader.totalSize]();

    allocatorMetaFile->IssueIO(MetaFsIoOpcode::Read, 0,
        allocatorMetaHeader.totalSize, buffer);
    _MetaLoaded(buffer);

    delete[] buffer;
}

void
AllocatorMetaArchive::LoadRebuildSegment(void)
{
    int lenToRead = sizeof(targetSegmentCnt) + sizeof(SegmentId) * targetSegmentCnt;
    // int buffer_offset = 0;

    AllocatorMetaIoContext* rebuildLoadRequest = new AllocatorMetaIoContext();
    rebuildLoadRequest->opcode = MetaFsIoOpcode::Read;
    rebuildLoadRequest->fd = rebuildSegmentsFile->GetFd();
    rebuildLoadRequest->fileOffset = 0;
    rebuildLoadRequest->length = lenToRead;
    rebuildLoadRequest->buffer = bufferInObj;
    rebuildLoadRequest->callback = std::bind(
        &AllocatorMetaArchive::_LoadRebuildSegmentCompleted, this,
        std::placeholders::_1);

    rebuildSegmentsFile->AsyncIO(rebuildLoadRequest);
}

void
AllocatorMetaArchive::LoadRebuildSegmentSync(void)
{
    int lenToRead = sizeof(targetSegmentCnt) + sizeof(SegmentId) * targetSegmentCnt;
    // int buffer_offset = 0;

    rebuildSegmentsFile->IssueIO(MetaFsIoOpcode::Read, 0, lenToRead, bufferInObj);

    _RebuildMetaLoaded();
}

char*
AllocatorMetaArchive::GetCopiedMetaBuffer(void)
{
    _HeaderUpdate();

    char* data = new char[allocatorMetaHeader.totalSize]();

    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        if (count == ACTIVE_STRIPE_TAIL)
        {
            // Temporal workaround to update wbuf tail seperately with seperated lock
            continue;
        }
        assert(allocatorMetaHeader.totalSize > metaList[count].offset);
        memcpy(data + metaList[count].offset, metaList[count].addr,
            metaList[count].size);
    }

    return data;
}

void
AllocatorMetaArchive::CopyWbufTail(char* data, int index)
{
    int offset = sizeof(VirtualBlkAddr) * index;
    memcpy(data + metaList[ACTIVE_STRIPE_TAIL].offset + offset,
        metaList[ACTIVE_STRIPE_TAIL].addr + offset, sizeof(VirtualBlkAddr));
}

int
AllocatorMetaArchive::Flush(char* data, EventSmartPtr callbackEvent)
{
    if (flushInProgress.exchange(true) == true)
    {
        return (int)IBOF_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS;
    }

    flushCallback = callbackEvent;

    AllocatorMetaIoContext* flushRequest = new AllocatorMetaIoContext();
    flushRequest->opcode = MetaFsIoOpcode::Write;
    flushRequest->fd = allocatorMetaFile->GetFd();
    flushRequest->fileOffset = 0;
    flushRequest->length = allocatorMetaHeader.totalSize;
    flushRequest->buffer = data;
    flushRequest->callback = std::bind(&AllocatorMetaArchive::_FlushCompleted,
        this, std::placeholders::_1);

    int ret = allocatorMetaFile->AsyncIO(flushRequest);

    return ret;
}

void
AllocatorMetaArchive::_FlushCompleted(AsyncMetaFileIoCtx* ctx)
{
    assert(ctx->length == allocatorMetaHeader.totalSize);
    delete[] ctx->buffer;
    delete ctx;

    flushInProgress = false;

    EventArgument::GetEventScheduler()->EnqueueEvent(flushCallback);
}

void
AllocatorMetaArchive::_MetaOperation(MetaFsIoOpcode opType, MetaFileIntf* file)
{
    assert(file->DoesFileExist());
    for (int count = 0; count < NUM_ALLOCATOR_META; count++)
    {
        printf("addr: %p, offset:%x, size: %d\n", metaList[count].addr,
            metaList[count].offset, metaList[count].size);

        file->IssueIO(opType, metaList[count].offset, metaList[count].size,
            metaList[count].addr);
    }
}

int
AllocatorMetaArchive::GetMeta(AllocatorMetaType type, std::string fname,
    AllocatorAddressInfo& addrInfo)
{
    MetaFileIntf* dumpFile = new MockFileIntf(fname);
    int ret = 0;

    dumpFile->Create(0);
    dumpFile->Open();
    uint32_t curOffset = 0;

    if (SEGMENT_INVALID_CNT == type)
    {
        uint32_t len = sizeof(uint32_t) * addrInfo.GetnumUserAreaSegments();
        char* buf = new char[len]();

        for (uint32_t segId = 0; segId < addrInfo.GetnumUserAreaSegments(); ++segId)
        {
            uint32_t invCount = segmentInfos[segId].GetinValidBlockCount();
            memcpy(buf + curOffset, &invCount, sizeof(invCount));
            curOffset += sizeof(invCount);
        }

        ret = dumpFile->IssueIO(MetaFsIoOpcode::Write, 0, len, buf);
        if (ret < 0)
        {
            IBOF_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE),
                "Sync Write to {} Failed, ret:{}", fname, ret);
            ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
        }
        delete[] buf;
    }
    else
    {
        if (WB_LSID_BITMAP == type)
        {
            uint32_t numBitsSet = wbLsidBitmap->GetNumBitsSet();
            ret = dumpFile->AppendIO(MetaFsIoOpcode::Write, curOffset, sizeof(numBitsSet),
                (char*)&numBitsSet);
        }
        else if (SEGMENT_BITMAP == type)
        {
            uint32_t numBitsSet = segmentBitmap->GetNumBitsSet();
            ret = dumpFile->AppendIO(MetaFsIoOpcode::Write, curOffset, sizeof(numBitsSet),
                (char*)&numBitsSet);
        }
        // ACTIVE_STRIPE_TAIL, CURRENT_SSD_LSID, SEGMENT_INFO
        else
        {
            // Do nothing
        }
        if (ret < 0)
        {
            IBOF_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE),
                "Sync Write to {} Failed, ret:{}", fname, ret);
            ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
        }

        ret = dumpFile->AppendIO(MetaFsIoOpcode::Write, curOffset, metaList[type].size,
            metaList[type].addr);
        if (ret < 0)
        {
            IBOF_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE),
                "Sync Write to {} Failed, ret:{}", fname, ret);
            ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
        }
    }

    dumpFile->Close();
    delete dumpFile;
    return ret;
}

int
AllocatorMetaArchive::SetMeta(AllocatorMetaType type, std::string fname,
    AllocatorAddressInfo& addrInfo)
{
    MetaFileIntf* fileProvided = new MockFileIntf(fname);
    int ret = 0;

    fileProvided->Open();
    uint32_t curOffset = 0;

    if (WB_LSID_BITMAP == type)
    {
        uint32_t numBitsSet = 0;
        ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset,
            sizeof(numBitsSet), (char*)&numBitsSet);
        wbLsidBitmap->SetNumBitsSet(numBitsSet);
    }
    else if (SEGMENT_BITMAP == type)
    {
        uint32_t numBitsSet = 0;
        ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset,
            sizeof(numBitsSet), (char*)&numBitsSet);
        segmentBitmap->SetNumBitsSet(numBitsSet);
    }
    // ACTIVE_STRIPE_TAIL, CURRENT_SSD_LSID, SEGMENT_INFO
    else
    {
        // Do nothing
    }
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_LOAD),
            "Sync Read from {} Failed, ret:{}", fname, ret);
        ret = -EID(ALLOCATOR_META_ARCHIVE_LOAD);
    }

    ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset,
        metaList[type].size, metaList[type].addr);
    if (ret < 0)
    {
        IBOF_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_LOAD),
            "Sync Read from {} Failed, ret:{}", fname, ret);
        ret = -EID(ALLOCATOR_META_ARCHIVE_LOAD);
    }

    fileProvided->Close();
    delete fileProvided;
    return ret;
}

int
AllocatorMetaArchive::_HeaderUpdate(void)
{
    allocatorMetaHeader.numValidWbLsid = wbLsidBitmap->GetNumBitsSet();
    allocatorMetaHeader.numValidSegment = segmentBitmap->GetNumBitsSet();
    return 0;
}

int
AllocatorMetaArchive::_HeaderLoaded(void)
{
    wbLsidBitmap->SetNumBitsSet(allocatorMetaHeader.numValidWbLsid);
    segmentBitmap->SetNumBitsSet(allocatorMetaHeader.numValidSegment);
    return 0;
}

uint32_t
AllocatorMetaArchive::GetTargetSegmentCnt(void)
{
    return targetSegmentCnt;
}

uint32_t
AllocatorMetaArchive::GetMetaHeaderTotalSize(void)
{
    return allocatorMetaHeader.totalSize;
}

bool
AllocatorMetaArchive::GetNeedRebuildCont(void)
{
    return needRebuildCont;
}

RTSegmentIter
AllocatorMetaArchive::RebuildTargetSegmentsBegin(void)
{
    return rebuildTargetSegments.begin();
}

RTSegmentIter
AllocatorMetaArchive::RebuildTargetSegmentsEnd(void)
{
    return rebuildTargetSegments.end();
}

bool
AllocatorMetaArchive::IsRebuidTargetSegmentsEmpty(void)
{
    return rebuildTargetSegments.empty();
}

void
AllocatorMetaArchive::EraseRebuildTargetSegments(RTSegmentIter iter)
{
    rebuildTargetSegments.erase(iter);
}

RTSegmentIter
AllocatorMetaArchive::FindRebuildTargetSegment(SegmentId segmentId)
{
    return rebuildTargetSegments.find(segmentId);
}

void
AllocatorMetaArchive::ClearRebuildTargetSegments(void)
{
    rebuildTargetSegments.clear();
}

std::pair<RTSegmentIter, bool>
AllocatorMetaArchive::EmplaceRebuildTargetSegment(SegmentId segmentId)
{
    return rebuildTargetSegments.emplace(segmentId);
}

SegmentInfo&
AllocatorMetaArchive::GetSegmentInfo(SegmentId segmentId)
{
    return segmentInfos[segmentId];
}

VirtualBlkAddr
AllocatorMetaArchive::GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTail[asTailArrayIdx];
}

void
AllocatorMetaArchive::SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx,
    VirtualBlkAddr vsa)
{
    activeStripeTail[asTailArrayIdx] = vsa;
}

StripeId
AllocatorMetaArchive::GetPrevSsdLsid(void)
{
    return prevSsdLsid;
}

void
AllocatorMetaArchive::SetPrevSsdLsid(StripeId stripeId)
{
    prevSsdLsid = stripeId;
}

StripeId
AllocatorMetaArchive::GetCurrentSsdLsid(void)
{
    return currentSsdLsid;
}

void
AllocatorMetaArchive::SetCurrentSsdLsid(StripeId stripeId)
{
    currentSsdLsid = stripeId;
}

std::mutex&
AllocatorMetaArchive::GetallocatorMetaLock(void)
{
    return allocatorMetaLock;
}

std::mutex&
AllocatorMetaArchive::GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx)
{
    return activeStripeTailLock[asTailArrayIdx];
}

} // namespace ibofos
