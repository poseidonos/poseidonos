#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_container.h"

namespace pos
{
class MockMetaVolumeContainer : public MetaVolumeContainer
{
public:
    using MetaVolumeContainer::MetaVolumeContainer;

    MOCK_METHOD(void, InitContext, (MetaVolumeType volumeType, int arrayId,
                MetaLpnType maxVolPageNum, MetaStorageSubsystem* metaStorage,
                MetaVolume* vol));

    MOCK_METHOD(bool, CreateVolume, (MetaVolumeType volumeType));
    MOCK_METHOD(bool, OpenAllVolumes, (bool isNPOR));
    MOCK_METHOD(bool, CloseAllVolumes, (bool& resetContext /*output*/));
    MOCK_METHOD(bool, IsGivenVolumeExist, (MetaVolumeType volType));

    MOCK_METHOD(bool, TrimData, (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));

    MOCK_METHOD(bool, CreateFile, (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));
    MOCK_METHOD(bool, DeleteFile, (MetaVolumeType volType, MetaFsFileControlRequest& reqMsg));

    MOCK_METHOD(size_t, GetAvailableSpace, (MetaVolumeType volType));
    MOCK_METHOD(bool, CheckFileInActive, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(bool, IsGivenFileCreated, (std::string& fileName));
    MOCK_METHOD(void, RemoveFileFromActiveList, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetFileSize, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(FileSizeType, GetDataChunkSize, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetFileBaseLpn, (MetaVolumeType volType, FileDescriptorType fd));
    MOCK_METHOD(MetaLpnType, GetMaxLpn, (MetaVolumeType volType));
    MOCK_METHOD(FileDescriptorType, LookupFileDescByName, (std::string& fileName));
    MOCK_METHOD(MetaFileInode&, GetInode, (FileDescriptorType fd, MetaVolumeType volumeType));
    MOCK_METHOD(void, GetInodeList, (std::vector<MetaFileInfoDumpCxt>*& fileInfoList));
    MOCK_METHOD(bool, CopyInodeToInodeInfo,
                    (FileDescriptorType fd, MetaVolumeType volumeType,
                    MetaFileInodeInfo* inodeInfo /* output */));

    MOCK_METHOD(VolumeAndResult, DetermineVolumeToCreateFile,
                    (FileSizeType fileByteSize, MetaFilePropertySet& prop,
                        MetaVolumeType volumeType));
    MOCK_METHOD(POS_EVENT_ID, LookupMetaVolumeType,
                    (FileDescriptorType fd, MetaVolumeType volumeType));
    MOCK_METHOD(POS_EVENT_ID, LookupMetaVolumeType,
                    (std::string& fileName, MetaVolumeType volumeType));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (MetaVolumeType volumeType));
};

} // namespace pos
