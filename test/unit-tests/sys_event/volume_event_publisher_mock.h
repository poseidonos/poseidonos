#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/sys_event/volume_event_publisher.h"

namespace pos
{
class MockVolumeEventPublisher : public VolumeEventPublisher
{
public:
    using VolumeEventPublisher::VolumeEventPublisher;
    MOCK_METHOD(bool, NotifyVolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, NotifyVolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, NotifyVolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, NotifyVolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, NotifyVolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(bool, NotifyVolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(void, NotifyVolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
