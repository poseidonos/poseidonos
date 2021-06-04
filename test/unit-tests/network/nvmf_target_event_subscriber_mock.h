#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/network/nvmf_target_event_subscriber.h"

namespace pos
{
class MockNvmfTargetEventSubscriber : public NvmfTargetEventSubscriber
{
public:
    using NvmfTargetEventSubscriber::NvmfTargetEventSubscriber;
    MOCK_METHOD(bool, VolumeCreated, (string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName), (override));
    MOCK_METHOD(bool, VolumeDeleted, (string volName, int volID, uint64_t volSizeByte, string arrayName), (override));
    MOCK_METHOD(bool, VolumeMounted, (string volName, string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (string volName, int volID, string arrayName), (override));
    MOCK_METHOD(bool, VolumeLoaded, (string volName, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, string arrayName), (override));
    MOCK_METHOD(bool, VolumeUpdated, (string volName, int volID, uint64_t maxiops, uint64_t maxbw, string arrayName), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, string arrayName), (override));
};

} // namespace pos
