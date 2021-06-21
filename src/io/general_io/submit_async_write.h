#pragma once

#include <list>
#include <string>

#include "src/array/ft/buffer_entry.h"
#include "src/array/service/io_locker/i_io_locker.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"
#include "src/include/pos_event_id.h"
#include "src/include/smart_ptr_type.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_submit_interface/io_submit_handler_status.h"

namespace pos
{
class SubmitAsyncWrite
{
public:
    SubmitAsyncWrite(void);
    SubmitAsyncWrite(IIOLocker* locker, IIOTranslator* translator, IODispatcher* ioDispatcher);
    ~SubmitAsyncWrite(void);
    IOSubmitHandlerStatus Execute(std::list<BufferEntry>& bufferList,
        LogicalBlkAddr& startLSA, uint64_t blockCount,
        PartitionType partitionToIO, CallbackSmartPtr callback,
        int arrayId, bool needTrim);

private:
    IIOLocker* locker;
    IIOTranslator* translator;
    IODispatcher* ioDispatcher;

    IOSubmitHandlerStatus _CheckAsyncWriteError(POS_EVENT_ID eventId, int arrayId);
};
} // namespace pos
