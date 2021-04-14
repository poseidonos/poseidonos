#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/metafs_manager_adapter.h"

namespace pos
{
template<typename RequestType, typename ReturnType, typename MetaFsManager>
class MockMetaFsManagerAdapter : public MetaFsManagerAdapter<RequestType, ReturnType, MetaFsManager>
{
public:
    using MetaFsManagerAdapter::MetaFsManagerAdapter;
};

} // namespace pos
