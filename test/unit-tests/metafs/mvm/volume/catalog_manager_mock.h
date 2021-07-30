#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/catalog_manager.h"

namespace pos
{
class MockCatalogManager : public CatalogManager
{
public:
    using CatalogManager::CatalogManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(bool, LoadVolCatalog, ());
    MOCK_METHOD(bool, CheckVolumeValidity, ());
    MOCK_METHOD(bool, BackupContent, (MetaVolumeType tgtVol, MetaLpnType baseLpn, MetaLpnType lpnCnts));
    MOCK_METHOD(bool, RestoreContent, (MetaVolumeType tgtVol, MetaLpnType baseLpn, MetaLpnType lpnCnts));
    MOCK_METHOD(bool, CreateCatalog, (MetaLpnType maxVolumeLpn, uint32_t maxFileSupportNum, bool save));
    MOCK_METHOD(void, RegisterRegionInfo, (MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn));
};

} // namespace pos
