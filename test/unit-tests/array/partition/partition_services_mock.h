#pragma once

#include <gmock/gmock.h>

#include <list>
#include <map>

#include "src/array/partition/partition_services.h"

namespace pos
{
class MockPartitionServices : public PartitionServices
{
public:
    using PartitionServices::PartitionServices;
    MOCK_METHOD(void, AddTranslator, (PartitionType type, ITranslator* trans), (override));
    MOCK_METHOD(void, AddRecover, (PartitionType type, IRecover* recov), (override));
    MOCK_METHOD(void, AddRebuildTarget, (RebuildTarget * tgt), (override));
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD((map<PartitionType, ITranslator*>), GetTranslator, (), (override));
    MOCK_METHOD((map<PartitionType, IRecover*>), GetRecover, (), (override));
    MOCK_METHOD(list<RebuildTarget*>, GetRebuildTargets, (), (override));
};

} // namespace pos
