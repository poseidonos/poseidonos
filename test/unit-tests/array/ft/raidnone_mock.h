#pragma once

#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/raid_none.h"

namespace pos
{
class MockRaidNone : public RaidNone
{
public:
    using RaidNone::RaidNone;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry&), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>&, const LogicalWriteEntry&), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba), (override));
    MOCK_METHOD(RaidState, GetRaidState, (vector<ArrayDeviceState> devs), (override));
};

} // namespace pos
