#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/metafs/mai/metafs_file_control_api.h"

namespace pos
{
class MockMetaFsFileControlApi : public MetaFsFileControlApi
{
public:
    using MetaFsFileControlApi::MetaFsFileControlApi;
    MOCK_METHOD(POS_EVENT_ID, Create, (std::string& fileName, uint64_t fileByteSize, MetaFilePropertySet prop), (override));
    MOCK_METHOD(POS_EVENT_ID, Delete, (std::string& fileName), (override));
    MOCK_METHOD(POS_EVENT_ID, Open, (std::string& fileName, int& fd), (override));
    MOCK_METHOD(POS_EVENT_ID, Close, (FileDescriptorType fd), (override));
    MOCK_METHOD(POS_EVENT_ID, CheckFileExist, (std::string& fileName), (override));
    MOCK_METHOD(size_t, GetFileSize, (int fd), (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize, (int fd), (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize, (MetaFilePropertySet& prop), (override));
    MOCK_METHOD(size_t, GetTheBiggestExtentSize, (MetaFilePropertySet& prop), (override));
    MOCK_METHOD(size_t, GetMaxMetaLpn, (MetaVolumeType type), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal), (override));
    MOCK_METHOD(std::vector<MetaFileInfoDumpCxt>, Wbt_GetMetaFileList, (), (override));
    MOCK_METHOD(FileSizeType, Wbt_GetMaxFileSizeLimit, (), (override));
    MOCK_METHOD(MetaFileInodeDumpCxt, Wbt_GetMetaFileInode, (std::string& fileName), (override));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* metaStorage), (override));
    MOCK_METHOD(void, InitVolume, (MetaVolumeType volType, std::string arrayName, MetaLpnType maxVolPageNum), (override));
    MOCK_METHOD(bool, CreateVolume, (MetaVolumeType volType), (override));
    MOCK_METHOD(bool, OpenVolume, (bool isNPOR), (override));
    MOCK_METHOD(bool, CloseVolume, (bool& isNPOR), (override));
    MOCK_METHOD(bool, Compaction, (bool isNPOR), (override));
};

} // namespace pos
