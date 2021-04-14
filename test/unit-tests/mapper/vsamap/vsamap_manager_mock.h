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
    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
};

} // namespace pos
