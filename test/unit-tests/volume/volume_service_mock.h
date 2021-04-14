#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_service.h"

namespace pos
{
class MockVolumeService : public VolumeService
{
public:
    using VolumeService::VolumeService;
    MOCK_METHOD(IVolumeManager*, GetVolumeManager, (std::string arrayName), (override));
};

} // namespace pos
