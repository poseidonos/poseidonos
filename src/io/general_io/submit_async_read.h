#pragma once

#include <list>
#include <string>

#include "src/array/ft/buffer_entry.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"
#include "src/include/pos_event_id.h"
#include "src/io/general_io/merged_io.h"
#include "src/io_submit_interface/io_submit_handler_status.h"

namespace pos
{
class SubmitAsyncRead
{
public:
    explicit SubmitAsyncRead(CallbackSmartPtr callback);
    SubmitAsyncRead(MergedIO* mergedIO, IIOTranslator* translator);
    ~SubmitAsyncRead(void);

    IOSubmitHandlerStatus Execute(std::list<BufferEntry>& bufferList,
        LogicalBlkAddr& startLSA, uint64_t blockCount,
        PartitionType partitionToIO, CallbackSmartPtr callback,
        std::string& arrayName);

private:
    MergedIO* mergedIO;
    IIOTranslator* translator;
};

} // namespace pos
