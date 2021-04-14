#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/unit_test/mim_io_range_overlap_chker_test.h"

namespace pos
{
class MockUtMIMRangeLockChker : public UtMIMRangeLockChker
{
public:
    using UtMIMRangeLockChker::UtMIMRangeLockChker;
};

} // namespace pos
