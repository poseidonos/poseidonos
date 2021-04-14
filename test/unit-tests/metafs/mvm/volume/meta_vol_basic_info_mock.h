#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/meta_vol_basic_info.h"

namespace pos
{
class MockVolumeBasicInfo : public VolumeBasicInfo
{
public:
    using VolumeBasicInfo::VolumeBasicInfo;
};

} // namespace pos
