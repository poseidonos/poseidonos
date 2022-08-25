#include "src/network/nvmf.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lib/spdk/include/spdk/pos_volume.h"
#include "src/include/pos_event_id.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/sys_event/volume_event.h"
#include "test/unit-tests/network/nvmf_volume_pos_mock.h"
#include "test/unit-tests/sys_event/volume_event_publisher_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(Nvmf, Nvmf_Constructor_TwoArgumnet_Stack)
{
    // Given
    std::string arrayName("array");

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0);

    // Then: Do Nothing
}

TEST(Nvmf, Nvmf_Constructor_TwoArgument_Heap)
{
    // Given
    std::string arrayName("array");

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf* nvmf = new Nvmf(arrayName, 0);

    // Then: Release Memory
    delete nvmf;
}

TEST(Nvmf, Nvmf_Constructor_ThreeArgumnet_Stack)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, nullptr);

    // Then: Do Nothing
}

TEST(Nvmf, Nvmf_Constructor_ThreeArgument_Heap)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf* nvmf = new Nvmf(arrayName, 0, &mockVolumeEventPublisher, nullptr);

    // Then: Release Memory
    delete nvmf;
}

TEST(Nvmf, Init_ioHandler)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");
    int actual, expected{0};

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, nullptr);

    // Then: call Init()
    actual = nvmf.Init();

    ASSERT_EQ(actual, expected);
}

TEST(Nvmf, Dispose_nvmfVolumePos)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);

    // Then: call Dispose()
    nvmf.Dispose();
}

TEST(Nvmf, Shutdown_Default)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);

    // Then: call Shutdown()
    nvmf.Shutdown();
}

TEST(Nvmf, SetuNVMfIOHandler_Default)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, nullptr);

    // Then: set uNVMf io handler
    nvmf.SetuNVMfIOHandler(handler);
}

TEST(Nvmf, SetuNVMfIOHandler_Override)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    std::string arrayName("array");

    unvmf_io_handler handler1 = {.submit = UNVMfSubmitHandler, .complete = UNVMfCompleteHandler};

    // When: Try to Create New Nvmf object with 2 argument
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, nullptr);

    // Then: set uNVMf io handler
    nvmf.SetuNVMfIOHandler(handler1);

    unvmf_io_handler handler2 = {.submit = nullptr, .complete = nullptr};
    nvmf.SetuNVMfIOHandler(handler2);
}

TEST(Nvmf, VolumeCreated_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, volSizeByte, volName, "", "", "");
    VolumeEventPerf volumeMountPerf;
    nvmf.SetVolumePerf(&volumeMountPerf, maxIops, maxBw);
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeCreated
    ON_CALL(*mockNvmfVolumePos, VolumeCreated(_, _)).WillByDefault(Return(true));
    actual = nvmf.VolumeCreated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeCreated_Fail)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_FAIL)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, volSizeByte, volName, "", "", "");
    VolumeEventPerf volumeMountPerf;
    nvmf.SetVolumePerf(&volumeMountPerf, maxIops, maxBw);
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeCreated
    ON_CALL(*mockNvmfVolumePos, VolumeCreated(_, _)).WillByDefault(Return(false));
    actual = nvmf.VolumeCreated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo);

    // Then: Expect result as false
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeDeleted_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};
    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, volSizeByte, volName, "", "", "");
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeDeleted
    ON_CALL(*mockNvmfVolumePos, VolumeDeleted(_, _)).WillByDefault(Return(true));
    actual = nvmf.VolumeDeleted(&volumeEventBase, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeDeleted_Fail)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_FAIL)};
    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, volSizeByte, volName, "", "", "");
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeDeleted
    ON_CALL(*mockNvmfVolumePos, VolumeDeleted(_, _)).WillByDefault(Return(false));
    actual = nvmf.VolumeDeleted(&volumeEventBase, &volumeArrayInfo);

    // Then: Expect result as false
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeMounted_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    std::string subNqn("subsystem_name");
    int volId = 0;
    uint64_t volSizeByte = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, volSizeByte, volName, "", "", "");
    VolumeEventPerf volumeMountPerf;
    nvmf.SetVolumePerf(&volumeMountPerf, maxIops, maxBw);
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: CAll VolumeMounted
    ON_CALL(*mockNvmfVolumePos, VolumeMounted(_)).WillByDefault(Return());
    actual = nvmf.VolumeMounted(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeUnmounted_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, 0, volName, "", "", "");
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: CAll VolumeUnmounted
    ON_CALL(*mockNvmfVolumePos, VolumeUnmounted(_, _)).WillByDefault(Return(true));
    actual = nvmf.VolumeUnmounted(&volumeEventBase, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeUnmounted_Fail)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_FAIL)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, 0, volName, "", "", "");
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: CAll VolumeUnmounted
    ON_CALL(*mockNvmfVolumePos, VolumeUnmounted(_, _)).WillByDefault(Return(false));
    actual = nvmf.VolumeUnmounted(&volumeEventBase, &volumeArrayInfo);

    // Then: Expect result as false
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeLoaded_SuccessWithVolumeCreate)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t totalSize = 1073741824;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, 0, volName, "", "", "");
    VolumeEventPerf volumeMountPerf;
    nvmf.SetVolumePerf(&volumeMountPerf, maxIops, maxBw);
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: CAll VolumeLoaded
    ON_CALL(*mockNvmfVolumePos, VolumeLoaded(_)).WillByDefault(Return(false));
    ON_CALL(*mockNvmfVolumePos, VolumeCreated).WillByDefault(Return(true));
    actual = nvmf.VolumeLoaded(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeUpdated_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    std::string volName("volume");
    int volId = 0;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeEventBase volumeEventBase;
    nvmf.SetVolumeBase(&volumeEventBase, volId, 0, volName, "", "", "");
    VolumeEventPerf volumeMountPerf;
    nvmf.SetVolumePerf(&volumeMountPerf, maxIops, maxBw);
    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeUpdated
    ON_CALL(*mockNvmfVolumePos, VolumeUpdated(_)).WillByDefault(Return());
    actual = nvmf.VolumeUpdated(&volumeEventBase, &volumeMountPerf, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}

TEST(Nvmf, VolumeDetached_Success)
{
    // Given
    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    unvmf_io_handler handler = {.submit = nullptr, .complete = nullptr};
    NiceMock<MockNvmfVolumePos>* mockNvmfVolumePos = new NiceMock<MockNvmfVolumePos>(handler);
    vector<int> volList{0, 1, 2};
    std::string arrayName("array");
    Nvmf nvmf(arrayName, 0, &mockVolumeEventPublisher, mockNvmfVolumePos);
    int actual, expected{EID(VOL_EVENT_OK)};

    VolumeArrayInfo volumeArrayInfo;
    nvmf.SetVolumeArrayInfo(&volumeArrayInfo, 0, arrayName);

    // When: Call VolumeDetached
    ON_CALL(*mockNvmfVolumePos, VolumeDetached(volList, arrayName, _)).WillByDefault(Return(true));
    actual = nvmf.VolumeDetached(volList, &volumeArrayInfo);

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
    delete mockNvmfVolumePos;
}
} // namespace pos
