#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_wbt_test.h"

namespace pos
{
class MockUtMetaFsWBT : public UtMetaFsWBT
{
public:
    using UtMetaFsWBT::UtMetaFsWBT;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos
