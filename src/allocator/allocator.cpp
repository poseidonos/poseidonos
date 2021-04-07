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

#include "allocator.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "active_stripe_index_info.h"
#include "array_duty.h"
#include "common_duty.h"
#include "gc_duty.h"
#include "io_duty.h"
#include "journalmanager_duty.h"
#include "main_duty.h"
#include "stripe_duty.h"

namespace ibofos
{
Allocator::Allocator(void)
: VolumeEvent("Allocator"),
  addrInfo(nullptr),
  meta(nullptr),
  commonDuty(nullptr),
  gcDuty(nullptr),
  mainDuty(nullptr),
  ioDuty(nullptr),
  journalManagerDuty(nullptr),
  stripeDuty(nullptr)
{
}

void
Allocator::Init(void)
{
    addrInfo = new AllocatorAddressInfo;
    meta = new AllocatorMetaArchive(*addrInfo);

    gcDuty = new GcDuty(*addrInfo, meta);
    commonDuty = new CommonDuty(*addrInfo, meta, gcDuty);
    ioDuty = new IoDuty(*addrInfo, meta, gcDuty, commonDuty);
    mainDuty = new MainDuty(*addrInfo, meta, commonDuty, ioDuty);
    journalManagerDuty = new JournalManagerDuty(*addrInfo, meta, commonDuty,
        mainDuty, ioDuty);
    arrayDuty = new ArrayDuty(*addrInfo, meta, commonDuty, mainDuty, ioDuty);
    stripeDuty = new StripeDuty(*addrInfo, meta, ioDuty);
}

Allocator::~Allocator(void)
{
    if (stripeDuty != nullptr)
    {
        delete stripeDuty;
        stripeDuty = nullptr;
        delete arrayDuty;
        arrayDuty = nullptr;
        delete journalManagerDuty;
        journalManagerDuty = nullptr;
        delete mainDuty;
        mainDuty = nullptr;
        delete ioDuty;
        ioDuty = nullptr;
        delete commonDuty;
        commonDuty = nullptr;
        delete gcDuty;
        gcDuty = nullptr;
        delete meta;
        meta = nullptr;
        delete addrInfo;
        addrInfo = nullptr;
    }
}

int
Allocator::PrepareRebuild(void)
{
    return arrayDuty->PrepareRebuild();
}

SegmentId
Allocator::GetRebuildTargetSegment(void)
{
    return arrayDuty->GetRebuildTargetSegment();
}

int
Allocator::ReleaseRebuildSegment(SegmentId segmentId)
{
    return arrayDuty->ReleaseRebuildSegment(segmentId);
}

bool
Allocator::NeedRebuildAgain(void)
{
    return arrayDuty->NeedRebuildAgain();
}

int
Allocator::StopRebuilding(void)
{
    return arrayDuty->StopRebuilding();
}

void
Allocator::FreeUserDataStripeId(StripeId lsid)
{
    commonDuty->FreeUserDataStripeId(lsid);
}

void
Allocator::InvalidateBlks(VirtualBlks blks)
{
    commonDuty->InvalidateBlks(blks);
}

bool
Allocator::IsValidWriteBufferStripeId(StripeId lsid)
{
    return commonDuty->IsValidWriteBufferStripeId(lsid);
}

bool
Allocator::IsValidUserDataSegmentId(SegmentId segId)
{
    return commonDuty->IsValidUserDataSegmentId(segId);
}

int
Allocator::GetInstantMetaInfo(std::string fname)
{
    std::ostringstream oss;
    std::ofstream ofs(fname, std::ofstream::app);

    oss << "<< WriteBuffers >>" << std::endl;
    oss << "Set:" << std::dec << meta->wbLsidBitmap->GetNumBitsSet() << " / ToTal:"
        << meta->wbLsidBitmap->GetNumBits() << std::endl;
    oss << "activeStripeTail[] Info" << std::endl;
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        for (int idx = volumeId; idx < ACTIVE_STRIPE_TAIL_ARRAYLEN; idx += MAX_VOLUME_COUNT)
        {
            VirtualBlkAddr asTail = meta->GetActiveStripeTail(idx);
            oss << "Idx:" << std::dec << idx << " stripeId:0x" << std::hex << asTail.stripeId << " offset:0x" << asTail.offset << "  ";
        }
        oss << std::endl;
    }
    oss << std::endl;

