#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/i_allocator_file_io_client.h"

namespace pos
{
class MockIAllocatorFileIoClient : public IAllocatorFileIoClient
{
public:
    using IAllocatorFileIoClient::IAllocatorFileIoClient;
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (char* buf), (override));
    MOCK_METHOD(void, AfterFlush, (char* buf), (override));
    MOCK_METHOD(ContextSectionAddr, GetSectionInfo, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
    MOCK_METHOD(uint64_t, GetTotalDataSize, (), (override));
};

} // namespace pos
