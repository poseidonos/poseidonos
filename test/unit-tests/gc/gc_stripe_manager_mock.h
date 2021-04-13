#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/gc/gc_stripe_manager.h"

namespace pos
{
class MockGcStripeManager : public GcStripeManager
{
public:
    using GcStripeManager::GcStripeManager;
    MOCK_METHOD(bool, VolumeCreated, (std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeDeleted, (std::string volName, int volID, uint64_t volSizeByte, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeMounted, (std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUnmounted, (std::string volName, int volID, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeLoaded, (std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(bool, VolumeUpdated, (std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName), (override));
    MOCK_METHOD(void, VolumeDetached, (vector<int> volList, std::string arrayName), (override));
    MOCK_METHOD(bool, AllocateWriteBufferBlks, (uint32_t volumeId, uint32_t numBlks, uint32_t& offset, uint32_t& allocatedBlks), (override));
    MOCK_METHOD(void, MoveActiveWriteBuffer, (uint32_t volumeId, GcWriteBuffer* buffer), (override));
    MOCK_METHOD(std::mutex&, GetWriteBufferLock, (uint32_t volumeId), (override));
    MOCK_METHOD(void, SetFinished, (GcWriteBuffer* buffer), (override));
    MOCK_METHOD(GcWriteBuffer*, GetWriteBuffer, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, DecreaseRemainingAndCheckIsFull, (uint32_t volumeId, uint32_t cnt), (override));
    MOCK_METHOD(void, SetBlkInfo, (uint32_t volumeId, uint32_t offset, BlkInfo blkInfo), (override));
    MOCK_METHOD(std::vector<BlkInfo>*, GetBlkInfoList, (uint32_t volumeId), (override));
    MOCK_METHOD(void, SetFlushed, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, IsAllFinished, (), (override));
};

} // namespace pos
