#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/partition/nvm_partition.h"

namespace pos
{
class MockNvmPartition : public NvmPartition
{
public:
    using NvmPartition::NvmPartition;
    MOCK_METHOD(void, RegisterService, (IPartitionServices* svc), (override));
    MOCK_METHOD(int, Translate, (list<PhysicalEntry>& pel, const LogicalEntry& le), (override));
    MOCK_METHOD(int, GetParityList, (list<PhysicalWriteEntry>& parity, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(bool, IsByteAccessSupported, (), (override));
};

} // namespace pos
