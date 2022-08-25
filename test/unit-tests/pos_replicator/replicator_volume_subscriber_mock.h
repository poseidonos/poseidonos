#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/pos_replicator/replicator_volume_subscriber.h"

namespace pos
{
class MockReplicatorVolumeSubscriber : public ReplicatorVolumeSubscriber
{
public:
    using ReplicatorVolumeSubscriber::ReplicatorVolumeSubscriber;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, Flush, (), (override));
    MOCK_METHOD(int, VolumeCreated, (VolumeEventBase * volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeMounted, (VolumeEventBase * volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeLoaded, (VolumeEventBase * volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUpdated, (VolumeEventBase * volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUnmounted, (VolumeEventBase * volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDeleted, (VolumeEventBase * volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
