#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_context.h"

namespace pos
{
class MockMetaVolumeContext : public MetaVolumeContext
{
public:
    using MetaVolumeContext::MetaVolumeContext;
};

} // namespace pos
