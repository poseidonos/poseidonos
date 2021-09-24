#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_manager.h"

namespace pos
{
class MockMetaVolumeManager : public MetaVolumeManager
{
public:
    using MetaVolumeManager::MetaVolumeManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volType, MetaLpnType maxVolPageNum), (override));
    MOCK_METHOD(bool, Bringup, (), (override));
    MOCK_METHOD(bool, Open, (bool isNPOR), (override));
    MOCK_METHOD(bool, Close, (bool& resetCxt), (override));
    MOCK_METHOD(bool, CreateVolume, (MetaVolumeType volType), (override));
    MOCK_METHOD(bool, Compaction, (bool isNPOR), (override));
    MOCK_METHOD(MetaLpnType, GetMaxMetaLpn, (MetaVolumeType mediaType), (override));
    MOCK_METHOD(POS_EVENT_ID, ProcessNewReq, (MetaFsFileControlRequest & reqMsg), (override));
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
    MOCK_METHOD(MetaLpnType, GetTheLastValidLpn, (MetaVolumeType volType));
};

} // namespace pos
