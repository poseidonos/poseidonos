#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/io/frontend_io/block_map_update_completion.h"

namespace pos
{
class MockBlockMapUpdateCompletion : public BlockMapUpdateCompletion
{
public:
    using BlockMapUpdateCompletion::BlockMapUpdateCompletion;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
