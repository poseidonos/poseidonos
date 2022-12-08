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
    MOCK_METHOD(VolumeIo::RbaList::iterator, AcquireOwnershipRbaList, (uint32_t volumeID, const VolumeIo::RbaList& sectorRbaList,
        VolumeIo::RbaList::iterator startIter, uint32_t& acquiredCnt), (override));
    MOCK_METHOD(void, ReleaseOwnershipRbaList, (uint32_t volumeID, const VolumeIo::RbaList& sectorRbaList), (override));
    MOCK_METHOD(int, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
