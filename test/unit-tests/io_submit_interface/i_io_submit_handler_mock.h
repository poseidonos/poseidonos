#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io_submit_interface/i_io_submit_handler.h"
 
namespace pos
{
class MockIIOSubmitHandler : public IIOSubmitHandler
{
  public:
    using IIOSubmitHandler::IIOSubmitHandler;
    MOCK_METHOD(IOSubmitHandlerStatus, SyncIO, (IODirection direction, std::list<BufferEntry>& bufferList, LogicalBlkAddr& startLSA, uint64_t blockCount, PartitionType partitionToIO, std::string arrayName), (override));
    MOCK_METHOD(IOSubmitHandlerStatus, SubmitAsyncIO, (IODirection direction, std::list<BufferEntry>& bufferList, LogicalBlkAddr& startLSA, uint64_t blockCount, PartitionType partitionToIO, CallbackSmartPtr callback, std::string arrayName), (override));
};

}  // namespace pos