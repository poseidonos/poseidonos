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

#include <list>
#include <tuple>

#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/meta_const.h"
#include "src/mapper/include/mapper_const.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/volume/volume_service.h"

namespace pos
{

ReverseMapPack::ReverseMapPack(void)
: linkedToVsid(false),
  vsid(UINT32_MAX),
  ioError(0),
  ioDirection(0),
  callback(nullptr)
{
    wbLsid = UNMAP_STRIPE;
    mpageSize = 0;
    numMpagesPerStripe = 0;
    fileSizePerStripe = 0;
    revMapfile = nullptr;
    iVSAMap = nullptr;
    iStripeMap = nullptr;
    reconstructStatus.lsid = UNMAP_STRIPE;
    reconstructStatus.vsid = UNMAP_STRIPE;
    reconstructStatus.volId = UINT32_MAX;
    reconstructStatus.totalRbaNum = UINT64_MAX;
}

ReverseMapPack::~ReverseMapPack(void)
{
    for (auto& revMap : revMaps)
    {
        delete revMap;
        revMap = nullptr;
    }
    revMaps.clear();

    revMapfile = nullptr;
}

void
ReverseMapPack::Init(uint64_t mpsize, uint64_t nmpPerStripe, MetaFileIntf* file, std::string arrName)
{
    mpageSize = mpsize;
    numMpagesPerStripe = nmpPerStripe;
    fileSizePerStripe = mpageSize * numMpagesPerStripe;
    revMapfile = file;
    arrayName = arrName;

    for (uint64_t mpage = 0; mpage < numMpagesPerStripe; ++mpage)
    {
        RevMap* revMap = new RevMap();
        memset(revMap, 0xFF, mpageSize);
        revMaps.push_back(revMap);
    }
}

void
ReverseMapPack::Init(StripeId wblsid, IVSAMap* ivsaMap, IStripeMap* istripeMap)
{
    wbLsid = wblsid;
    iVSAMap = ivsaMap;
    iStripeMap = istripeMap;
    _HeaderInit(wbLsid);
}

int
ReverseMapPack::LinkVsid(StripeId vsid)
{
    if (unlikely(linkedToVsid))
    {
        POS_TRACE_ERROR(EID(REVMAP_PACK_ALREADY_LINKED),
            "ReverseMapPack for wbLsid:{} is already linked to vsid:{} but tried to link vsid:{} again",
            this->wbLsid, this->vsid, vsid);
        return -EID(REVMAP_PACK_ALREADY_LINKED);
    }

    // Set vsid to object and header
    this->vsid = vsid;
    RevMapSector& hdr = revMaps[0]->sector[0];
    hdr.u.header.vsid = vsid;

    linkedToVsid = true;
    return 0;
}

int
ReverseMapPack::UnLinkVsid(void)
{
    if (unlikely(linkedToVsid == false))
    {
        POS_TRACE_ERROR(EID(REVMAP_NOT_LINKED_PACK),
            "ReverseMapPack for wbLsid:{} is not linked but tried to unlink", this->wbLsid);
        return -EID(REVMAP_NOT_LINKED_PACK);
    }

    vsid = UINT32_MAX;

    _HeaderInit(wbLsid);

    linkedToVsid = false;

    return 0;
}

int
ReverseMapPack::Load(EventSmartPtr inputCallback)
{
    ioDirection = IO_LOAD;
    ioError = 0;
    if (unlikely(linkedToVsid == false))
    {
        POS_TRACE_ERROR(EID(REVMAP_NOT_LINKED_PACK),
            "This Pack wblsid:{} is not linked but tried to Load()", this->wbLsid);
        ioError = -EID(REVMAP_NOT_LINKED_PACK);
        return ioError;
    }

    mfsAsyncIoDonePages = 0;
    int pageNum = 0;
    callback = inputCallback;

    for (auto& revMap : revMaps)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx();
        revMapPageAsyncIoReq->opcode = MetaFsIoOpcode::Read;
        revMapPageAsyncIoReq->fd = revMapfile->GetFd();
        revMapPageAsyncIoReq->fileOffset = (fileSizePerStripe * vsid) + mpageSize * pageNum;
        revMapPageAsyncIoReq->length = mpageSize;
        revMapPageAsyncIoReq->buffer = (char*)revMap;
        revMapPageAsyncIoReq->callback = std::bind(&ReverseMapPack::_RevMapPageIoDone,
            this, std::placeholders::_1);

        revMapPageAsyncIoReq->mpageNum = pageNum++;

        int ret = revMapfile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
                "Calling AsyncIO Failed at RevMap LOAD, mpageNum:{}",
                revMapPageAsyncIoReq->mpageNum);
            ioError = ret;
        }
    }

    return ioError;
}

