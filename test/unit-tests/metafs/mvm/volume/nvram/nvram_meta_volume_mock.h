#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/nvram/nvram_meta_volume.h"

namespace pos
{
class MockNvRamMetaVolume : public NvRamMetaVolume
{
public:
    using NvRamMetaVolume::NvRamMetaVolume;
    MOCK_METHOD(bool, IsOkayToStore, (FileSizeType fileByteSize, MetaFilePropertySet& prop), (override));
    MOCK_METHOD(void, InitVolumeBaseLpn, (), (override));
};

} // namespace pos
