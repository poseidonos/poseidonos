#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/on_volume_meta_region_mgr.h"

namespace pos
{
class MockOnVolumeMetaRegionManager : public OnVolumeMetaRegionManager
{
public:
    using OnVolumeMetaRegionManager::OnVolumeMetaRegionManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(void, Init, (MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
};

} // namespace pos