int
ReverseMapPack::Flush(Stripe* stripe, EventSmartPtr inputCallback)
{
    ioDirection = IO_FLUSH;
    ioError = 0;
    if (unlikely(linkedToVsid == false))
    {
        POS_TRACE_ERROR(EID(REVMAP_NOT_LINKED_PACK),
            "This Pack wblsid:{} is not linked but tried to Flush()", this->wbLsid);
        ioError = -EID(REVMAP_NOT_LINKED_PACK);
        return ioError;
    }

    mfsAsyncIoDonePages = 0;
    int pageNum = 0;

    callback = inputCallback;

    for (auto& revMap : revMaps)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx();
        revMapPageAsyncIoReq->opcode = MetaFsIoOpcode::Write;
        revMapPageAsyncIoReq->fd = revMapfile->GetFd();
        revMapPageAsyncIoReq->fileOffset = (fileSizePerStripe * vsid) + mpageSize * pageNum;
        revMapPageAsyncIoReq->length = mpageSize;
        revMapPageAsyncIoReq->buffer = (char*)revMap;
        revMapPageAsyncIoReq->callback = std::bind(&ReverseMapPack::_RevMapPageIoDone,
            this, std::placeholders::_1);

        revMapPageAsyncIoReq->mpageNum = pageNum++;
        revMapPageAsyncIoReq->stripeToFlush = stripe;

        int ret = revMapfile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
                "Calling AsyncIO Failed at RevMap FLUSH, mpageNum:{}",
                revMapPageAsyncIoReq->mpageNum);
            ioError = ret;
            callback = nullptr;
        }
    }

    return ioError;
}

void
ReverseMapPack::_RevMapPageIoDone(AsyncMetaFileIoCtx* ctx)
{
    RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = static_cast<RevMapPageAsyncIoCtx*>(ctx);
    if (revMapPageAsyncIoReq->error != 0)
    {
        ioError = revMapPageAsyncIoReq->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "MFS AsyncIO error, ioError:{}  mpageNum:{}", ioError,
            revMapPageAsyncIoReq->mpageNum);
    }

    if (++mfsAsyncIoDonePages == numMpagesPerStripe)
    {
        if (callback != nullptr)
        {
            EventSchedulerSingleton::Instance()->EnqueueEvent(callback);
            callback = nullptr;
        }
    }

    delete ctx;
}

std::tuple<uint32_t, uint32_t, uint32_t>
ReverseMapPack::_ReverseMapGeometry(uint64_t offset) //   Ex) offset = 300 | 500
{
    uint32_t sectorNr = offset / NUM_ENTRIES + 1;    // sectorNr =  15 | 24
    uint32_t entNr = offset % NUM_ENTRIES;           //    entNr =   6 | 17
    uint32_t pageNr = sectorNr / NUM_SECTOR_ENTRIES; //   pageNr =   0 |  1
    sectorNr = sectorNr % NUM_SECTOR_ENTRIES;        // sectorNr =  15 |  8
    // Check the results by calculation
    // (16*21) * 0 + 21 * (15-1) + 6  == 300
    // (16*21) * 1 + 21 * (8-1)  + 17 == 500
    return std::make_tuple(pageNr, sectorNr, entNr);
}

int
ReverseMapPack::SetReverseMapEntry(uint32_t offset, BlkAddr rba, uint32_t volumeId)
{
    uint32_t pageIndex;
    uint32_t sectorIndex;
    uint32_t entryIndex;
    std::tie(pageIndex, sectorIndex, entryIndex) = _ReverseMapGeometry(offset);

    RevMapEntry& entry =
        revMaps[pageIndex]->sector[sectorIndex].u.body.entry[entryIndex];
    entry.u.entry.rba = rba;
    entry.u.entry.volumeId = volumeId;

    return 0;
}

std::tuple<BlkAddr, uint32_t>
ReverseMapPack::GetReverseMapEntry(uint32_t offset)
{
    uint32_t pageIndex;
    uint32_t sectorIndex;
    uint32_t entryIndex;
    std::tie(pageIndex, sectorIndex, entryIndex) = _ReverseMapGeometry(offset);

    RevMapEntry& entry =
        revMaps[pageIndex]->sector[sectorIndex].u.body.entry[entryIndex];
    BlkAddr rba = entry.u.entry.rba;
    uint32_t volumeId = entry.u.entry.volumeId;

    return std::make_tuple(rba, volumeId);
}

int
ReverseMapPack::IsAsyncIoDone(void)
{
    if (unlikely(ioError != 0))
    {
        POS_TRACE_ERROR(EID(REVMAP_MFS_IO_ERROR),
            "RevMap MFS IO Error:{}  IoDirection:{}", ioError, ioDirection);
        return ioError; // < 0
    }

    return numMpagesPerStripe - mfsAsyncIoDonePages; // 0: Done, > 0: Remaining pages
}

int
ReverseMapPack::GetIoError(void)
{
    return ioError;
}

