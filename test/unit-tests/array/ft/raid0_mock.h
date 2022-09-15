#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/ft/raid0.h"

namespace pos
{
class MockRaid0 : public Raid0
{
public:
    using Raid0::Raid0;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry& le), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>& ftl, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(RaidState, GetRaidState, (const vector<ArrayDeviceState>& devs), (override));
    MOCK_METHOD(bool, CheckNumofDevsToConfigure, (uint32_t numofDevs), (override));
    MOCK_METHOD(bool, IsRecoverable, (), (override));
};

} // namespace pos
