#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_functional_test.h"

namespace pos
{
class MockUtMetaFsTopFunctionalPositive : public UtMetaFsTopFunctionalPositive
{
public:
    using UtMetaFsTopFunctionalPositive::UtMetaFsTopFunctionalPositive;
};

class MockUtMetaFsTopFunctionalNegative : public UtMetaFsTopFunctionalNegative
{
public:
    using UtMetaFsTopFunctionalNegative::UtMetaFsTopFunctionalNegative;
};

} // namespace pos
