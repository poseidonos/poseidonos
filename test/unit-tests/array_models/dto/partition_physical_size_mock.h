#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/dto/partition_physical_size.h"

namespace pos
{
class MockPartitionPhysicalSize : public PartitionPhysicalSize
{
public:
    using PartitionPhysicalSize::PartitionPhysicalSize;
};

} // namespace pos
