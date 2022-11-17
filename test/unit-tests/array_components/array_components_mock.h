#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/array_components.h"

namespace pos
{
class MockArrayComponents : public ArrayComponents
{
public:
    using ArrayComponents::ArrayComponents;
    MOCK_METHOD(int, Create, (DeviceSet<string> nameSet, string metaFt, string dataFt), (override));
    MOCK_METHOD(int, Load, (), (override));
    MOCK_METHOD(int, Mount, (bool isWT), (override));
    MOCK_METHOD(int, Unmount, (), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(int, PrepareRebuild, (bool& resume), (override));
    MOCK_METHOD(void, RebuildDone, (), (override));
    MOCK_METHOD(Array*, GetArray, (), (override));
    MOCK_METHOD(ComponentsInfo*, GetInfo, (), (override));
    MOCK_METHOD(void, SetTargetAddress, (string targetAddress), (override));
    MOCK_METHOD(string, GetTargetAddress, (), (override));
};

} // namespace pos
