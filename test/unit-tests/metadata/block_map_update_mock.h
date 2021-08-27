#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/metadata/block_map_update.h"

namespace pos
{
class MockBlockMapUpdate : public BlockMapUpdate
{
public:
    using BlockMapUpdate::BlockMapUpdate;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
