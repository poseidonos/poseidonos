#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/metafs_file_intf.h"

namespace pos
{
class MockMetaFsFileIntf : public MetaFsFileIntf
{
public:
    using MetaFsFileIntf::MetaFsFileIntf;
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
