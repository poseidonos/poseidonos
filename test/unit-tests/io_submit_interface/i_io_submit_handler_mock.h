#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io_submit_interface/i_io_submit_handler.h"

namespace pos
{
class MockIIOSubmitHandler : public IIOSubmitHandler
{
public:
    using IIOSubmitHandler::IIOSubmitHandler;
    MOCK_METHOD(IOSubmitHandlerStatus, SyncIO, (IODirection direction, std::list<BufferEntry>& bufferList, LogicalBlkAddr& startLSA, uint64_t blockCount, PartitionType partitionToIO, int arrayId), (override));
    MOCK_METHOD(IOSubmitHandlerStatus, SubmitAsyncIO, (IODirection direction, std::list<BufferEntry>& bufferList, LogicalBlkAddr& startLSA, uint64_t blockCount, PartitionType partitionToIO, CallbackSmartPtr callback, int arrayId, bool parityOnly), (override));
    MOCK_METHOD(IOSubmitHandlerStatus, SubmitAsyncByteIO, (IODirection direction, void* buffer, LogicalByteAddr& startLSA, PartitionType partitionToIO, CallbackSmartPtr callback, int arrayId), (override));
};

} // namespace pos