void
ReverseMapPack::_HeaderInit(StripeId wblsid)
{
    // Set Header as default
    RevMapSector& hdr = revMaps[0]->sector[0];
    hdr.u.header.magic = REVMAP_MAGIC;
    hdr.u.header.wbLsid = wblsid;
}

std::tuple<uint32_t, uint32_t>
ReverseMapPack::_GetCurrentTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint32_t sec = ts.tv_sec;
    uint32_t nsec = ts.tv_nsec;

    return std::make_tuple(sec, nsec);
}

int
ReverseMapPack::_SetTimeToHeader(ACTION act)
{
    uint32_t sec, nsec;
    std::tie(sec, nsec) = _GetCurrentTime();

    RevMapSector& hdr = revMaps[0]->sector[0];
    hdr.u.header.closeSec = sec;
    hdr.u.header.closeNsec = nsec;

    return 0;
}

int
ReverseMapPack::WbtFileSyncIo(MetaFileIntf* fileLinux, MetaFsIoOpcode IoDirection)
{
    int ret = 0;
    uint64_t curOffset = 0;

    for (auto& revMap : revMaps)
    {
        ret = fileLinux->AppendIO(IoDirection, curOffset, mpageSize, (char*)revMap);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MAPPER_FAILED), "AppendIO Error, ret:{}  fd:{}", ret,
                fileLinux->GetFd());
            return ret;
        }
    }

    return ret;
}

int
ReverseMapPack::ReconstructMap(uint32_t volumeId, StripeId vsid, StripeId lsid, uint64_t blockCount)
{
    int ret = 0;
    std::list<BlkAddr> foundRbas;

    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA),
        "[RMR]START, volumeId:{}  wbLsid:{}  vsid:{}  blockCount:{}",
        volumeId, lsid, vsid, blockCount);

    ret = reconstructStatus.Init(lsid, vsid, volumeId, arrayName);
    if (ret != 0)
    {
        return ret;
    }

    for (uint64_t offset = 0; offset < blockCount; ++offset)
    {
        // Set the RBA to start finding
        BlkAddr rbaStart = 0;
        if (0 != foundRbas.size())
        {
            rbaStart = foundRbas.front() + 1;
        }

        BlkAddr foundRba = INVALID_RBA;
        bool found = _FindRba(offset, rbaStart, foundRba);

        if (found == true)
        {
            foundRbas.emplace_front(foundRba);
        }
        SetReverseMapEntry(offset, foundRba, volumeId);
    }

    POS_TRACE_INFO(EID(REVMAP_RECONSTRUCT_FOUND_RBA),
        "[RMR] {}/{} blocks are reconstructed for Stripe(wbLsid:{})",
        foundRbas.size(), blockCount, lsid);

    return ret;
}

bool
ReverseMapPack::_FindRba(uint64_t offset, BlkAddr rbaStart, BlkAddr& foundRba)
{
    bool looped = false;
    for (BlkAddr rbaToCheck = rbaStart; rbaToCheck <= reconstructStatus.totalRbaNum; ++rbaToCheck)
    {
        // If rbaToCheck exceeds more than totalRbaNum, looped would be set as TRUE
        if (rbaToCheck == reconstructStatus.totalRbaNum)
        {
            rbaToCheck = 0;
            looped = true;
        }

        if ((rbaToCheck == rbaStart) && looped)
        {
            return false;
        }

        int caller = CALLER_NOT_EVENT;
        VirtualBlkAddr vsaToCheck = iVSAMap->GetVSAInternal(reconstructStatus.volId, rbaToCheck, caller);
        if (vsaToCheck == UNMAP_VSA || vsaToCheck.offset != offset || vsaToCheck.stripeId != reconstructStatus.vsid)
        {
            continue;
        }

        StripeAddr lsaToCheck = iStripeMap->GetLSA(vsaToCheck.stripeId);
        if (iStripeMap->IsInUserDataArea(lsaToCheck) || lsaToCheck.stripeId != reconstructStatus.lsid)
        {
            continue;
        }

        foundRba = rbaToCheck;
        return true;
    }

    return false;
}

int
ReverseMapPack::ReconstructInfo::Init(StripeId inputLsid, StripeId inputVsid, uint32_t volumeId, std::string arrName)
{
    IVolumeManager& volumeManager = *VolumeServiceSingleton::Instance()->GetVolumeManager(arrName);

    uint64_t volSize = 0;
    int ret = volumeManager.GetVolumeSize(volumeId, volSize);
    if (ret != 0)
    {
        POS_TRACE_WARN(EID(GET_VOLUMESIZE_FAILURE), "[RMR]GetVolumeSize failure, volumeId:{}", volumeId);
        return -(int)EID(GET_VOLUMESIZE_FAILURE);
    }

    lsid = inputLsid;
    vsid = inputVsid;
    volId = volumeId;
    totalRbaNum = DivideUp(volSize, BLOCK_SIZE);

    return 0;
}

} // namespace pos
