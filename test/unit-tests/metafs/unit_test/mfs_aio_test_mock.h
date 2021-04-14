#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/unit_test/mfs_aio_test.h"

namespace pos
{
class MockUtMetaFsTopAIOPositive : public UtMetaFsTopAIOPositive
{
public:
    using UtMetaFsTopAIOPositive::UtMetaFsTopAIOPositive;
};

} // namespace pos