    oss << "<< Segments >>" << std::endl;
    oss << "Set:" << std::dec << meta->segmentBitmap->GetNumBitsSet() << " / ToTal:" << meta->segmentBitmap->GetNumBits() << std::endl;
    oss << "currentSsdLsid: " << meta->GetCurrentSsdLsid() << std::endl;
    for (uint32_t segmentId = 0; segmentId < addrInfo->GetnumUserAreaSegments(); ++segmentId)
    {
        SegmentInfo& segInfo = meta->GetSegmentInfo(segmentId);
        if ((segmentId > 0) && (segmentId % 4 == 0))
        {
            oss << std::endl;
        }
        oss << "SegmentId:" << segmentId << " state:" << static_cast<int>(segInfo.Getstate()) << " InvalidBlockCnt:" << segInfo.GetinValidBlockCount() << "  ";
    }
    oss << std::endl
        << std::endl;

    oss << "<< Rebuild >>" << std::endl;
    oss << "NeedRebuildCont:" << std::boolalpha << meta->GetNeedRebuildCont() << std::endl;
    oss << "TargetSegmentCount:" << meta->GetTargetSegmentCnt() << std::endl;
    oss << "TargetSegnent ID" << std::endl;
    int cnt = 0;
    for (RTSegmentIter iter = meta->RebuildTargetSegmentsBegin(); iter != meta->RebuildTargetSegmentsEnd(); ++iter, ++cnt)
    {
        if (cnt > 0 && (cnt % 16 == 0))
        {
            oss << std::endl;
        }
        oss << *iter << " " << std::endl;
    }
    oss << std::endl;

    ofs << oss.str();
    return 0;
}

int
Allocator::GetMeta(AllocatorMetaType type, std::string fname)
{
    return meta->GetMeta(type, fname, *addrInfo);
}

int
Allocator::SetMeta(AllocatorMetaType type, std::string fname)
{
    return meta->SetMeta(type, fname, *addrInfo);
}

Stripe*
Allocator::GetStripe(StripeAddr& lsidEntry)
{
    return commonDuty->GetStripe(lsidEntry);
}

int
Allocator::GetBitmapLayout(std::string fname)
{
    std::ofstream ofs(fname, std::ofstream::app);

    ofs << "numWbStripe: 0x" << std::hex << addrInfo->GetnumWbStripes() << std::endl;
    ofs << "numUserAreaSegment: 0x" << std::hex << addrInfo->GetnumUserAreaSegments() << std::endl;
    ofs << "numUserAreaStripes: 0x" << std::hex << addrInfo->GetnumUserAreaStripes()
        << std::endl;
    ofs << "blksPerStripe: 0x" << std::hex << addrInfo->GetblksPerStripe() << std::endl;
    ofs << "InvalidCountSize: 0x" << std::hex << sizeof(uint32_t) << std::endl;
    ofs << std::endl;

    return 0;
}

int
Allocator::Store(void)
{
    return commonDuty->Store();
}

void
Allocator::TryToResetSegmentState(StripeId lsid, bool replay)
{
    commonDuty->TryToResetSegmentState(lsid, replay);
}

void
Allocator::FreeUserDataSegmentId(SegmentId segId)
{
    gcDuty->FreeUserDataSegmentId(segId);
}

uint32_t
Allocator::GetNumOfFreeUserDataSegment(void)
{
    return gcDuty->GetNumOfFreeUserDataSegment();
}

SegmentId
Allocator::GetMostInvalidatedSegment(void)
{
    return gcDuty->GetMostInvalidatedSegment();
}

void
Allocator::SetGcThreshold(uint32_t inputThreshold)
{
    gcDuty->SetGcThreshold(inputThreshold);
}

void
Allocator::SetUrgentThreshold(uint32_t inputThreshold)
{
    gcDuty->SetUrgentThreshold(inputThreshold);
}

uint32_t
Allocator::GetGcThreshold(void)
{
    return gcDuty->GetGcThreshold();
}

uint32_t
Allocator::GetUrgentThreshold(void)
{
    return gcDuty->GetUrgentThreshold();
}

