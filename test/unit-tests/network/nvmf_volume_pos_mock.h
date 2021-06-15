#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "src/network/nvmf_volume_pos.h"

namespace pos
{
class MockvolumeListInfo : public volumeListInfo
{
public:
    using volumeListInfo::volumeListInfo;
};

class MockNvmfVolumePos : public NvmfVolumePos
{
public:
    using NvmfVolumePos::NvmfVolumePos;
    MOCK_METHOD(void, VolumeCreated, (struct pos_volume_info * info), (override));
    MOCK_METHOD(void, VolumeDeleted, (struct pos_volume_info * info), (override));
    MOCK_METHOD(void, VolumeMounted, (struct pos_volume_info * info), (override));
    MOCK_METHOD(void, VolumeUnmounted, (struct pos_volume_info * info), (override));
    MOCK_METHOD(void, VolumeUpdated, (struct pos_volume_info * info), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> & volList, string arrayName), (override));
};

} // namespace pos
