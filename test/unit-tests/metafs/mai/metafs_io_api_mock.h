#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_io_api.h"

namespace pos
{
class MockMetaFsIoApi : public MetaFsIoApi
{
public:
    using MetaFsIoApi::MetaFsIoApi;
    MOCK_METHOD(POS_EVENT_ID, Read, (FileDescriptorType fd, void* buf), (override));
    MOCK_METHOD(POS_EVENT_ID, Read, (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf), (override));
    MOCK_METHOD(POS_EVENT_ID, Write, (FileDescriptorType fd, void* buf), (override));
    MOCK_METHOD(POS_EVENT_ID, Write, (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize, void* buf), (override));
    MOCK_METHOD(POS_EVENT_ID, SubmitIO, (MetaFsAioCbCxt * cxt), (override));
    MOCK_METHOD(bool, AddArray, (std::string & arrayName), (override));
    MOCK_METHOD(bool, RemoveArray, (std::string & arrayName), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal), (override));
};

} // namespace pos
