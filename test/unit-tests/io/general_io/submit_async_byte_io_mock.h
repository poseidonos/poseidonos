#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/general_io/submit_async_byte_io.h"

namespace pos
{
class MockAsyncByteIO : public AsyncByteIO
{
public:
    using AsyncByteIO::AsyncByteIO;
    MOCK_METHOD(void*, _GetReadAddress, (LogicalByteAddr& startLSA, PartitionType partitionToIO, int arrayId), (override));
    MOCK_METHOD(void*, _GetWriteAddress, (LogicalByteAddr& startLSA, PartitionType partitionToIO, int arrayId), (override));
};

} // namespace pos
