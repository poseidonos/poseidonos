#include "src/volume/volume_creator.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeCreator, VolumeCreator_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeCreator* volCreate = new VolumeCreator(volumes, arrayName, arrayID);
    delete volCreate;
}
TEST(VolumeCreator, Do_wrongName)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;
    uint64_t minIops = 0;
    uint64_t minBw = 0;

    int actual;
    int expected = EID(SUCCESS);

    // When
    VolumeList volumes;

    VolumeCreator volumeCreator(volumes, arrayName, arrayID);

    actual = volumeCreator.Do(newName, size, maxIops, maxBw, minIops, minBw);

    // Then
    ASSERT_NE(actual, expected);
}

TEST(VolumeCreator, Do_SameName)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024;
    uint64_t maxIops = 100;
    uint64_t maxBw = 100;
    uint64_t minIops = 0;
    uint64_t minBw = 0;

    int actual;
    int expected = (int)POS_EVENT_ID::CREATE_VOL_SAME_VOL_NAME_EXISTS;

    // When
    VolumeList volumes;

    VolumeBase* vol = new Volume(arrayName, arrayID, name, size);

    volumes.Add(vol);

    VolumeCreator volumeCreator(volumes, arrayName, arrayID);

    actual = volumeCreator.Do(name, size, maxIops, maxBw, minIops, minBw);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeCreator, Do_SetSizeFail)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024;
    uint64_t maxIops = 1;
    uint64_t maxBw = 1;
    uint64_t minIops = 0;
    uint64_t minBw = 0;

    int actual;
    int expected = (int)POS_EVENT_ID::CREATE_VOL_SIZE_NOT_ALIGNED;

    // When
    VolumeList volumes;

    VolumeCreator volumeCreator(volumes, arrayName, arrayID);

    actual = volumeCreator.Do(name, size, maxIops, maxBw, minIops, minBw);

    // Then
    ASSERT_EQ(actual, expected);
}

TEST(VolumeCreator, Do_NotEnoughArraySize)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "";
    uint64_t size = 1024*1024;
    uint64_t maxIops = 1;
    uint64_t maxBw = 1;
    uint64_t minIops = 0;
    uint64_t minBw = 0;

    int actual;
    int expected = (int)POS_EVENT_ID::CREATE_VOL_SIZE_EXCEEDED;

    // When
    VolumeList volumes;

    VolumeCreator volumeCreator(volumes, arrayName, arrayID);

    actual = volumeCreator.Do(name, size, maxIops, maxBw, minIops, minBw);

    // Then
    ASSERT_EQ(actual, expected);
}

} // namespace pos
