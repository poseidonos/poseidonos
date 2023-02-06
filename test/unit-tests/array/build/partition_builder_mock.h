#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/build/partition_builder.h"

namespace pos
{
class MockPartitionBuilder : public PartitionBuilder
{
public:
    using PartitionBuilder::PartitionBuilder;
};

} // namespace pos
