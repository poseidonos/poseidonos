#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/ft/method.h"

namespace pos
{
class MockFtSizeInfo : public FtSizeInfo
{
public:
    using FtSizeInfo::FtSizeInfo;
};

class MockMethod : public Method
{
public:
    using Method::Method;
    MOCK_METHOD(int, Translate, (FtBlkAddr&, const LogicalBlkAddr&), (override));
    MOCK_METHOD(int, Convert, (list<FtWriteEntry>&, const LogicalWriteEntry&), (override));
    MOCK_METHOD(list<FtBlkAddr>, GetRebuildGroup, (FtBlkAddr fba), (override));
    MOCK_METHOD(void, _BindRecoverFunc, (), (override));
};

} // namespace pos
