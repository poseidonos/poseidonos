#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/on_volume_meta_region.h"

namespace pos
{
template<typename VolMetaRegionT, typename MetaContentT>
class MockOnVolumeMetaRegion : public OnVolumeMetaRegion<VolMetaRegionT, MetaContentT>
{
public:
    using OnVolumeMetaRegion::OnVolumeMetaRegion;
};

} // namespace pos
