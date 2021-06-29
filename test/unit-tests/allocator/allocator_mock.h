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
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID, std::string arrayName), (override));
    MOCK_METHOD(void, SetGcThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(void, SetUrgentThreshold, (uint32_t inputThreshold), (override));
    MOCK_METHOD(int, GetMeta, (WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file), (override));
    MOCK_METHOD(int, SetMeta, (WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file), (override));
    MOCK_METHOD(int, GetBitmapLayout, (std::string fname), (override));
    MOCK_METHOD(int, GetInstantMetaInfo, (std::string fname), (override));
    MOCK_METHOD(void, FlushAllUserdataWBT, (), (override));
    MOCK_METHOD(IBlockAllocator*, GetIBlockAllocator, (), (override));
    MOCK_METHOD(IWBStripeAllocator*, GetIWBStripeAllocator, (), (override));
    MOCK_METHOD(IContextManager*, GetIContextManager, (), (override));
    MOCK_METHOD(IContextReplayer*, GetIContextReplayer, (), (override));
};

} // namespace pos
