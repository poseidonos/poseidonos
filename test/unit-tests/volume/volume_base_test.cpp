#include "src/volume/volume_base.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeBase, VolumeBase_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "abcd";
    std::string uuid = "TestTest";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;

    std::string actual;
    std::string expected;
    VolumeMountStatus actualstatus;
    VolumeMountStatus expectedstatus;
    uint64_t actualvalue;
    uint64_t expectedvalue;

    // When
    VolumeBase* vol = new VolumeBase(arrayID, arrayName, DataAttribute::UserData, name, size, 0xFFFF, ReplicationRole::Primary);
    vol->SetUuid(uuid);
    vol->SetMaxIOPS(maxBw);
    vol->SetMaxBW(maxIops);

    // Then
    actual = vol->GetVolumeName();
    expected = "volumetest";
    ASSERT_EQ(actual, expected);

    actual = vol->GetUuid();
    expected = uuid;
    ASSERT_EQ(actual, expected);

    actual = vol->GetArrayName();
    expected = arrayName;
    ASSERT_EQ(actual, expected);

    actualstatus = vol->GetVolumeMountStatus();
    expectedstatus = VolumeMountStatus::Unmounted;
    ASSERT_EQ(actualstatus, expectedstatus);

    actualvalue = vol->GetMaxIOPS();
    expectedvalue = maxIops;
    ASSERT_EQ(actualvalue, expectedvalue);

    actualvalue = vol->GetMaxBW();
    expectedvalue = maxBw;
    ASSERT_EQ(actualvalue, expectedvalue);

    vol->SetVolumeName(newName);
    actual = newName;
    expected = vol->GetVolumeName();
    ASSERT_EQ(actual, expected);

    delete vol;
}
} // namespace pos
