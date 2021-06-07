#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/network/nvmf_target_event_subscriber.h"
#include "test/unit-tests/network/nvmf_volume_mock.h"
#include "src/sys_event/volume_event.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
 
namespace pos {

TEST(NvmfTargetEventSubscriber, NvmfTargetEventSubscriber_Constructor_Stack)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string arrayName("array");

    // When: Try to Create New NvmfTargetEventSubscriber object with 2 argument
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);

    // Then: Do Nothing
}

TEST(NvmfTargetEventSubscriber, NvmfTargetEventSubscriber_Constructor_Heap)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string arrayName("array");

    // When: Try to Create New NvmfTargetEventSubscriber object with 2 argument
    NvmfTargetEventSubscriber* nvmfTargetEventSubscriber = new NvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);

    // Then: Release Memory
    delete nvmfTargetEventSubscriber;
}
TEST(NvmfTargetEventSubscriber, VolumeCreated_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: Call VolumeCreated
    ON_CALL(mockNvmfVolume, VolumeCreated(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeCreated(volName, volId, volSizeByte, maxIops, maxBw, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeDeleted_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: Call VolumeDeleted
    ON_CALL(mockNvmfVolume, VolumeDeleted(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeDeleted(volName, volId, volSizeByte, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeMounted_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    std::string subNqn("subsystem_name");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: CAll VolumeMounted
    ON_CALL(mockNvmfVolume, VolumeMounted(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeMounted(volName, subNqn, volId, volSizeByte, maxIops, maxBw, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeUnmounted_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    int volId = 0;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: CAll VolumeUnmounted
    ON_CALL(mockNvmfVolume, VolumeUnmounted(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeUnmounted(volName, volId, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeLoaded_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    int volId = 0;
    uint64_t totalSize = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: CAll VolumeLoaded
    ON_CALL(mockNvmfVolume, VolumeCreated(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeLoaded(volName, volId, totalSize, maxIops, maxBw, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeUpdated_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    std::string volName("volume");
    int volId = 0;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);
    bool actual, expected{true};

    // When: Call VolumeUpdated
    ON_CALL(mockNvmfVolume, VolumeUpdated(_)).WillByDefault(Return());
    actual = nvmfTargetEventSubscriber.VolumeUpdated(volName, volId, maxIops, maxBw, arrayName, 0);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(NvmfTargetEventSubscriber, VolumeDetached_Success)
{
    // Given
    NiceMock<MockNvmfVolume> mockNvmfVolume;
    vector<int> volList{0,1,2};
    std::string arrayName("array");
    NvmfTargetEventSubscriber nvmfTargetEventSubscriber(&mockNvmfVolume, arrayName, 0);

    // When: Call VolumeDetached
    ON_CALL(mockNvmfVolume, VolumeDetached(volList, arrayName)).WillByDefault(Return());
    nvmfTargetEventSubscriber.VolumeDetached(volList, arrayName, 0);

    // Then: Do Nothing
}
}  // namespace pos
