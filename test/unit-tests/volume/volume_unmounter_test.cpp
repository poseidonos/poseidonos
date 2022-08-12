#include "src/volume/volume_unmounter.h"

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
TEST(VolumeUnmounter, VolumeUnmounter_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeUnmounter* volUnmount = new VolumeUnmounter(volumes, arrayName, arrayID);
    delete volUnmount;
}

TEST(VolumeUnmounter, Do_nullvol)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    int actual;
    int expected = EID(VOL_NOT_FOUND);

    // When
    VolumeList volumes;

    VolumeUnmounter volumeUnmounter(volumes, arrayName, arrayID);

    actual = volumeUnmounter.Do(name);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeUnmounter, Do_volUnmounted)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;

    int actual;
    int expected = EID(UNMOUNT_VOL_ALREADY_UNMOUNTED);

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);
    vol->Unmount();

    VolumeUnmounter volumeUnmounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    // ON_CALL(mockVolumeEventPublisher, RemoveSubsystemArrayName(_)).WillByDefault(Return(void));

    actual = volumeUnmounter.Do(name);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeUnmounter, Do_notifyFail)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;

    int actual;
    int expected = EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;
    NiceMock<MockNvmfTarget> mockNvmfTarget;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);
    vol->Mount();

    VolumeUnmounter volumeUnmounter(volumes, arrayName, arrayID, &mockVolumeEventPublisher, &mockNvmfTarget);

    actual = volumeUnmounter.Do(name);

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeUnmounted(_, _)).WillByDefault(Return(false));

    // Then
    ASSERT_EQ(actual, expected);
}
} // namespace pos
