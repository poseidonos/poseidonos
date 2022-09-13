#include "src/volume/volume_interface.h"

#include "src/include/pos_event_id.h"
#include "src/volume/volume.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_name_policy.h"
#include "src/volume/volume_list.h"


#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeInterface, VolumeInterface_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string name = "volumetest";
    std::string newName = "volumetest";

    VolumeList volumes;
    // When
    VolumeInterface* volInter = new VolumeInterface(volumes, arrayName, arrayID);
    delete volInter;
}

TEST(VolumeInterface, _CheckVolumeSize_)
{
}

TEST(VolumeInterface, _SetVolumeQos_)
{
}

TEST(VolumeInterface, _SaveVolumes_)
{
}

} // namespace pos
