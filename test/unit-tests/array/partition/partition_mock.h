#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/partition/partition.h"

namespace pos
{
class MockPartition : public Partition
{
public:
    using Partition::Partition;
    MOCK_METHOD(void, Format, (), (override));
};

} // namespace pos
