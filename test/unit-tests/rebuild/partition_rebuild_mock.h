#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/rebuild/partition_rebuild.h"

namespace pos
{
class MockPartitionRebuild : public PartitionRebuild
{
public:
    using PartitionRebuild::PartitionRebuild;
};

} // namespace pos
