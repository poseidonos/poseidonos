#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/general_io/rba_state_manager.h"

namespace pos
{
class MockRBAStateManager : public RBAStateManager
{
public:
    using RBAStateManager::RBAStateManager;
    MOCK_METHOD(bool, BulkAcquireOwnership, (uint32_t volumeID, BlkAddr startRba, uint32_t count), (override));
    MOCK_METHOD(void, BulkReleaseOwnership, (uint32_t volumeID, BlkAddr startRba, uint32_t count), (override));

    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, std::string arrayName), (override));
};

} // namespace pos
