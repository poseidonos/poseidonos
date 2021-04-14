#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mai/metafs_file_control_api.h"

namespace pos
{
class MockMetaFsFileControlApi : public MetaFsFileControlApi
{
public:
    using MetaFsFileControlApi::MetaFsFileControlApi;
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, CreateVolume, (std::string & fileName, uint64_t fileByteSize, MetaFilePropertySet prop), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Delete, (std::string & fileName), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Open, (std::string & fileName), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, Close, (FileDescriptorType fd), (override));
    MOCK_METHOD(MetaFsReturnCode<POS_EVENT_ID>, CheckFileExist, (std::string & fileName), (override));
    MOCK_METHOD(size_t, GetFileSize, (int fd), (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize, (int fd), (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize, (MetaFilePropertySet & prop), (override));
    MOCK_METHOD(size_t, GetTheBiggestExtentSize, (MetaFilePropertySet & prop), (override));
};

} // namespace pos
