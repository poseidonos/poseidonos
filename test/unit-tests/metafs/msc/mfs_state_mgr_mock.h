#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/mfs_state_mgr.h"

namespace pos
{
class MockMetaFsSystemStateTransitionEntry : public MetaFsSystemStateTransitionEntry
{
public:
    using MetaFsSystemStateTransitionEntry::MetaFsSystemStateTransitionEntry;
};

class MockMetaFsStateManager : public MetaFsStateManager
{
public:
    using MetaFsStateManager::MetaFsStateManager;
};

} // namespace pos
