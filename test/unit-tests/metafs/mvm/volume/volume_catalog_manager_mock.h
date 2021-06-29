#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/volume_catalog_manager.h"

namespace pos
{
class MockVolumeCatalogManager : public VolumeCatalogManager
{
public:
    using VolumeCatalogManager::VolumeCatalogManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
};

} // namespace pos
