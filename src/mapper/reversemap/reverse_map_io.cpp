#include "src/mapper/reversemap/reverse_map_io.h"

#include <vector>

#include "src/event_scheduler/event_scheduler.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/meta_file_intf/meta_file_intf.h"

namespace pos
{
ReverseMapIo::ReverseMapIo(ReverseMapPack* pack, EventSmartPtr clientCallback,
    MetaFileIntf* file, uint64_t offset, IoDirection direction,
    TelemetryPublisher* tp, EventScheduler* es,
    std::function<void(ReverseMapIo*)> notify)
: revMapPack(pack),
  ioDirection(direction),
  fileOffset(offset),
  revMapFile(file),
  mfsAsyncIoDonePages(0),
  mapFlushState(MapFlushState::FLUSH_DONE),
  callback(clientCallback),
  issuedIoCnt(0),
  totalIoCnt(0),
  notifyIoDone(notify),
  telemetryPublisher(tp),
  eventSchceduler(es)
{
}

int
ReverseMapIo::Load(void)
{
    mapFlushState = MapFlushState::FLUSHING; // TODO: use load enum

    uint64_t offset = fileOffset;
    uint64_t pageNum = 0;
    int ret = EID(SUCCESS);

    std::vector<ReverseMapPage> revMapPages = revMapPack->GetReverseMapPages();
    totalIoCnt = revMapPages.size();
    for (auto& page : revMapPages)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx(pageNum++, revMapPack->GetVsid());
        revMapPageAsyncIoReq->SetIoInfo(MetaFsIoOpcode::Read, offset, page.length, page.buffer);
        revMapPageAsyncIoReq->SetFileInfo(revMapFile->GetFd(), revMapFile->GetIoDoneCheckFunc());
        revMapPageAsyncIoReq->SetCallback(std::bind(&ReverseMapIo::_RevMapPageIoDone, this, std::placeholders::_1));

        offset += page.length;
        issuedIoCnt++;
        ret = revMapFile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), revMapPageAsyncIoReq->ToString());

            mapFlushState = MapFlushState::FLUSH_DONE;
            callback = nullptr;
            break;
        }

    }
    return ret;
}

int
ReverseMapIo::Flush(void)
{
    mapFlushState = MapFlushState::FLUSHING;

    uint64_t offset = fileOffset;
    uint64_t pageNum = 0;
    int ret = EID(SUCCESS);

    std::vector<ReverseMapPage> revMapPages = revMapPack->GetReverseMapPages();
    totalIoCnt = revMapPages.size();

    // we used local variable so that there is no problem with operation even if instance is deleted
    const uint64_t totalCount = totalIoCnt;
    uint64_t issuedCount = 0;

    if (telemetryPublisher)
    {
        POSMetric metric(TEL33011_MAP_REVERSE_FLUSH_IO_ISSUED_CNT, POSMetricTypes::MT_COUNT);
        metric.SetCountValue(totalIoCnt);
        telemetryPublisher->PublishMetric(metric);
    }

    for (auto& page : revMapPages)
    {
        RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = new RevMapPageAsyncIoCtx(pageNum++, revMapPack->GetVsid());
        revMapPageAsyncIoReq->SetIoInfo(MetaFsIoOpcode::Write, offset, page.length, page.buffer);
        revMapPageAsyncIoReq->SetFileInfo(revMapFile->GetFd(), revMapFile->GetIoDoneCheckFunc());
        revMapPageAsyncIoReq->SetCallback(std::bind(&ReverseMapIo::_RevMapPageIoDone,
            this, std::placeholders::_1));

        offset += page.length;
        issuedCount = (issuedIoCnt++);
        ret = revMapFile->AsyncIO(revMapPageAsyncIoReq);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR), revMapPageAsyncIoReq->ToString());

            mapFlushState = MapFlushState::FLUSH_DONE;
            callback = nullptr;
            break;
        }

    }

    if (totalCount != issuedCount)
    {
        // TODO: Publish error metric
        POS_TRACE_ERROR(EID(REVMAP_IO_ERROR),
            "Not all reverse map flushed due to error, total {}, issued {}",
            totalCount, issuedCount);
    }

    return ret;
}

void
ReverseMapIo::_RevMapPageIoDone(AsyncMetaFileIoCtx* ctx)
{
    RevMapPageAsyncIoCtx* revMapPageAsyncIoReq = static_cast<RevMapPageAsyncIoCtx*>(ctx);
    if (revMapPageAsyncIoReq->error != 0)
    {
        int ioError = revMapPageAsyncIoReq->error;
        POS_TRACE_ERROR(EID(MFS_ASYNCIO_ERROR),
            "[ReverseMapPack] Error!, MFS AsyncIO error, ioError:{} mpageNum:{}", ioError, revMapPageAsyncIoReq->GetMpageNum());
    }

    if (telemetryPublisher)
    {
        POSMetric metric(TEL33012_MAP_REVERSE_FLUSH_IO_DONE_CNT, POSMetricTypes::MT_COUNT);
        metric.SetCountValue(1);
        telemetryPublisher->PublishMetric(metric);
    }

    if ((ioDirection == IO_LOAD) && (revMapPageAsyncIoReq->GetMpageNum() == 0))
    {
        int ret = revMapPack->HeaderLoaded();
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(REVMAP_IO_ERROR), revMapPageAsyncIoReq->ToString());
            assert(false);
        }
    }

    // TODO: Handle zombie request (request that was not issued due to previous error)
    // we used local variable so that there is no problem with operation even if instance is deleted
    const uint64_t issuedCount = totalIoCnt;
    const uint32_t doneCount = mfsAsyncIoDonePages.fetch_add(1);
    if ((doneCount + 1) == issuedCount)
    {
        mapFlushState = MapFlushState::FLUSH_DONE;

        if (callback != nullptr)
        {
            EventSmartPtr enqueue = callback;
            callback = nullptr;
            eventSchceduler->EnqueueEvent(enqueue);
        }

        notifyIoDone(this);
        // NOTE that ReverseMapPack should not access to its private variables
        // after notify io done as it might delete this ReverseMapPack
    }

    delete ctx;
    // NOTE that ctx should not be deleted at the end of this function
}

void
ReverseMapIo::WaitPendingIoDone(void)
{
    while (mapFlushState == MapFlushState::FLUSHING)
    {
        usleep(1);
    }
}

RevMapPageAsyncIoCtx::RevMapPageAsyncIoCtx(int num, StripeId vsid_)
: mpageNum(num),
  vsid(vsid_)
{
}

int
RevMapPageAsyncIoCtx::GetMpageNum(void) const
{
    return mpageNum;
}

} // namespace pos
