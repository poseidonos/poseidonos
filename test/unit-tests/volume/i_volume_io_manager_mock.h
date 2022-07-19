#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/i_volume_io_manager.h"

namespace pos
{
class MockIVolumeIoManager : public IVolumeIoManager
{
public:
    using IVolumeIoManager::IVolumeIoManager;
    MOCK_METHOD(int, IncreasePendingIOCountIfNotZero, (int volId, VolumeIoType volumeIoType, uint32_t ioCountToSubmit), (override));
    MOCK_METHOD(int, DecreasePendingIOCount, (int volId, VolumeIoType volumeIoType, uint32_t ioCountCompleted), (override));

};

} // namespace pos
