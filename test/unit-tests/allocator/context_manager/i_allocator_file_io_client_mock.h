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
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx* ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(std::string, GetFilename, (), (override));
    MOCK_METHOD(uint32_t, GetSignature, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
};

} // namespace pos
