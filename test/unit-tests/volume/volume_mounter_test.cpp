#include "src/volume/volume_mounter.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"
#include "test/unit-tests/sys_event/volume_event_publisher_mock.h"
#include "test/unit-tests/network/nvmf_target_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(VolumeMounter, VolumeMounter_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeMounter* volMount = new VolumeMounter(volumes, arrayName, arrayID);
    delete volMount;
}

TEST(VolumeMounter, _CheckIfExistVolume_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    int actual;
    int expected = EID(VOL_NOT_FOUND);

    // When
    VolumeList volumes;

    VolumeMounter volumeMounter(volumes, arrayName, arrayID);

    actual = volumeMounter.Do(name, subnqn);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _CheckIfExistSubsystem_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected = EID(MOUNT_VOL_SUBSYSTEM_NOT_FOUND);

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(false));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(nullptr));

    actual = volumeMounter.Do(name, subnqn);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _CheckAndSetSubsystemToArray_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return(""));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(false));

    expected = EID(MOUNT_VOL_SUBSYSTEM_ALREADY_OCCUPIED);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _CheckAndSetSubsystemToArray_1)
{
    // Given
    std::string arrayName = "Array";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return("Test"));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(true));

    expected = EID(MOUNT_VOL_SUBSYSTEM_MISMATCH);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _CheckAndSetSubsystemToArray_2)
{
    // Given
    std::string arrayName = "Array";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return("Test"));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(false));

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeMounted(_, _, _)).WillByDefault(Return(false));

    expected = EID(MOUNT_VOL_SUBSYSTEM_MISMATCH);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}


TEST(VolumeMounter, _MountVolume_)
{
     // Given
    std::string arrayName = "Array";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);
    vol->Mount();

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return("Array"));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(false));

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeMounted(_, _, _)).WillByDefault(Return(false));

    expected = EID(MOUNT_VOL_ALREADY_MOUNTED);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _MountVolume_1)
{
    // Given
    std::string arrayName = "Array";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return("Array"));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(false));

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeMounted(_, _, _)).WillByDefault(Return(false));

    expected = EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}

TEST(VolumeMounter, _RollBackVolumeMount_)
{
    // Given
    std::string arrayName = "Array";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    std::string subnqn = "Test";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    int actual;
    int expected;

    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);

    struct spdk_nvmf_subsystem* subsystem[1];
    subsystem[0] = reinterpret_cast<struct spdk_nvmf_subsystem*>(0x1);

    VolumeMounter volumeMounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // When 1

    ON_CALL(mockNvmfTarget, CheckSubsystemExistance()).WillByDefault(Return(true));
    ON_CALL(mockNvmfTarget, FindSubsystem(_)).WillByDefault(Return(subsystem[0]));

    ON_CALL(mockNvmfTarget, GetSubsystemArrayName(_)).WillByDefault(Return("Array"));
    ON_CALL(mockNvmfTarget, SetSubsystemArrayName(_, _)).WillByDefault(Return(false));

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeMounted(_, _, _)).WillByDefault(Return(true));

    ON_CALL(mockNvmfTarget, TryToAttachNamespace(_, _, _, _)).WillByDefault(Return(false));

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeUnmounted(_, _)).WillByDefault(Return(false));

    expected = EID(MOUNT_VOL_UNABLE_TO_ATTACH_TO_NVMF);
    actual = volumeMounter.Do(name, subnqn);

    // Then 1
    ASSERT_EQ(actual, expected);
}

} // namespace pos
