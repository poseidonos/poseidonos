#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/raid5.h"

namespace pos
{
class MockRaid5 : public Raid5
{
public:
    using Raid5::Raid5;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry&), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>&, const LogicalWriteEntry&), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba), (override));
    MOCK_METHOD(RaidState, GetRaidState, (vector<ArrayDeviceState> devs), (override));
};

} // namespace pos
