#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/msc/unit_test/metafs_system_manager_test.h"

namespace pos
{
class MockMetaFsSystemManagerTest : public MetaFsSystemManagerTest
{
public:
    using MetaFsSystemManagerTest::MetaFsSystemManagerTest;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos
