#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/volume/mf_extent_mgr.h"

namespace pos
{
class MockMetaFileExtentManager : public MetaFileExtentManager
{
public:
    using MetaFileExtentManager::MetaFileExtentManager;
};

} // namespace pos
