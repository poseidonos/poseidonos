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
    VolumeStatus actualstatus;
    VolumeStatus expectedstatus;
    uint64_t actualvalue;
    uint64_t expectedvalue;

    // When
    VolumeBase* vol = new VolumeBase(arrayName, arrayID, name, size, VolumeAttribute::UserData);
    vol->SetUuid(uuid);
    vol->SetMaxIOPS(maxBw);
    vol->SetMaxBW(maxIops);

    // Then
    actual = vol->GetName();
    expected = "volumetest";
    ASSERT_EQ(actual, expected);

    actual = vol->GetUuid();
    expected = uuid;
    ASSERT_EQ(actual, expected);

    actual = vol->GetArrayName();
    expected = arrayName;
    ASSERT_EQ(actual, expected);

    actualstatus = vol->GetStatus();
    expectedstatus = VolumeStatus::Unmounted;
    ASSERT_EQ(actualstatus, expectedstatus);

    actualvalue = vol->MaxIOPS();
    expectedvalue = maxIops;
    ASSERT_EQ(actualvalue, expectedvalue);

    actualvalue = vol->MaxBW();
    expectedvalue = maxBw;
    ASSERT_EQ(actualvalue, expectedvalue);

    vol->Rename(newName);
    actual = newName;
    expected = vol->GetName();
    ASSERT_EQ(actual, expected);

    delete vol;
}
} // namespace pos
