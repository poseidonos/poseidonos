#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/metadata/meta_volume_event_handler.h"

namespace pos
{
class MockMetaVolumeEventHandler : public MetaVolumeEventHandler
{
public:
    using MetaVolumeEventHandler::MetaVolumeEventHandler;
    MOCK_METHOD(int, VolumeCreated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeMounted, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeLoaded, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUpdated, (VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeUnmounted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDeleted, (VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo), (override));
    MOCK_METHOD(int, VolumeDetached, (vector<int> volList, VolumeArrayInfo* volArrayInfo), (override));
};

} // namespace pos
