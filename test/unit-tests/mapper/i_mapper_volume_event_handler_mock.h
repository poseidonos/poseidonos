#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_mapper_volume_event_handler.h"

namespace pos
{
class MockIMapperVolumeEventHandler : public IMapperVolumeEventHandler
{
public:
    using IMapperVolumeEventHandler::IMapperVolumeEventHandler;
    MOCK_METHOD(int, VolumeCreated, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(int, VolumeMounted, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(int, VolumeLoaded, (int volId, uint64_t volSizeByte), (override));
    MOCK_METHOD(int, VolumeUnmounted, (int volId, bool flushMapRequired), (override));
    MOCK_METHOD(int, PrepareVolumeDelete, (int volId), (override));
    MOCK_METHOD(int, DeleteVolumeMap, (int volId), (override));
    MOCK_METHOD(int, VolumeDetached, (vector<int> volList), (override));
};

} // namespace pos
