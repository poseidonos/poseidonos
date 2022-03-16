#include "src/volume/volume_deleter.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeDeleter, VolumeDeleter_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeDeleter* volDelete = new VolumeDeleter(volumes, arrayName, arrayID);
    delete volDelete;
}
TEST(VolumeDeleter, Do_nullvol)
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
    int expected = (int)POS_EVENT_ID::VOL_NOT_FOUND;

    // When
    VolumeList volumes;

    VolumeDeleter volumeDeleter(volumes, arrayName, arrayID);

    actual = volumeDeleter.Do(name);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeDeleter, Do_mountedVol)
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
    int expected = (int)POS_EVENT_ID::DELETE_VOL_MOUNTED_VOL_CANNOT_BE_DELETED;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);
    volumes.Add(vol);
    vol->Mount();

    VolumeDeleter volumeDeleter(volumes, arrayName, arrayID);

    actual = volumeDeleter.Do(name);

    // Then
     ASSERT_EQ(actual, expected);
}


} // namespace pos
