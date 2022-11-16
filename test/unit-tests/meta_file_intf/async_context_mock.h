#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/meta_file_intf/async_context.h"

namespace pos
{
class MockAsyncMetaFileIoCtx : public AsyncMetaFileIoCtx
{
public:
    using AsyncMetaFileIoCtx::AsyncMetaFileIoCtx;
    MOCK_METHOD(void, HandleIoComplete, (void* data), (override));
    MOCK_METHOD(char*, GetBuffer, (), (override));
    MOCK_METHOD(MetaIoCbPtr, GetCallback, (), (override));
    MOCK_METHOD(int, GetError, (), (const, override));
    MOCK_METHOD(uint64_t, GetLength, (), (const, override));
    MOCK_METHOD(MetaFsIoOpcode, GetOpcode, (), (const, override));
    MOCK_METHOD(int, GetFd, (), (const, override));
    MOCK_METHOD(uint64_t, GetFileOffset, (), (const, override));
    MOCK_METHOD(std::string, ToString, (), (const, override));
    MOCK_METHOD(bool, IsReadyToUse, (), (const, override));
    MOCK_METHOD(void, SetIoInfo, (MetaFsIoOpcode opcode, uint64_t fileOffset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(void, SetFileInfo, (int fd, MetaFileIoCbPtr ioDoneCheckCallback), (override));
    MOCK_METHOD(void, SetCallback, (MetaIoCbPtr callback), (override));
};

} // namespace pos
