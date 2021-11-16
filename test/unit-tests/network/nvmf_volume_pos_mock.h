#include <gmock/gmock.h>

#include <list>
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
    MOCK_METHOD(bool, VolumeCreated, (struct pos_volume_info * info, uint64_t time), (override));
    MOCK_METHOD(bool, VolumeLoaded, (struct pos_volume_info * info), (override));
    MOCK_METHOD(bool, VolumeDeleted, (struct pos_volume_info * info, uint64_t time), (override));
    MOCK_METHOD(void, VolumeMounted, (struct pos_volume_info * info), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (struct pos_volume_info * info, uint64_t time), (override));
    MOCK_METHOD(void, VolumeUpdated, (struct pos_volume_info * info), (override));
    MOCK_METHOD(bool, VolumeDetached, (vector<int> & volList, string arrayName, uint64_t time), (override));
};

} // namespace pos
