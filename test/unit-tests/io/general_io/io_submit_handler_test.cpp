#include "src/io/general_io/io_submit_handler.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <list>
#include <string>

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(IOSubmitHandler, IOSubmitHandler_Constructor_Stack)
{
    // Given

    // When: Create ioSubmitHandler in stack
    IOSubmitHandler ioSubmitHandler();

    // Then: Do nothing
}

TEST(IOSubmitHandler, IOSubmitHandler_Constructor_Heap)
{
    // Given

    // When: Create ioSubmitHandler in heap
    IOSubmitHandler* ioSubmitHandler = new IOSubmitHandler();

    // Then: Do nothing
    delete ioSubmitHandler;
}

TEST(IOSubmitHandler, IOSubmitHander_SyncIO)
{
    // Given
    IOSubmitHandler ioSubmitHandler;
    std::list<BufferEntry> bufferList;
    LogicalBlkAddr startLSA = {0, 0};
    uint64_t blockCount = 0;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction;

    // When: SyncIO 
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SyncIO(direction, bufferList, startLSA, blockCount,
        partitionToIO, 0);
    
    // Then: fail return
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

TEST(IOSubmitHandler, IOSubmitHandler_SubmitAsyncIO_InvalidIoDirection)
{
    // Given
    IOSubmitHandler ioSubmitHandler;
    std::list<BufferEntry> bufferList;
    LogicalBlkAddr startLSA = {0, 10};
    uint64_t blockCount = 1;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction;

    // When: IODirection is invalid    
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SubmitAsyncIO(direction, bufferList, startLSA, blockCount,
        partitionToIO, callback, 0);
    
    // Then: fail return
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

TEST(IOSubmitHandler, IOSubmitHandler_AsyncRead)
{
    IOSubmitHandler ioSubmitHandler;
    std::list<BufferEntry> bufferList;
    LogicalBlkAddr startLSA = {0, 0};
    uint64_t blockCount = 0;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction = IODirection::READ;

    // When: IODirection is Read
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SubmitAsyncIO(direction, bufferList, startLSA, blockCount,
        partitionToIO, callback, 0);

    // Then: fail return
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

TEST(IOSubmitHandler, IOSubmitHandler_AsyncWrite)
{
    IOSubmitHandler ioSubmitHandler;
    std::list<BufferEntry> bufferList;
    LogicalBlkAddr startLSA = {0, 0};
    uint64_t blockCount = 0;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction = IODirection::WRITE;

    // When: IODirection is Write
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SubmitAsyncIO(direction, bufferList, startLSA, blockCount,
        partitionToIO, callback, 0);
    
    // Then: fail return
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

TEST(IOSubmitHandler, IOSubmitHandler_TrimData)
{
    IOSubmitHandler ioSubmitHandler;
    std::list<BufferEntry> bufferList;
    LogicalBlkAddr startLSA = {0, 0};
    uint64_t blockCount = 0;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction = IODirection::TRIM;

    // When: IODirection is trim
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SubmitAsyncIO(direction, bufferList, startLSA, blockCount,
        partitionToIO, callback, 0);
    
    // Then: fail return
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

TEST(IOSubmitHandler, IOSubmitHandler_SubmitAsyncByteIO)
{
    IOSubmitHandler ioSubmitHandler;

    uint64_t blockCount = 0;
    PartitionType partitionToIO = PartitionType::USER_DATA;
    CallbackSmartPtr callback = nullptr;
    IODirection direction = IODirection::READ;

    // When: IODirection is trim
    LogicalByteAddr startLSA;
    startLSA.byteOffset = 0;
    startLSA.byteSize = 8;
    IOSubmitHandlerStatus errorToReturn;
    errorToReturn = ioSubmitHandler.SubmitAsyncByteIO(direction, nullptr, startLSA,
        partitionToIO, callback, 0);

    // Then: fail return, success case is handled by submit_async_byte_io_test.
    EXPECT_EQ(errorToReturn, IOSubmitHandlerStatus::FAIL);
}

} // namespace pos
