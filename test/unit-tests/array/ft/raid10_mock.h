#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/ft/raid10.h"

namespace pos
{
class MockRaid10 : public Raid10
{
public:
    using Raid10::Raid10;
    MOCK_METHOD(list<FtEntry>, Translate, (const LogicalEntry& le), (override));
    MOCK_METHOD(int, MakeParity, (list<FtWriteEntry>& ftl, const LogicalWriteEntry& src), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba, const vector<uint32_t>& abnormals), (override));
    MOCK_METHOD(RecoverFunc, GetRecoverFunc, (vector<uint32_t> targets, vector<uint32_t> abnormals), (override));
    MOCK_METHOD(RaidState, GetRaidState, (const vector<ArrayDeviceState>& devs), (override));
    MOCK_METHOD(vector<uint32_t>, GetParityOffset, (StripeId lsid), (override));
    MOCK_METHOD(bool, CheckNumofDevsToConfigure, (uint32_t numofDevs), (override));
    MOCK_METHOD((vector<pair<vector<uint32_t>, vector<uint32_t>>>), GetRebuildGroupPairs, (vector<uint32_t>& targetIndexs), (override));
};

} // namespace pos
