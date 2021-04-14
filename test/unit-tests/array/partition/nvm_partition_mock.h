#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/partition/nvm_partition.h"

namespace pos
{
class MockNvmPartition : public NvmPartition
{
public:
    using NvmPartition::NvmPartition;
    MOCK_METHOD(int, Translate, (PhysicalBlkAddr & dst, const LogicalBlkAddr& src), (override));
    MOCK_METHOD(int, Convert, (list<PhysicalWriteEntry> & dst, const LogicalWriteEntry& src), (override));
};

} // namespace pos
