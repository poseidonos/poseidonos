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
    MOCK_METHOD(void, SetMss, (MetaStorageSubsystem* mss));

    MOCK_METHOD(bool, IsValidMBRExist, ());
    MOCK_METHOD(uint64_t, GetEpochSignature, ());
    MOCK_METHOD(bool, LoadMBR, ());
    MOCK_METHOD(bool, CreateMBR, ());
    MOCK_METHOD(void, RegisterVolumeGeometry, (MetaStorageInfo& mediaInfo));
    MOCK_METHOD(MetaFsStorageIoInfoList&, GetAllStoragePartitionInfo, ());

    MOCK_METHOD(void, SetPowerStatus, (bool isShutDownOff));
    MOCK_METHOD(bool, GetPowerStatus, ());
    MOCK_METHOD(void, InvalidMBR, ());
};

} // namespace pos
