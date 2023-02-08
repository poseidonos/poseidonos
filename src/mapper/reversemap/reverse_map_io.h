#pragma once

#include <atomic>

#include "src/debug_lib/dump_shared_ptr.h"
#include "src/include/address_type.h"
#include "src/include/smart_ptr_type.h"
#include "src/mapper/include/mapper_const.h"
#include "src/meta_file_intf/async_context.h"
namespace pos
{
enum IoDirection
{
    IO_FLUSH,
    IO_LOAD,
    NUM_DIRECTIONS,
};

class ReverseMapPack;
class MetaFileIntf;
class TelemetryPublisher;
class EventScheduler;

class ReverseMapIo : public DumpSharedPtr<ReverseMapIo*, static_cast<int>(DumpSharedPtrType::REVERSE_MAP_IO)>
{
public:
    ReverseMapIo(ReverseMapPack* pack, EventSmartPtr clientCallback,
        MetaFileIntf* file, uint64_t offset, IoDirection direction,
        TelemetryPublisher* tp, EventScheduler* eventScheduler,
        std::function<void(ReverseMapIo*)> notify);
    virtual ~ReverseMapIo(void) = default;

    virtual int Load(void);
    virtual int Flush(void);
    virtual void WaitPendingIoDone(void);

    uint64_t GetNumIssuedIoCnt(void)
    {
        return issuedIoCnt;
    }
    MapFlushState GetMapFlushState(void)
    {
        return mapFlushState;
    }
    virtual IoDirection GetIoDirection(void)
    {
        return ioDirection;
    }

private:
    void _RevMapPageIoDone(AsyncMetaFileIoCtx* ctx);

    ReverseMapPack* revMapPack;
    IoDirection ioDirection;
    uint64_t fileOffset;
    MetaFileIntf* revMapFile;
    std::atomic<uint32_t> mfsAsyncIoDonePages;
    std::atomic<MapFlushState> mapFlushState;
    EventSmartPtr callback;
    uint64_t issuedIoCnt;
    uint64_t totalIoCnt;
    std::function<void(ReverseMapIo*)> notifyIoDone;

    TelemetryPublisher* telemetryPublisher;
    EventScheduler* eventSchceduler;
};

using ReverseMapIoPtr = std::shared_ptr<ReverseMapIo>;

class RevMapPageAsyncIoCtx : public AsyncMetaFileIoCtx
{
public:
    RevMapPageAsyncIoCtx(int mpageNum, StripeId vsid);
    virtual ~RevMapPageAsyncIoCtx(void) = default;

    int GetMpageNum(void) const;

private:
    int mpageNum;
    StripeId vsid;
};

} // namespace pos
