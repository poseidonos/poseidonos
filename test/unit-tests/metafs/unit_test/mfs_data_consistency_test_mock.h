#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_data_consistency_test.h"

namespace pos
{
class MockMetaFSNonFuncDataConsistency : public MetaFSNonFuncDataConsistency
{
public:
    using MetaFSNonFuncDataConsistency::MetaFSNonFuncDataConsistency;
    MOCK_METHOD(void, SetUp, (), (override));
    MOCK_METHOD(void, TearDown, (), (override));
};

} // namespace pos
