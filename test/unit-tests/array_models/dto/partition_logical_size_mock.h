#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/dto/partition_logical_size.h"

namespace pos
{
class MockPartitionLogicalSize : public PartitionLogicalSize
{
public:
    using PartitionLogicalSize::PartitionLogicalSize;
};

} // namespace pos
