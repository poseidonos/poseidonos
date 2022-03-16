#include "src/volume/volume_renamer.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeRenamer, VolumeRenamer_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeRenamer* volRename = new VolumeRenamer(volumes, arrayName, arrayID);
    delete volRename;
}

TEST(VolumeRenamer, Do_nullvol)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;

    int actual;
    int expected = (int)POS_EVENT_ID::VOL_NOT_FOUND;

    // When
    VolumeRenamer volumeRenamer(volumes, arrayName, arrayID);

    actual = volumeRenamer.Do(name, newName);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeRenamer, Do_sameName)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";
    uint64_t size = 1024;

    int actual;
    int expected = (int)POS_EVENT_ID::CREATE_VOL_NAME_DUPLICATED;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);

    VolumeRenamer volumeRenamer(volumes, arrayName, arrayID);

    actual = volumeRenamer.Do(name, newName);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeRenamer, Do_wrongName)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024;

    int actual;
    int expected = (int)POS_EVENT_ID::CREATE_VOL_NAME_TOO_SHORT;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);

    VolumeRenamer volumeRenamer(volumes, arrayName, arrayID);

    actual = volumeRenamer.Do(name, newName);

    // Then
    ASSERT_EQ(actual, expected);
}

/*TEST(VolumeRenamer, Do_saveFail)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumerenmae";
    uint64_t size = 1024;

    int actual;
    int expected = (int)POS_EVENT_ID::VOL_NOT_FOUND;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);

    VolumeRenamer volumeRenamer(volumes, arrayName, arrayID);

    actual = volumeRenamer.Do(name, newName);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeRenamer, Do_Pass)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumerenmae";
    uint64_t size = 1024;

    int actual;
    int expected = (int)POS_EVENT_ID::VOL_NOT_FOUND;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);

    VolumeRenamer volumeRenamer(volumes, arrayName, arrayID);

    actual = volumeRenamer.Do(name, newName);

    // Then
    ASSERT_EQ(actual, expected);
}*/

} // namespace pos
