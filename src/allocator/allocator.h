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

#include <string>
#include <vector>

#include "allocator_meta_archive.h"
#include "src/include/meta_const.h"
#include "src/lib/singleton.h"
#include "src/sys_event/volume_event.h"

namespace ibofos
{
using StripeVec = std::vector<Stripe*>;

const uint32_t BUFFER_ALLOCATION_SIZE = 2 * 1024 * 1024;
const uint32_t CHUNK_PER_BUFFER_ALLOCATION = BUFFER_ALLOCATION_SIZE /
    CHUNK_SIZE;
const int BLOCKS_PER_GROUP = 8;

class AllocatorAddressInfo;
class AllocatorMetaArchive;
class ArrayDuty;
class CommonDuty;
class GcDuty;
class MainDuty;
class IoDuty;
class JournalManagerDuty;
class StripeDuty;
class StripeAllocateDoneEvent;
class WriteHandler;

class Allocator : public VolumeEvent
{
public:
    Allocator(void);
    virtual ~Allocator(void);
    void Init(void);

    void FreeWriteBufferStripeId(StripeId lsid);
    virtual void TryToUpdateSegmentValidBlks(StripeId lsid);

    VirtualBlks AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks);
    VirtualBlks AllocateGcBlk(uint32_t volumeId, uint32_t numBlks);
    StripeId AllocateUserDataStripeId(StripeId vsid);

    uint32_t GetNumOfFreeUserDataSegment(void);
    SegmentId GetMostInvalidatedSegment(void);
    void SetGcThreshold(uint32_t inputThreshold);
    void SetUrgentThreshold(uint32_t inputThreshold);
    uint32_t GetGcThreshold(void);
    uint32_t GetUrgentThreshold(void);
    void SetBlockingSegmentAllocation(bool isBlock);
    void FreeUserDataSegmentId(SegmentId segId);

    int PrepareRebuild(void);
    int StopRebuilding(void);
    bool NeedRebuildAgain(void);
    SegmentId GetRebuildTargetSegment(void);
    int ReleaseRebuildSegment(SegmentId segmentId);

    int FlushMetadata(EventSmartPtr callback);
    virtual void ReplaySsdLsid(StripeId currentSsdLsid);
    virtual int FlushStripe(VolumeId volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa);
    void FlushFullActiveStripes(void);
    virtual void ReplaySegmentAllocation(StripeId userLsid);
    virtual void ReplayStripeAllocation(StripeId vsid, StripeId wbLsid);
    virtual void ReplayStripeFlushed(StripeId wbLsid);
    std::vector<VirtualBlkAddr> GetActiveStripeTail(void);
    virtual void ResetActiveStripeTail(int index);
    virtual int RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid);

    void FlushAllUserdata(void);
#if defined NVMe_FLUSH_HANDLING
    void GetAllActiveStripes(int volumeId);
    bool FlushPartialStripes(void);
    bool WaitForPartialStripesFlush(void);
    bool WaitForAllStripesFlush(int volumeId);
    void UnblockAllocating(void);
#endif

    int Store(void);
    Stripe* GetStripe(StripeAddr& lsidEntry);
    virtual void InvalidateBlks(VirtualBlks blks);

    void TryToResetSegmentState(StripeId lsid, bool replay = false);
    int GetMeta(AllocatorMetaType type, std::string fname); // WBT
    int SetMeta(AllocatorMetaType type, std::string fname); // WBT
    int GetBitmapLayout(std::string fname);                 // WBT
    int GetInstantMetaInfo(std::string fname);              // WBT
    void FlushAllUserdataWBT(void);                         // WBT
    void FreeUserDataStripeId(StripeId lsid);
    bool IsValidWriteBufferStripeId(StripeId lsid);
    bool IsValidUserDataSegmentId(SegmentId segId);

    bool VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem,
        uint64_t maxiops, uint64_t maxbw) override;
    bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
        uint64_t maxbw) override;
    bool VolumeDeleted(std::string volName, int volID,
        uint64_t volSizeByte) override;
    bool VolumeMounted(std::string volName, std::string subnqn, int volID,
        uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw) override;
    bool VolumeUnmounted(std::string volName, int volID) override;
    bool VolumeLoaded(std::string name, int id, uint64_t totalSize,
        uint64_t maxiops, uint64_t maxbw) override;
    void VolumeDetached(vector<int> volList) override;

private:
    AllocatorAddressInfo* addrInfo;
    AllocatorMetaArchive* meta;
    ArrayDuty* arrayDuty;
    CommonDuty* commonDuty;
    GcDuty* gcDuty;
    MainDuty* mainDuty;
    IoDuty* ioDuty;
    JournalManagerDuty* journalManagerDuty;
    StripeDuty* stripeDuty;
};

using AllocatorSingleton = Singleton<Allocator>;

} // namespace ibofos
