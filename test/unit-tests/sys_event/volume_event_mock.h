#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/sys_event/volume_event.h"

namespace pos
{
class MockVolumeEvent : public VolumeEvent
{
public:
    using VolumeEvent::VolumeEvent;
    MOCK_METHOD(bool, VolumeCreated, (string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUpdated, (string volName, int volID, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeDeleted, (string volName, int volID, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeMounted, (string volName, string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (string volName, int volID), (override));
    MOCK_METHOD(bool, VolumeLoaded, (string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
};

} // namespace pos
