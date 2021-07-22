#pragma once

#include <list>
#include <tuple>

#include "src/array/ft/buffer_entry.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"
#include "src/include/pos_event_id.h"
#include "src/io/general_io/merged_io.h"
#include "src/io_submit_interface/io_submit_handler_status.h"
#include "src/io_submit_interface/i_io_submit_handler.h"

namespace pos
{
using ByteAddressType = std::tuple<void*, void*>;
class AsyncByteIO
{
public:
    AsyncByteIO(void);
    explicit AsyncByteIO(IIOTranslator* translator);
    ~AsyncByteIO(void);

    IOSubmitHandlerStatus Execute(IODirection direction,
        void* buffer,
        LogicalByteAddr& startLSA,
        PartitionType partitionToIO,
        CallbackSmartPtr callback, int arrayId);

protected:
    virtual void* _GetReadAddress(LogicalByteAddr& startLSA,
        PartitionType partitionToIO,
        int arrayId);
    virtual void* _GetWriteAddress(LogicalByteAddr& startLSA,
        PartitionType partitionToIO,
        int arrayId);

private:
    static void _CallbackFunc(void *callbackPtr);
    IIOTranslator* translator;
};

} // namespace pos
