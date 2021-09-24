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
    MOCK_METHOD(POS_EVENT_ID, Create,
        (std::string & fileName, uint64_t fileByteSize,
            MetaFilePropertySet prop, StorageOpt storage),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Delete,
        (std::string & fileName, StorageOpt storage),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Open,
        (std::string & fileName, int& fd, StorageOpt storage),
        (override));
    MOCK_METHOD(POS_EVENT_ID, Close,
        (FileDescriptorType fd, StorageOpt storage),
        (override));
    MOCK_METHOD(POS_EVENT_ID, CheckFileExist,
        (std::string & fileName, StorageOpt storage),
        (override));
    MOCK_METHOD(size_t, GetFileSize,
        (int fd, StorageOpt storage),
        (override));
    MOCK_METHOD(size_t, GetAlignedFileIOSize,
        (int fd, StorageOpt storage),
        (override));
    MOCK_METHOD(size_t, EstimateAlignedFileIOSize,
        (MetaFilePropertySet & prop, StorageOpt storage),
        (override));
    MOCK_METHOD(size_t, GetAvailableSpace,
        (MetaFilePropertySet & prop, StorageOpt storage),
        (override));
    MOCK_METHOD(size_t, GetMaxMetaLpn, (MetaVolumeType type), (override));
    MOCK_METHOD(void, SetStatus, (bool isNormal));
    MOCK_METHOD(std::vector<MetaFileInfoDumpCxt>, Wbt_GetMetaFileList,
        (MetaVolumeType type),
        (override));
    MOCK_METHOD(MetaFileInodeInfo*, Wbt_GetMetaFileInode,
        (std::string & fileName, MetaVolumeType type),
        (override));
    MOCK_METHOD(void, SetMss,
        (MetaStorageSubsystem * metaStorage));
    MOCK_METHOD(void, InitVolume,
        (MetaVolumeType volType, int arrayId, MetaLpnType maxVolPageNum),
        (override));
    MOCK_METHOD(bool, CreateVolume, (MetaVolumeType volType), (override));
    MOCK_METHOD(bool, OpenVolume, (bool isNPOR), (override));
    MOCK_METHOD(bool, CloseVolume, (bool& isNPOR), (override));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (MetaVolumeType type));
};

} // namespace pos
