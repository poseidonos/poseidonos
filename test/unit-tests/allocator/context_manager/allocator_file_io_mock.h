#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/allocator_file_io.h"

namespace pos
{
class MockAllocatorFileIo : public AllocatorFileIo
{
public:
    using AllocatorFileIo::AllocatorFileIo;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, LoadContext, (), (override));
    MOCK_METHOD(int, Flush, (AllocatorCtxIoCompletion clientCallback, int dstSectionId, char* externalBuf), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(int, GetNumFilesReading, (), (override));
    MOCK_METHOD(int, GetNumFilesFlushing, (), (override));
};

} // namespace pos
