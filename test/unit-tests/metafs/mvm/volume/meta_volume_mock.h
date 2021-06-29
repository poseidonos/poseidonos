#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/meta_volume.h"

namespace pos
{
class MockMetaVolume : public MetaVolume
{
public:
    using MetaVolume::MetaVolume;
    MOCK_METHOD(void, InitVolumeBaseLpn, (), (override));
    MOCK_METHOD(bool, IsOkayToStore, (FileSizeType fileByteSize, MetaFilePropertySet& prop), (override));
};

} // namespace pos
