#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/mapper/i_mapper_volume_event_handler.h"

namespace pos
{
class MockIMapperVolumeEventHandler : public IMapperVolumeEventHandler
{
public:
    using IMapperVolumeEventHandler::IMapperVolumeEventHandler;
    MOCK_METHOD(bool, VolumeCreated, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeMounted, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeLoaded, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (int volId, bool flushMapRequired), (override));
    MOCK_METHOD(int, PrepareVolumeDelete, (int volId), (override));
    MOCK_METHOD(int, DeleteVolumeMap, (int volId), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
};

} // namespace pos
