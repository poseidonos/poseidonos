#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/metafs_mbr_mgr.h"

namespace pos
{
class MockMetaFsMBRManager : public MetaFsMBRManager
{
public:
    using MetaFsMBRManager::MetaFsMBRManager;
    MOCK_METHOD(void, Init, (MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
};

} // namespace pos
