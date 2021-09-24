#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/meta_volume.h"

namespace pos
{
class MockMetaVolume : public MetaVolume
{
public:
    using MetaVolume::MetaVolume;
    MOCK_METHOD(void, InitVolumeBaseLpn, (), (override));
    MOCK_METHOD(bool, IsOkayToStore,
        (FileSizeType fileByteSize, MetaFilePropertySet& prop), (override));
    MOCK_METHOD(void, Init, (MetaStorageSubsystem* metaStorage));
    MOCK_METHOD(bool, CreateVolume, ());
    MOCK_METHOD(MetaVolumeType, GetVolumeType, ());
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, ());
    MOCK_METHOD(MetaLpnType, GetBaseLpn, ());
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (MetaRegionType regionType));
    MOCK_METHOD(bool, OpenVolume, (MetaLpnType* info, bool isNPOR));
    MOCK_METHOD(bool, CloseVolume, (MetaLpnType* info, bool& resetContext));
    MOCK_METHOD(bool, TrimData, (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(FileControlResult, CreateFile,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(FileControlResult, DeleteFile,
        (MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(size_t, GetAvailableSpace, ());
    MOCK_METHOD(bool, CheckFileInActive, (FileDescriptorType fd));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (FileDescriptorType fd));
    MOCK_METHOD(void, RemoveFileFromActiveList, (FileDescriptorType fd));
    MOCK_METHOD(bool, IsGivenFileCreated, (StringHashType fileKey));
    MOCK_METHOD(FileSizeType, GetFileSize,
        (FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetDataChunkSize,
        (FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn,
        (FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetMaxLpn,
        ());
    MOCK_METHOD(void, GetInodeList,
        (std::vector<MetaFileInfoDumpCxt>*& fileInfoList));
    MOCK_METHOD(MetaFileInode&, GetInode,
        (FileDescriptorType fd));
    MOCK_METHOD(bool, CopyInodeToInodeInfo,
        (FileDescriptorType fd, MetaFileInodeInfo* inodeInfo /* output */));
    MOCK_METHOD(std::string, LookupNameByDescriptor,
        (FileDescriptorType fd));
    MOCK_METHOD(FileDescriptorType, LookupDescriptorByName,
        (std::string& fileName));
    MOCK_METHOD(VolumeAndResult, DetermineVolumeToCreateFile,
        (FileSizeType fileByteSize,
            MetaFilePropertySet& prop, MetaVolumeType volumeType));
};

} // namespace pos
