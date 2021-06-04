#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/network/nvmf_volume.h"

namespace pos
{
class MockNvmfVolume : public NvmfVolume
{
public:
    using NvmfVolume::NvmfVolume;
    MOCK_METHOD(void, VolumeCreated, (struct pos_volume_info* info), (override));
    MOCK_METHOD(void, VolumeDeleted, (struct pos_volume_info* info), (override));
    MOCK_METHOD(void, VolumeMounted, (struct pos_volume_info* info), (override));
    MOCK_METHOD(void, VolumeUnmounted, (struct pos_volume_info* info), (override));
    MOCK_METHOD(void, VolumeUpdated, (struct pos_volume_info* info), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int>& volList, std::string arrayName), (override));
};

} // namespace pos
