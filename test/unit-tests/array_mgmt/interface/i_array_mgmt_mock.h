#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_mgmt/interface/i_array_mgmt.h"

namespace pos
{
class MockIArrayMgmt : public IArrayMgmt
{
public:
    using IArrayMgmt::IArrayMgmt;
    MOCK_METHOD(int, Create, (string name, DeviceSet<string> devs, string metaFt, string dataFt), (override));
    MOCK_METHOD(int, Delete, (string name), (override));
    MOCK_METHOD(int, Mount, (string name, bool isWTEnabled), (override));
    MOCK_METHOD(int, Unmount, (string name), (override));
    MOCK_METHOD(int, AddDevice, (string name, string dev), (override));
    MOCK_METHOD(int, RemoveDevice, (string name, string dev), (override));
    MOCK_METHOD(ComponentsInfo*, GetInfo, (string name), (override));
    MOCK_METHOD(ComponentsInfo*, GetInfo, (uint32_t arrayIdx), (override));
};

} // namespace pos
