#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/vsamap/vsamap_manager.h"

namespace pos
{
class MockVSAMapManager : public VSAMapManager
{
public:
    using VSAMapManager::VSAMapManager;
    MOCK_METHOD(void, MapAsyncFlushDone, (int mapId), (override));
    MOCK_METHOD(int, EnableInternalAccess, (int volID, int caller), (override));
    MOCK_METHOD(std::atomic<int>&, GetLoadDoneFlag, (int volumeId), (override));
    MOCK_METHOD(bool, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
