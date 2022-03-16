#include "src/volume/volume_detacher.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"


#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeDetacher, VolumeDetacher_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeDetacher* volDetach = new VolumeDetacher(volumes, arrayName, arrayID);
    delete volDetach;
}

TEST(VolumeDetacher, DoAll_noVolume)
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

    VolumeDetacher volumeDetacher(volumes, arrayName, arrayID);

    volumeDetacher.DoAll();

    // Then
}

} // namespace pos
