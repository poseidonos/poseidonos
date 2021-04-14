#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_container.h"

namespace pos
{
class MockMetaVolumeContainer : public MetaVolumeContainer
{
public:
    using MetaVolumeContainer::MetaVolumeContainer;
};

} // namespace pos
