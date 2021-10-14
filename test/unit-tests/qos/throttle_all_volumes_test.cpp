#include "src/qos/throttle_all_volumes.h"

#include <gtest/gtest.h>

#include "src/qos/throttle_volume.h"

namespace pos
{
TEST(AllVolumeThrottle, AllVolumeThrottle_Contructor_One_Stack)
{
    AllVolumeThrottle allVolumeThrottle();
}
TEST(AllVolumeThrottle, AllVolumeThrottle_Contructor_One_Heap)
{
    AllVolumeThrottle* allVolumeThrottle = new AllVolumeThrottle();
    delete allVolumeThrottle;
}

TEST(AllVolumeThrottle, Reset_)
{
}

TEST(AllVolumeThrottle, CheckVolumeThrottle_Getter_and_Setter)
{
    AllVolumeThrottle allVolumeThrottle;
    VolumeThrottle volThrottle;
    volThrottle.Reset();
    uint32_t volId = 0;
    uint32_t arrayId = 0;
    allVolumeThrottle.InsertVolumeThrottle(arrayId, volId, volThrottle);
    std::map<std::pair<uint32_t, uint32_t>, VolumeThrottle>& volumeThrottleMap = allVolumeThrottle.GetVolumeThrottleMap();
    // ASSERT_NE(volumeThrottleMap, NULL);
    uint32_t expectedCorrectionFlag = 0;

    uint32_t actualCorrectionFlag = volumeThrottleMap[make_pair(arrayId, volId)].CorrectionType();
    ASSERT_EQ(expectedCorrectionFlag, actualCorrectionFlag);
}

} // namespace pos
