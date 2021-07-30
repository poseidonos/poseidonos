#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/meta_file_intf/meta_file_intf.h"

namespace pos
{
class MockMetaFileIntf : public MetaFileIntf
{
public:
    using MetaFileIntf::MetaFileIntf;
    MOCK_METHOD(int, Create, (uint64_t size), (override));
    MOCK_METHOD(bool, DoesFileExist, (), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(uint64_t, GetFileSize, (), (override));
    MOCK_METHOD(int, AsyncIO, (AsyncMetaFileIoCtx * ctx), (override));
    MOCK_METHOD(int, CheckIoDoneStatus, (void* data), (override));
    MOCK_METHOD(int, Open, (), (override));
    MOCK_METHOD(int, Close, (), (override));
    MOCK_METHOD(bool, IsOpened, (), (override));
    MOCK_METHOD(int, GetFd, (), (override));
    MOCK_METHOD(std::string, GetFileName, (), (override));
    MOCK_METHOD(int, IssueIO, (MetaFsIoOpcode opType, uint64_t fileOffset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(int, AppendIO, (MetaFsIoOpcode opType, uint64_t& offset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(int, _Read, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
    MOCK_METHOD(int, _Write, (int fd, uint64_t fileOffset, uint64_t length, char* buffer), (override));
};

} // namespace pos