void
Allocator::SetBlockingSegmentAllocation(bool isBlock)
{
    gcDuty->SetUpBlockSegmentAllocForUser(isBlock);
}

VirtualBlks
Allocator::AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks)
{
    return ioDuty->AllocateWriteBufferBlks(volumeId, numBlks);
}

VirtualBlks
Allocator::AllocateGcBlk(uint32_t volumeId, uint32_t numBlks)
{
    return ioDuty->AllocateGcBlk(volumeId, numBlks);
}

StripeId
Allocator::AllocateUserDataStripeId(StripeId vsid)
{
    return ioDuty->AllocateUserDataStripeId(vsid);
}

int
Allocator::FlushMetadata(EventSmartPtr callback)
{
    return journalManagerDuty->FlushMetadata(callback);
}

int
Allocator::FlushStripe(VolumeId volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa)
{
    return journalManagerDuty->FlushStripe(volumeId, wbLsid, tailVsa);
}

void
Allocator::FlushFullActiveStripes(void)
{
    journalManagerDuty->FlushFullActiveStripes();
}

void
Allocator::ReplaySegmentAllocation(StripeId userLsid)
{
    journalManagerDuty->ReplaySegmentAllocation(userLsid);
}

void
Allocator::ReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    journalManagerDuty->ReplayStripeAllocation(vsid, wbLsid);
}

void
Allocator::ReplayStripeFlushed(StripeId wbLsid)
{
    journalManagerDuty->ReplayStripeFlushed(wbLsid);
}

std::vector<VirtualBlkAddr>
Allocator::GetActiveStripeTail(void)
{
    return journalManagerDuty->GetActiveStripeTail();
}

void
Allocator::ResetActiveStripeTail(int index)
{
    journalManagerDuty->ResetActiveStripeTail(index);
}

int
Allocator::RestoreActiveStripeTail(int index, VirtualBlkAddr tail, StripeId wbLsid)
{
    return journalManagerDuty->RestoreActiveStripeTail(index, tail, wbLsid);
}

void
Allocator::ReplaySsdLsid(StripeId currentSsdLsid)
{
    journalManagerDuty->ReplaySsdLsid(currentSsdLsid);
}

void
Allocator::FlushAllUserdata(void)
{
    mainDuty->FlushAllUserdata();
}

void
Allocator::FlushAllUserdataWBT(void)
{
    mainDuty->FlushAllUserdataWBT();
}

#if defined NVMe_FLUSH_HANDLING
void
Allocator::GetAllActiveStripes(int volumeId)
{
    mainDuty->GetAllActiveStripes(volumeId);
}

bool
Allocator::FlushPartialStripes(void)
{
    return mainDuty->FlushPartialStripes();
}

bool
Allocator::WaitForPartialStripesFlush(void)
{
    return mainDuty->WaitForPartialStripesFlush();
}

bool
Allocator::WaitForAllStripesFlush(int volumeId)
{
    return mainDuty->WaitForAllStripesFlush(volumeId);
}

void
Allocator::UnblockAllocating()
{
    mainDuty->UnblockAllocating();
}
#endif

void
Allocator::FreeWriteBufferStripeId(StripeId lsid)
{
    stripeDuty->FreeWriteBufferStripeId(lsid);
}

void
Allocator::TryToUpdateSegmentValidBlks(StripeId lsid)
{
    stripeDuty->TryToUpdateSegmentValidBlks(lsid);
}

bool
Allocator::VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeUpdated(std::string volName, int volID, uint64_t maxiops,
    uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte)
{
    return true;
}

bool
Allocator::VolumeMounted(std::string volName, std::string subnqn, int volID,
    uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

bool
Allocator::VolumeUnmounted(std::string volName, int volID)
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    {
        std::unique_lock<std::mutex> lock(meta->GetallocatorMetaLock());
        commonDuty->PickActiveStripe(volID, stripesToFlush, vsidToCheckFlushDone);
    }

    mainDuty->FinalizeWriteIO(stripesToFlush, vsidToCheckFlushDone);
    return true;
}

bool
Allocator::VolumeLoaded(std::string name, int id, uint64_t totalSize,
    uint64_t maxiops, uint64_t maxbw)
{
    return true;
}

void
Allocator::VolumeDetached(vector<int> volList)
{
}

} // namespace ibofos
