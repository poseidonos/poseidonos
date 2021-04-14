#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mdi/mdi_manager.h"

namespace pos
{
class MockMetaIntegrityManager : public MetaIntegrityManager
{
public:
    using MetaIntegrityManager::MetaIntegrityManager;
    MOCK_METHOD(bool, _IsSiblingModuleReady, (), (override));
};

} // namespace pos
