#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/raid1.h"

namespace pos
{
class MockRaid1 : public Raid1
{
public:
    using Raid1::Raid1;
    MOCK_METHOD(int, Translate, (FtBlkAddr&, const LogicalBlkAddr&), (override));
    MOCK_METHOD(int, Convert, (list<FtWriteEntry>&, const LogicalWriteEntry&), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba), (override));
    MOCK_METHOD(LogicalBlkAddr, _Translate, (const FtBlkAddr&), (override));
    MOCK_METHOD(void, _BindRecoverFunc, (), (override));
};

} // namespace pos
