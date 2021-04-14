#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"

namespace pos
{
class MockSsdMetaVolume : public SsdMetaVolume
{
public:
    using SsdMetaVolume::SsdMetaVolume;
    MOCK_METHOD(void, InitVolumeBaseLpn, (), (override));
    MOCK_METHOD(bool, IsOkayToStore, (FileSizeType fileByteSize, MetaFilePropertySet& prop), (override));
};

} // namespace pos
