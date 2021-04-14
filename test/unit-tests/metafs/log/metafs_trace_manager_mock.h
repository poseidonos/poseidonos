#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/log/metafs_trace_manager.h"

namespace pos
{
class MockMetaFsTraceManager : public MetaFsTraceManager
{
public:
    using MetaFsTraceManager::MetaFsTraceManager;
};

} // namespace pos
