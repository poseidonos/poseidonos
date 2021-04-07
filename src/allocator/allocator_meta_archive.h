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

#pragma once

#include <set>
#include <string>
#include <utility>

#include "allocator_address_info.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/volume/volume_list.h"

namespace ibofos
{
const int ACTIVE_STRIPE_TAIL_ARRAYLEN = MAX_VOLUME_COUNT * 2;

class SegmentInfo;

using RTSegmentIter = std::set<SegmentId>::iterator;
using VolumeId = uint32_t;
using ASTailArrayIdx = uint32_t;

struct AllocatorMetaHeader
{
    uint32_t totalSize = 0; // Sum of all allocator meta size (Byte)
    uint32_t numValidWbLsid = 0;
    uint32_t numValidSegment = 0;
};

class RawMetaInfo
{
public:
    char* addr = nullptr;
    uint32_t size = 0;
    uint32_t offset = 0;
};

enum AllocatorMetaType
{
    HEADER = 0,
    WB_LSID_BITMAP,
    SEGMENT_BITMAP,
    ACTIVE_STRIPE_TAIL,
    CURRENT_SSD_LSID,
    SEGMENT_INFO,

    NUM_ALLOCATOR_META,
    SEGMENT_INVALID_CNT,
};

class AllocatorMetaIoContext : public AsyncMetaFileIoCtx
{
public:
    int segmentCnt;
};

class AllocatorMetaArchive
{
public:
    explicit AllocatorMetaArchive(AllocatorAddressInfo& info);
    virtual ~AllocatorMetaArchive(void);

    void Load(void);
    void LoadSync(void);
    void LoadRebuildSegment(void);
    void LoadRebuildSegmentSync(void);
    void Store(void);
    int StoreSync(void);
    void StoreRebuildSegment(void);
    void StoreRebuildSegmentSync(void);

    uint32_t GetMetaHeaderTotalSize(void);
    bool GetNeedRebuildCont(void);
    uint32_t GetTargetSegmentCnt(void);
    void ClearRebuildTargetSegments(void);
    void EraseRebuildTargetSegments(RTSegmentIter iter);
    bool IsRebuidTargetSegmentsEmpty(void);
    RTSegmentIter RebuildTargetSegmentsBegin(void);
    RTSegmentIter RebuildTargetSegmentsEnd(void);
    RTSegmentIter FindRebuildTargetSegment(SegmentId segmentId);
    std::pair<RTSegmentIter, bool> EmplaceRebuildTargetSegment(SegmentId segmentId);
    SegmentInfo& GetSegmentInfo(SegmentId segmentId);
    VirtualBlkAddr GetActiveStripeTail(ASTailArrayIdx asTailArrayIdx);
    void SetActiveStripeTail(ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa);
    StripeId GetPrevSsdLsid(void);
    void SetPrevSsdLsid(StripeId stripeId);
    StripeId GetCurrentSsdLsid(void);
    void SetCurrentSsdLsid(StripeId stripeId);

    char* GetCopiedMetaBuffer(void);
    void CopyWbufTail(char* data, int index);
    int Flush(char* data, EventSmartPtr callback);
    int GetMeta(AllocatorMetaType type, std::string fname,
        AllocatorAddressInfo& addrInfo);
    int SetMeta(AllocatorMetaType type, std::string fname,
        AllocatorAddressInfo& addrInfo);

    std::mutex& GetallocatorMetaLock(void);
    std::mutex& GetActiveStripeTailLock(ASTailArrayIdx asTailArrayIdx);

    // WriteBuffer
    BitMapMutex* wbLsidBitmap;
    // Segment
    BitMapMutex* segmentBitmap; // Unset:Free, Set:Not-Free

private:
    void _MetaOperation(MetaFsIoOpcode opType, MetaFileIntf* file);
    void _InitMetadata(AllocatorAddressInfo& info);
    void _UpdateMetaList(AllocatorAddressInfo& info);

    void _StoreCompleted(AsyncMetaFileIoCtx* ctx);
    void _StoreRebuildSegmentCompleted(AsyncMetaFileIoCtx* ctx);
    void _LoadCompleted(AsyncMetaFileIoCtx* ctx);
    void _LoadRebuildSegmentCompleted(AsyncMetaFileIoCtx* ctx);

    int _HeaderUpdate(void);
    int _HeaderLoaded(void);

    void _PrepareMetaStore(char* data);
    void _MetaLoaded(char* data);
    void _FlushCompleted(AsyncMetaFileIoCtx* ctx);

    int _PrepareRebuildMeta(void);
    void _RebuildMetaLoaded(void);
    void _PrintMetaInfo(void);

    AllocatorMetaHeader allocatorMetaHeader;  // allocatorMetaLock
    RawMetaInfo metaList[NUM_ALLOCATOR_META]; // allocatorMetaLock
    std::mutex allocatorMetaLock;
    MetaFileIntf* allocatorMetaFile;
    char* bufferInObj;
    EventSmartPtr flushCallback;
    std::atomic<bool> flushInProgress;

    // WriteBuffer
    //    Description about 'ASTailArrayIdx'
    //    Index  0  ... 255 are for Userdata IO of volume 0 ... 255
    //    Index 256 ... 511 are for GC IO of volume 0 ... 255
    VirtualBlkAddr activeStripeTail[ACTIVE_STRIPE_TAIL_ARRAYLEN];
    std::mutex activeStripeTailLock[ACTIVE_STRIPE_TAIL_ARRAYLEN];

    // Segment
    StripeId prevSsdLsid;    // allocatorMetaLock
    StripeId currentSsdLsid; // allocatorMetaLock
    SegmentInfo* segmentInfos;

    // Rebuild
    bool needRebuildCont;
    uint32_t targetSegmentCnt;
    std::set<SegmentId> rebuildTargetSegments; // No lock
    MetaFileIntf* rebuildSegmentsFile;
};

} // namespace ibofos
