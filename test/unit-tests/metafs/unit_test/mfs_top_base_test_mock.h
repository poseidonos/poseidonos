#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_top_base_test.h"

namespace pos
{
class MockUtMetaFsTop : public UtMetaFsTop
{
public:
    using UtMetaFsTop::UtMetaFsTop;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos
