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
    MOCK_METHOD(bool, AcquireOwnershipRbaList, (uint32_t volumeID, const VolumeIo::RbaList& sectorRbaList), (override));
    MOCK_METHOD(void, ReleaseOwnershipRbaList, (uint32_t volumeID, const VolumeIo::RbaList& sectorRbaList), (override));
    MOCK_METHOD(bool, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
