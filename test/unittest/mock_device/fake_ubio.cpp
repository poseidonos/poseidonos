#include "src/io/ubio.h"

#include <sstream>

#include "src/array/device/array_device.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"
#include "src/volume/volume_list.h"

namespace ibofos {

IoState initialState;
static const VirtualBlkAddr INVALID_VSA =
{
    .stripeId = UNMAP_STRIPE,
    .offset = 0
};

Ubio::Ubio(void *buffer, uint32_t unitCount)
:   dir(UbioDir::Read),
    error(0),
    size(BYTES_PER_UNIT * unitCount),
    endio(nullptr),
    remaining(1),
    ubioPrivate(nullptr),
    ibofIo(nullptr),
    allocatedBlockCount(0),
    vsaRangeCount(0),
    isParity(false),
    ioState(&initialState),
    isGCIo(false),
    isOldData(false), 
    callbackEvent(nullptr),
    vecIdx(0),
    vecOff(0),
    sectorRba(INVALID_RBA),
    volumeId(MAX_VOLUME_COUNT),
    isChaining(false),
    retry(false),
    vsa(INVALID_VSA),
    oldVsa(INVALID_VSA),
    requestContext(nullptr),
    origin(nullptr),
    isRebuild(false)
{
    pba.dev = nullptr;
    pba.lba = INVALID_LBA;

    SetAsyncMode();
    mem = nullptr;
    memoryOwnership = false;

    if (0 == unitCount)
    {
        return;
    }

    if (nullptr == buffer)
    {
        memoryOwnership = true;
        buffer = ibofos::Memory<BYTES_PER_UNIT>::Alloc(unitCount);
    }

    mem = buffer;
    memSize = unitCount * BYTES_PER_UNIT;
}

Ubio::~Ubio(void)
{
    if (memoryOwnership)
    {
        ibofos::Memory<>::Free(mem);
    }

    if(nullptr != callbackEvent)
    {
        delete callbackEvent;
    }
}

void
Ubio::SetAsyncMode(void)
{
    sync = false;
}

void
Ubio::SetEventCallback(Event *inputEvent)
{
    if (unlikely(nullptr != callbackEvent))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_CALLBACK_EVENT_ALREADY_SET;
        std::stringstream oldEvent, newEvent;
        oldEvent << "0x" << std::hex <<
                reinterpret_cast<uint64_t>(callbackEvent);
        newEvent << "0x" << std::hex <<
                reinterpret_cast<uint64_t>(inputEvent);
        IBOF_TRACE_WARN(static_cast<int>(eventId),
            IbofEventId::GetString(eventId), oldEvent.str(), newEvent.str());
    }

    callbackEvent = inputEvent;
}

void
Ubio::Complete(void)
{
    if (nullptr != endio)
    {
        (*endio)(this);
    }
}

void
Ubio::SetPba(PhysicalBlkAddr& pbaInput)
{
    if (unlikely(IsInvalidPba(pbaInput)))
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        return;
    }

    pba = pbaInput;
}

void*
Ubio::GetBuffer(uint32_t blockIndex, uint32_t sectorOffset) const
{
    if (unlikely(nullptr == mem))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_REQUEST_NULL_BUFFER,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_REQUEST_NULL_BUFFER));

        return mem;
    }

    uint64_t shiftSize = ChangeBlockToByte(blockIndex) +
                         ChangeSectorToByte(sectorOffset);

    if (unlikely(size <= shiftSize))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_REQUEST_OUT_RANGE,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_REQUEST_OUT_RANGE));
        return mem;
    }

    shiftSize += ChangeBlockToByte(vecIdx) + ChangeSectorToByte(vecOff);
    uint8_t *addr = reinterpret_cast<uint8_t *>(mem);
    addr += shiftSize;

    return addr;
}

void
Ubio::MarkDone(void)
{
    assert(true == sync);
    assert(false == syncDone);

    syncDone = true;
}

bool
Ubio::IsInvalidPba(PhysicalBlkAddr& inputPba)
{
    return (inputPba.dev == nullptr);
}

UBlockDevice*
Ubio::GetUBlock(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return pba.dev->uBlock;
}

ArrayDevice*
Ubio::GetDev(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }
    return pba.dev;
}

const PhysicalBlkAddr&
Ubio::GetPba(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }

    return pba;
}

uint64_t
Ubio::GetLba(void)
{
    if (unlikely(false == CheckPbaSet()))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UBIO_INVALID_PBA,
                IbofEventId::GetString(IBOF_EVENT_ID::UBIO_INVALID_PBA));
        throw IBOF_EVENT_ID::UBIO_INVALID_PBA;
    }

    return pba.lba;
}

bool
Ubio::CheckPbaSet(void)
{
    return (false == IsInvalidPba(pba));
}

void
Ubio::SetIoAbstraction(RequestContext* ctx)
{
    if (unlikely(CheckRequestContextSet()))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_DUPLICATE_IO_ABSTRACTION;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
        return;
    }
    requestContext = ctx;
}

RequestContext*
Ubio::GetRequestContext(void)
{
    if (unlikely(false == CheckRequestContextSet()))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UBIO_DUPLICATE_IO_ABSTRACTION;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
        return nullptr;
    }
    return requestContext;
}

bool
Ubio::CheckRequestContextSet(void)
{
    bool isRequestContextSet = (nullptr != requestContext);
    return isRequestContextSet;
}

}
