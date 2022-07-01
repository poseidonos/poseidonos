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

#include "src/mapper/reversemap/reverse_map.h"

#include <list>
#include <tuple>

#include "src/array_mgmt/array_manager.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/meta_const.h"
#include "src/mapper/include/mapper_const.h"
#include "src/meta_file_intf/meta_file_intf.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
ReverseMapPack::ReverseMapPack(void)
: wbLsid(UNMAP_STRIPE),
  vsid(UINT32_MAX),
  revMapfile(nullptr),
  mpageSize(0),
  mfsAsyncIoDonePages(0),
  mapFlushState(MapFlushState::FLUSH_DONE),
  numMpagesPerStripe(0),
  ioError(0),
  ioDirection(0),
  callback(nullptr),
  telemetryPublisher(nullptr),
  issuedIoCnt(0)
{
}

// LCOV_EXCL_START
ReverseMapPack::~ReverseMapPack(void)
{
    for (auto& revMap : revMaps)
    {
        if (revMap != nullptr)
        {
            delete revMap;
            revMap = nullptr;
        }
    }
    revMaps.clear();
    callback = nullptr;
    revMapfile = nullptr;
}
// LCOV_EXCL_STOP

void
ReverseMapPack::Init(MetaFileIntf* file, StripeId wbLsid_, StripeId vsid_, uint32_t mpageSize_, uint32_t numMpagesPerStripe_, TelemetryPublisher* tp)
{
    mapFlushState = MapFlushState::FLUSH_DONE;
    revMapfile = file;
    wbLsid = wbLsid_;
    vsid = vsid_;
    mpageSize = mpageSize_;
    numMpagesPerStripe = numMpagesPerStripe_;
    telemetryPublisher = tp;
    for (uint64_t mpage = 0; mpage < numMpagesPerStripe; ++mpage)
    {
        RevMap* revMap = new RevMap();
        memset(revMap, 0xFF, mpageSize);
        revMaps.push_back(revMap);
    }
    _SetHeader(wbLsid, vsid);
}

void
ReverseMapPack::Assign(StripeId vsid_)
{
    assert(mapFlushState == MapFlushState::FLUSH_DONE);
    vsid = vsid_;
    _SetHeader(wbLsid, vsid);
    callback = nullptr;
}

int
ReverseMapPack::Load(uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid)
{
    ioDirection = IO_LOAD;
    ioError = 0;
    mfsAsyncIoDonePages = 0;
    mapFlushState = MapFlushState::FLUSHING;
    uint64_t pageNum = 0;
    callback = cb;

    for (auto& revMap : revMaps)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx();
        revMapPageAsyncIoReq->opcode = MetaFsIoOpcode::Read;
        revMapPageAsyncIoReq->fd = revMapfile->GetFd();
        revMapPageAsyncIoReq->fileOffset = fileOffset + (uint64_t)mpageSize * pageNum;
        revMapPageAsyncIoReq->length = mpageSize;
        revMapPageAsyncIoReq->buffer = (char*)revMap;
        revMapPageAsyncIoReq->callback = std::bind(&ReverseMapPack::_RevMapPageIoDone, this, std::placeholders::_1);
        revMapPageAsyncIoReq->mpageNum = pageNum++;
        revMapPageAsyncIoReq->vsid = vsid;
        int ret = revMapfile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), "[ReverseMapPack] Error!, Calling AsyncIO Failed at RevMap LOAD, mpageNum:{}",
                revMapPageAsyncIoReq->mpageNum);
            ioError = ret;
            mapFlushState = MapFlushState::FLUSH_DONE;
            callback = nullptr;
            break;
        }
    }
    return ioError;
}

int
ReverseMapPack::Flush(Stripe* stripe, uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid)
{
    ioDirection = IO_FLUSH;
    ioError = 0;
    mfsAsyncIoDonePages = 0;
    uint64_t pageNum = 0;
    assert(callback == nullptr);
    callback = cb;
    for (auto& revMap : revMaps)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx();
        revMapPageAsyncIoReq->opcode = MetaFsIoOpcode::Write;
        revMapPageAsyncIoReq->fd = revMapfile->GetFd();
        revMapPageAsyncIoReq->fileOffset = fileOffset + (uint64_t)mpageSize * pageNum;
        revMapPageAsyncIoReq->length = mpageSize;
        revMapPageAsyncIoReq->buffer = (char*)revMap;
        revMapPageAsyncIoReq->callback = std::bind(&ReverseMapPack::_RevMapPageIoDone,
            this, std::placeholders::_1);
        revMapPageAsyncIoReq->mpageNum = pageNum++;
        revMapPageAsyncIoReq->stripeToFlush = stripe;
        revMapPageAsyncIoReq->vsid = vsid;
        int ret = revMapfile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
                "[ReverseMapPack] Error!, Calling AsyncIO Failed at RevMap FLUSH, mpageNum:{}",
                revMapPageAsyncIoReq->mpageNum);
            ioError = ret;
            mapFlushState = MapFlushState::FLUSH_DONE;
            callback = nullptr;
            break;
        }
        issuedIoCnt++;
    }

    if (telemetryPublisher)
    {
        POSMetric metric(TEL33011_MAP_REVERSE_FLUSH_IO_ISSUED_CNT, POSMetricTypes::MT_COUNT);
        metric.SetCountValue(issuedIoCnt);
        telemetryPublisher->PublishMetric(metric);
        issuedIoCnt = 0;
    }

    return ioError;
}

