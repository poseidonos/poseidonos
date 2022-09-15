#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/ft/raid_none.h"

namespace pos
{
class MockRaidNone : public RaidNone
{
public:
    using RaidNone::RaidNone;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry& le), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>& ftl, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(RaidState, GetRaidState, (const vector<ArrayDeviceState>& devs), (override));
    MOCK_METHOD(bool, CheckNumofDevsToConfigure, (uint32_t numofDevs), (override));
    MOCK_METHOD(bool, IsRecoverable, (), (override));
};

} // namespace pos
