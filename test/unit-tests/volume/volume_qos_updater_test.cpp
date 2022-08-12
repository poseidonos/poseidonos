#include "src/volume/volume_qos_updater.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"
#include "test/unit-tests/sys_event/volume_event_publisher_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(VolumeQosUpdater, VolumeQosUpdater_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeQosUpdater* volQoS = new VolumeQosUpdater(volumes, arrayName, arrayID);
    delete volQoS;
}

TEST(VolumeQosUpdater, Do_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;

    NiceMock<MockVolumeEventPublisher> mockVolumeEventPublisher;

    int actual;
    int expected = EID(VOL_REQ_PROCESSED_BUT_ERROR_OCCURED);

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);
    VolumeQosUpdater VolumeQosUpdater(volumes, arrayName, arrayID, &mockVolumeEventPublisher);

    ON_CALL(mockVolumeEventPublisher, NotifyVolumeUpdated(_, _, _)).WillByDefault(Return(false));
    actual = VolumeQosUpdater.Do(name, 1024*1024, 1024*1024, 0, 0);

    // Then
    ASSERT_EQ(actual, expected);
}

} // namespace pos
