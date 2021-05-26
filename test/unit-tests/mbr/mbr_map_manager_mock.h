#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mbr/mbr_map_manager.h"

namespace pos
{
class MockMbrMapManager : public MbrMapManager
{
public:
    using MbrMapManager::MbrMapManager;
    MOCK_METHOD(int, InsertDevices, (ArrayMeta & meta, unsigned int arrayIndex), (override));
    MOCK_METHOD(int, InsertDevice, (string deviceUid, unsigned int arrayIndex), (override));
    MOCK_METHOD(int, DeleteDevices, (unsigned int arrayIndex), (override));
    MOCK_METHOD(int, ResetMap, (), (override));
    MOCK_METHOD(int, CheckAllDevices, (ArrayMeta & meta), (override));
    MOCK_METHOD(int, FindArrayIndex, (string devName), (override));
};

} // namespace pos
