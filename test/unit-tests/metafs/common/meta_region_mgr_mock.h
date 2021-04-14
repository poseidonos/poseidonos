#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/meta_region_mgr.h"

namespace pos
{
class MockMetaRegionManager : public MetaRegionManager
{
public:
    using MetaRegionManager::MetaRegionManager;
    MOCK_METHOD(void, Init, (MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
};

} // namespace pos
