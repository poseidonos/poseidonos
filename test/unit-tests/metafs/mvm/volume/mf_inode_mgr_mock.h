#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/mf_inode_mgr.h"

namespace pos
{
class MockMetaFileInodeManager : public MetaFileInodeManager
{
public:
    using MetaFileInodeManager::MetaFileInodeManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem * metaStorage), (override));
};

} // namespace pos
