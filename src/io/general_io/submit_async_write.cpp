#include "src/io/general_io/submit_async_write.h"

#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/event_scheduler/callback.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/io_error_type.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/array_unlocking.h"
#include "src/io/general_io/internal_write_completion.h"
#include "src/logger/logger.h"
#include "src/state/state_manager.h"
/*To do Remove after adding array Idx by Array*/
#include "src/array_mgmt/array_manager.h"
namespace pos
{
SubmitAsyncWrite::SubmitAsyncWrite(void)
: SubmitAsyncWrite(IOLockerSingleton::Instance(),
      ArrayService::Instance()->Getter()->GetTranslator(),
      IODispatcherSingleton::Instance())
{
}

SubmitAsyncWrite::SubmitAsyncWrite(IIOLocker* locker, IIOTranslator* translator, IODispatcher* ioDispatcher)
: locker(locker),
  translator(translator),
  ioDispatcher(ioDispatcher)
{
}

SubmitAsyncWrite::~SubmitAsyncWrite(void)
{
}

IOSubmitHandlerStatus
SubmitAsyncWrite::Execute(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback,
    int arrayId, bool needTrim)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    if (bufferList.empty())
    {
        return errorToReturn;
    }
    LogicalWriteEntry logicalWriteEntry = {
        .addr = startLSA,
        .blkCnt = static_cast<uint32_t>(blockCount),
        .buffers = &bufferList};

    std::list<PhysicalWriteEntry> physicalWriteEntries;
    int ret = translator->Convert(
        arrayId, partitionToIO, physicalWriteEntries, logicalWriteEntry);
    if (ret != 0)
    {
        callback->InformError(IOErrorType::GENERIC_ERROR);
        EventSchedulerSingleton::Instance()->EnqueueEvent(callback);
        return IOSubmitHandlerStatus::SUCCESS;
    }

    std::set<IArrayDevice*> targetDevices;
    StripeId stripeId = startLSA.stripeId;
    if (partitionToIO == PartitionType::META_SSD)
    {
        for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
        {
            targetDevices.insert(physicalWriteEntry.addr.arrayDev);
        }
        bool result = locker->TryLock(targetDevices, stripeId);
        if (result == false)
        {
            return IOSubmitHandlerStatus::TRYLOCK_FAIL;
        }
    }

    uint32_t totalIoCount = 0;

    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        totalIoCount += physicalWriteEntry.buffers.size();
    }
    callback->SetWaitingCount(1);
    CallbackSmartPtr arrayUnlocking(
        new ArrayUnlocking(targetDevices, stripeId, locker));
    arrayUnlocking->SetCallee(callback);
    arrayUnlocking->SetWaitingCount(totalIoCount);
    arrayUnlocking->SetEventType(callback->GetEventType());
    for (PhysicalWriteEntry& physicalWriteEntry : physicalWriteEntries)
    {
        for (BufferEntry& buffer : physicalWriteEntry.buffers)
        {
            UbioSmartPtr ubio(new Ubio(buffer.GetBufferPtr(),
                buffer.GetBlkCnt() * Ubio::UNITS_PER_BLOCK, arrayId));
            if (needTrim == false)
            {
                ubio->dir = UbioDir::Write;
            }
            else
            {
                ubio->dir = UbioDir::Deallocate;
            }
            ubio->SetPba(physicalWriteEntry.addr);
            CallbackSmartPtr event(new InternalWriteCompletion(buffer));
            ubio->SetEventType(callback->GetEventType());
            event->SetEventType(ubio->GetEventType());
            event->SetCallee(arrayUnlocking);
            ubio->SetCallback(event);

            if (ioDispatcher->Submit(ubio) < 0)
            {
                errorToReturn =
                    _CheckAsyncWriteError(POS_EVENT_ID::REF_COUNT_RAISE_FAIL, arrayId);
                continue;
            }
        }
    }
    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }

    return errorToReturn;
}

IOSubmitHandlerStatus
SubmitAsyncWrite::_CheckAsyncWriteError(POS_EVENT_ID eventId, int arrayId)
{
    /*To do Remove after adding array Idx by Array*/
    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayId);

    IStateControl* stateControl = StateManagerSingleton::Instance()->GetStateControl(info->GetName());
    if (stateControl->GetState()->ToStateType() == StateEnum::STOP)
    {
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId));
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }

    return IOSubmitHandlerStatus::SUCCESS;
}
} // namespace pos
