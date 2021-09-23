#include "src/volume/volume_loader.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeLoader, VolumeLoader_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeLoader* volumeLoader = new VolumeLoader(volumes, arrayName, arrayID);
    delete volumeLoader;
}

TEST(VolumeLoader, Do_)
{
}

} // namespace pos
