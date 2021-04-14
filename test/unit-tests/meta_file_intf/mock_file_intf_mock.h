#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_file_intf/mock_file_intf.h"

namespace pos
{
class MockMockFileIntf : public MockFileIntf
{
public:
    using MockFileIntf::MockFileIntf;
    MOCK_METHOD(int, Create, (uint64_t fileSize, StorageOpt storageOpt), (override));
    MOCK_METHOD(bool, DoesFileExist, (), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(uint64_t, GetFileSize, (), (override));
    MOCK_METHOD(int, AsyncIO, (AsyncMetaFileIoCtx * ctx), (override));
    MOCK_METHOD(int, CheckIoDoneStatus, (void* data), (override));
    MOCK_METHOD(int, Open, (), (override));
    MOCK_METHOD(int, Close, (), (override));
    MOCK_METHOD(int, _Read, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(int, _Write, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
};

} // namespace pos
