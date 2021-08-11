#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/rebuild/partition_rebuild.h"

namespace pos
{
class MockPartitionRebuild : public PartitionRebuild
{
public:
    using PartitionRebuild::PartitionRebuild;
    MOCK_METHOD(void, Start, (RebuildComplete cb), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(uint64_t, TotalStripes, (), (override));
};

} // namespace pos
