#include <cassert>

#include "src/io/general_io/submit_async_byte_io.h"

#include "src/array/service/array_service_layer.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/io/general_io/io_submit_handler_count.h"

namespace pos
{
AsyncByteIO::AsyncByteIO(void)
: AsyncByteIO(ArrayService::Instance()->Getter()->GetTranslator())
{
}

AsyncByteIO::AsyncByteIO(IIOTranslator* translator)
: translator(translator)
{
}

AsyncByteIO::~AsyncByteIO(void)
{
}

void
AsyncByteIO::_CallbackFunc(void *callbackPtr)
{
    CallbackSmartPtr* callbackSmartPtr = static_cast<CallbackSmartPtr *>(callbackPtr);
    if (callbackPtr != nullptr)
    {
        bool flag = (*callbackSmartPtr)->Execute();
        if (unlikely(flag == false))
        {
            EventSchedulerSingleton::Instance()->EnqueueEvent(*callbackSmartPtr);
        }
        delete callbackSmartPtr;
    }
}

void*
AsyncByteIO::_GetReadAddress(LogicalByteAddr& startLSA,
    PartitionType partitionToIO,
    int arrayId)
{
    PhysicalByteAddr physicalByteAddr = {.byteAddress = 0x0};
    int ret = translator->ByteTranslate(arrayId, partitionToIO,
        physicalByteAddr, startLSA);
    if (ret != 0)
    {
        return nullptr;
    }
    return reinterpret_cast<void*>(physicalByteAddr.byteAddress);
}

void*
AsyncByteIO::_GetWriteAddress(LogicalByteAddr& startLSA,
    PartitionType partitionToIO,
    int arrayId)
{
    std::list<BufferEntry> bufferList;
    // for nvm partition, there is no usage for byteCnt
    LogicalByteWriteEntry logicalByteAddr = {.addr = startLSA,
        .byteCnt = startLSA.byteSize,
        .buffers = &bufferList};
    std::list<PhysicalByteWriteEntry> physicalByteAddrList;
    int ret = translator->ByteConvert(arrayId, partitionToIO,
        physicalByteAddrList, logicalByteAddr);
    if (ret != 0 || physicalByteAddrList.size() == 0)
    {
        return nullptr;
    }
    return reinterpret_cast<void*>(physicalByteAddrList.front().addr.byteAddress);
}

IOSubmitHandlerStatus
AsyncByteIO::Execute(
    IODirection direction,
    void* buffer,
    LogicalByteAddr& startLSA,
    PartitionType partitionToIO,
    CallbackSmartPtr callback, int arrayId)
{
    uint32_t byteCount = startLSA.byteSize;
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;
    if (unlikely(byteCount == 0))
    {
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingByteIo--;
        return errorToReturn;
    }
    if (unlikely(buffer == nullptr))
    {
        int event = static_cast<int>(POS_EVENT_ID::IOSMHDLR_BYTEIO_BUFFER_NULLPTR);
        POS_TRACE_ERROR(event,
            PosEventId::GetString(POS_EVENT_ID::IOSMHDLR_BYTEIO_BUFFER_NULLPTR));
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingByteIo--;
        return errorToReturn;
    }

    if (unlikely(partitionToIO != PartitionType::META_NVM
        && partitionToIO != PartitionType::WRITE_BUFFER))
    {
        int event
            = static_cast<int>(POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_IS_NOT_BYTE_ACCESSIBLE);
        POS_TRACE_ERROR(event,
            PosEventId::GetString(POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_IS_NOT_BYTE_ACCESSIBLE));
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingByteIo--;
        return errorToReturn;
    }

    void* srcBuffer = nullptr;
    void* dstBuffer = nullptr;

    if (direction ==  IODirection::READ)
    {
        srcBuffer = _GetReadAddress(startLSA, partitionToIO, arrayId);
        dstBuffer = buffer;
    }
    else if (direction == IODirection::WRITE)
    {
        srcBuffer = buffer;
        dstBuffer = _GetWriteAddress(startLSA, partitionToIO, arrayId);
    }
    else
    {
        int event
            = static_cast<int>(POS_EVENT_ID::IOSMHDLR_BYTEIO_DIR_NOT_SUPORTTED);
        POS_TRACE_ERROR(event,
            PosEventId::GetString(POS_EVENT_ID::IOSMHDLR_BYTEIO_DIR_NOT_SUPORTTED),
            static_cast<int>(direction));
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingByteIo--;
        return errorToReturn;
    }

    if (unlikely(srcBuffer == nullptr || dstBuffer == nullptr))
    {
        int event
            = static_cast<int>(POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_TRANSLATE_ERROR);
        POS_TRACE_ERROR(event,
            PosEventId::GetString(POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_TRANSLATE_ERROR));
        IOSubmitHandlerCountSingleton::Instance()->callbackNotCalledCount++;
        IOSubmitHandlerCountSingleton::Instance()->pendingByteIo--;
        return errorToReturn;
    }
    CallbackSmartPtr* callbackSmartPtr = new CallbackSmartPtr(callback);

    // ToDo : It needs to be replaced abstracted api to support also nvram cache flush.
    AccelEngineApi::SubmitCopy(dstBuffer, srcBuffer, byteCount,
        _CallbackFunc, callbackSmartPtr);
    return IOSubmitHandlerStatus::SUCCESS;
}

} // namespace pos
