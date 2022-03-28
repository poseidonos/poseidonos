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
    MOCK_METHOD(POS_EVENT_ID, Read,
        (FileDescriptorType fd, void* buf, MetaStorageType mediaType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Read,
        (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize,
            void* buf, MetaStorageType mediaType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Write,
        (FileDescriptorType fd, void* buf, MetaStorageType mediaType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Write,
        (FileDescriptorType fd, FileSizeType byteOffset, FileSizeType byteSize,
            void* buf, MetaStorageType mediaType),
        (override));
    MOCK_METHOD(POS_EVENT_ID, SubmitIO,
        (MetaFsAioCbCxt * cxt, MetaStorageType mediaType),
        (override));
    MOCK_METHOD(bool, AddArray, (const int arrayId, const MaxMetaLpnMapPerMetaStorage& map), (override));
    MOCK_METHOD(bool, RemoveArray, (const int arrayId), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* metaStorage));
};

} // namespace pos