int
ReverseMapPack::SetReverseMapEntry(uint64_t offset, BlkAddr rba, uint32_t volumeId)
{
    uint32_t pageIndex;
    uint32_t sectorIndex;
    uint32_t entryIndex;
    std::tie(pageIndex, sectorIndex, entryIndex) = _ReverseMapGeometry(offset);

    RevMapEntry& entry = revMaps[pageIndex]->sector[sectorIndex].u.body.entry[entryIndex];
    entry.u.entry.rba = rba;
    entry.u.entry.volumeId = volumeId;

    return 0;
}

std::tuple<BlkAddr, uint32_t>
ReverseMapPack::GetReverseMapEntry(uint64_t offset)
{
    uint32_t pageIndex;
    uint32_t sectorIndex;
    uint32_t entryIndex;
    std::tie(pageIndex, sectorIndex, entryIndex) = _ReverseMapGeometry(offset);

    RevMapEntry& entry = revMaps[pageIndex]->sector[sectorIndex].u.body.entry[entryIndex];
    BlkAddr rba = entry.u.entry.rba;
    uint32_t volumeId = entry.u.entry.volumeId;

    return std::make_tuple(rba, volumeId);
}

void
ReverseMapPack::WaitPendingIoDone(void)
{
    while (mapFlushState == MapFlushState::FLUSHING)
    {
    }
}

void
ReverseMapPack::_SetHeader(StripeId wblsid_, StripeId vsid_)
{
    RevMapSector& hdr = revMaps[0]->sector[0];
    hdr.u.header.magic = REVMAP_MAGIC;
    hdr.u.header.wbLsid = wblsid_;
    hdr.u.header.vsid = vsid_;
}

void
ReverseMapPack::_RevMapPageIoDone(AsyncMetaFileIoCtx* ctx)
{
    RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = static_cast<RevMapPageAsyncIoCtx*>(ctx);
    if (revMapPageAsyncIoReq->error != 0)
    {
        ioError = revMapPageAsyncIoReq->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "[ReverseMapPack] Error!, MFS AsyncIO error, ioError:{} mpageNum:{}", ioError, revMapPageAsyncIoReq->mpageNum);
    }

    uint32_t res = mfsAsyncIoDonePages.fetch_add(1);
    if ((res + 1) == numMpagesPerStripe)
    {
        mapFlushState = MapFlushState::FLUSH_DONE;
        if ((ioDirection == IO_LOAD) && (revMapPageAsyncIoReq->mpageNum == 0))
        {
            RevMapSector& hdr = revMaps[0]->sector[0];
            RevMapSector& hdr2 = revMaps[0]->sector[1];
            if (hdr.u.header.magic != REVMAP_MAGIC)
            {
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[ReverseMapPack] Error!! WRONG SIGNATURE IN REVMAP HEADER, vsid:{}, hdvsid{}, sig:{}", revMapPageAsyncIoReq->vsid, hdr.u.header.vsid, hdr.u.header.magic);
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[ReverseMapPack] FirstEntry Data In ReverseMap, rba:{}, volumeId{}", hdr2.u.body.entry[0].u.entry.rba, hdr2.u.body.entry[0].u.entry.volumeId);
                assert(false);
            }
            else if (hdr.u.header.vsid != revMapPageAsyncIoReq->vsid)
            {
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[ReverseMapPack] WRONG VSID IN REVMAP HEADER, vsid:{}, hdvsid{}, sig:{}", revMapPageAsyncIoReq->vsid, hdr.u.header.vsid, hdr.u.header.magic);
                POS_TRACE_ERROR(EID(MAPPER_FAILED), "[ReverseMapPack] FirstEntry Data In ReverseMap, rba:{}, volumeId{}", hdr2.u.body.entry[0].u.entry.rba, hdr2.u.body.entry[0].u.entry.volumeId);
                assert(false);
            }
            else
            {
                assert(hdr.u.header.vsid == revMapPageAsyncIoReq->vsid);
                vsid = hdr.u.header.vsid;
                wbLsid = hdr.u.header.wbLsid;
            }
        }
        if (callback != nullptr)
        {
            EventSchedulerSingleton::Instance()->EnqueueEvent(callback);
        }
    }

    delete ctx;

    if (telemetryPublisher)
    {
        POSMetric metric(TEL33012_MAP_REVERSE_FLUSH_IO_DONE_CNT, POSMetricTypes::MT_COUNT);
        metric.SetCountValue(1);
        telemetryPublisher->PublishMetric(metric);
    }
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

} // namespace pos
