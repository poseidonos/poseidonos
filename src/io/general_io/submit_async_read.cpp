#include "src/io/general_io/submit_async_read.h"

#include "src/array/service/array_service_layer.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
SubmitAsyncRead::SubmitAsyncRead(CallbackSmartPtr callback)
: SubmitAsyncRead(new MergedIO(callback),
      ArrayService::Instance()->Getter()->GetTranslator())
{
}

SubmitAsyncRead::SubmitAsyncRead(MergedIO* mergedIO, IIOTranslator* translator)
: mergedIO(mergedIO),
  translator(translator)
{
}
SubmitAsyncRead::~SubmitAsyncRead(void)
{
    if (nullptr != mergedIO)
    {
        delete mergedIO;
        mergedIO = nullptr;
    }
}

IOSubmitHandlerStatus
SubmitAsyncRead::Execute(
    std::list<BufferEntry>& bufferList,
    LogicalBlkAddr& startLSA, uint64_t blockCount,
    PartitionType partitionToIO,
    CallbackSmartPtr callback, std::string& arrayName)
{
    IOSubmitHandlerStatus errorToReturn = IOSubmitHandlerStatus::FAIL;

    if (bufferList.empty())
    {
        return errorToReturn;
    }

    std::list<BufferEntry>::iterator it = bufferList.begin();
    LogicalBlkAddr currentLSA = startLSA;
    callback->SetWaitingCount(blockCount);

    uint64_t blockCountFromBufferList = 0;
    for (auto& iter : bufferList)
    {
        blockCountFromBufferList += iter.GetBlkCnt();
    }

    if (unlikely(blockCount != blockCountFromBufferList))
    {
        if (blockCount < blockCountFromBufferList)
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT;
            POS_TRACE_WARN(eventId, PosEventId::GetString(eventId),
                blockCountFromBufferList, blockCount);
        }
        else
        {
            return errorToReturn;
        }
    }

    for (uint64_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        BufferEntry& currentBufferEntry = *it;

        uint32_t bufferCount =
            Min((blockCount - blockIndex), currentBufferEntry.GetBlkCnt());

        for (uint32_t bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
        {
            PhysicalBlkAddr physicalBlkAddr;

            // Ignore handling the return status.
            translator->Translate(
                arrayName, partitionToIO, physicalBlkAddr, currentLSA);

            if (mergedIO->IsContiguous(physicalBlkAddr))
            {
                mergedIO->AddContiguousBlock();
            }
            else
            {
                // Ignore handling the return status.
                mergedIO->Process(arrayName);

                void* newBuffer = currentBufferEntry.GetBlock(bufferIndex);
                mergedIO->SetNewStart(newBuffer, physicalBlkAddr);
            }

            currentLSA.offset++;
        }
        // Net status of the upper operations is gathered from here.
        errorToReturn = mergedIO->Process(arrayName);
        mergedIO->Reset();

        blockIndex += (bufferCount - 1);
        it++;
    }

    if (errorToReturn != IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP)
    {
        errorToReturn = IOSubmitHandlerStatus::SUCCESS;
    }
    return errorToReturn;
}

} // namespace pos
