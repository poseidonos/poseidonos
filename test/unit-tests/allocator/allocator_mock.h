#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/allocator.h"

namespace pos
{
class MockAllocator : public Allocator
{
public:
    using Allocator::Allocator;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID), (override));
    MOCK_METHOD(void, SetGcThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(void, SetUrgentThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(int, GetMeta, (AllocatorCtxType type, std::string fname), (override));
    MOCK_METHOD(int, SetMeta, (AllocatorCtxType type, std::string fname), (override));
    MOCK_METHOD(int, GetBitmapLayout, (std::string fname), (override));
    MOCK_METHOD(int, GetInstantMetaInfo, (std::string fname), (override));
    MOCK_METHOD(void, FlushAllUserdataWBT, (), (override));
    MOCK_METHOD(IBlockAllocator*, GetIBlockAllocator, (), (override));
    MOCK_METHOD(IWBStripeAllocator*, GetIWBStripeAllocator, (), (override));
    MOCK_METHOD(IAllocatorCtx*, GetIAllocatorCtx, (), (override));
    MOCK_METHOD(IWBStripeCtx*, GetIWBStripeCtx, (), (override));
    MOCK_METHOD(ISegmentCtx*, GetISegmentCtx, (), (override));
};

} // namespace pos
