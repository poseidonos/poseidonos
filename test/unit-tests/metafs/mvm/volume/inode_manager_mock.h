#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>
#include <unordered_map>

#include "src/metafs/mvm/volume/inode_manager.h"

namespace pos
{
class MockInodeManager : public InodeManager
{
public:
    using InodeManager::InodeManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(void, SetMetaFileBaseLpn, (MetaLpnType lpn));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionBaseLpn, (MetaRegionType regionType));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (MetaRegionType regionType));
    MOCK_METHOD(MetaLpnType, GetMetaFileBaseLpn, ());
    MOCK_METHOD(void, CreateInitialInodeContent, (uint32_t maxInodeNum));
    MOCK_METHOD(bool, LoadInodeContent, ());
    MOCK_METHOD(bool, BackupContent,
        (MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts,
            MetaLpnType iNodeTableLpnCnts));
    MOCK_METHOD(bool, RestoreContent,
        (MetaVolumeType tgtVol, MetaLpnType BaseLpn, MetaLpnType iNodeHdrLpnCnts,
            MetaLpnType iNodeTableLpnCnts));
    MOCK_METHOD(POS_EVENT_ID, AddFileInActiveList, (FileDescriptorType fd));
    MOCK_METHOD(bool, CheckFileInActive, (FileDescriptorType fd));
    MOCK_METHOD(void, RemoveFileFromActiveList, (FileDescriptorType fd));
    MOCK_METHOD(size_t, GetFileCountInActive, ());
    MOCK_METHOD(void, PopulateFDMapWithVolumeType, (FileDescriptorInVolume& dest));
    MOCK_METHOD(void, PopulateFileNameWithVolumeType, (FileHashInVolume& dest));
    MOCK_METHOD(uint32_t, GetExtent,
        (const FileDescriptorType fd, std::vector<MetaFileExtent>& extents));
};

} // namespace pos
