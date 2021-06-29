#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/meta_volume_handler.h"

namespace pos
{
class MockMetaVolumeHandler : public MetaVolumeHandler
{
public:
    using MetaVolumeHandler::MetaVolumeHandler;
};

} // namespace pos
