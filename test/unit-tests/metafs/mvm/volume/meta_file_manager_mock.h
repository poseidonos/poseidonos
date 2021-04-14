#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/meta_file_manager.h"

namespace pos
{
class MockMetaFileManager : public MetaFileManager
{
public:
    using MetaFileManager::MetaFileManager;
    MOCK_METHOD(void, Init, (MetaVolumeType volumeType, MetaLpnType regionBaseLpn, MetaLpnType maxVolumeLpn), (override));
    MOCK_METHOD(MetaLpnType, GetRegionSizeInLpn, (), (override));
    MOCK_METHOD(void, Bringup, (), (override));
    MOCK_METHOD(void, Finalize, (), (override));
    MOCK_METHOD(bool, SaveContent, (), (override));
};

} // namespace pos
