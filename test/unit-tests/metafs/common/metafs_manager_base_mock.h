#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/metafs_manager_base.h"

namespace pos
{
class MockMetaFsManagerBase : public MetaFsManagerBase
{
public:
    using MetaFsManagerBase::MetaFsManagerBase;
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
};

} // namespace pos
