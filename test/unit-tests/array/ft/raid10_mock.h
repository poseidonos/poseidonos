#pragma once

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/raid10.h"

namespace pos
{
class MockRaid10 : public Raid10
{
public:
    using Raid10::Raid10;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry&), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>&, const LogicalWriteEntry&), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba), (override));
    MOCK_METHOD(RaidState, GetRaidState, (vector<ArrayDeviceState> devs), (override));
};

} // namespace pos
