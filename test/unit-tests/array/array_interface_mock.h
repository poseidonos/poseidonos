#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/array_interface.h"

namespace pos
{
class MockArrayInterface : public ArrayInterface
{
public:
    using ArrayInterface::ArrayInterface;
    MOCK_METHOD(void, AddTranslator, (PartitionType type, ITranslator* trans), (override));
    MOCK_METHOD(void, AddRecover, (PartitionType type, IRecover* recov), (override));
    MOCK_METHOD(void, AddRebuildTarget, (RebuildTarget * tgt), (override));
    MOCK_METHOD(void, ClearInterface, (), (override));
    MOCK_METHOD((map<PartitionType, ITranslator*>), GetTranslator, (), (override));
    MOCK_METHOD((map<PartitionType, IRecover*>), GetRecover, (), (override));
    MOCK_METHOD(list<RebuildTarget*>, GetRebuildTargets, (), (override));
};

} // namespace pos
