#include "src/qos/throttle_volume.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"

namespace pos
{
TEST(VolumeThrottle, VolumeThrottle_Contructore_One_Stack)
{
    VolumeThrottle volumeThrottle();
}
TEST(VolumeThrottle, VolumeThrottle_Contructore_One_Heap)
{
    VolumeThrottle* volumeThrottle = new VolumeThrottle();
    delete volumeThrottle;
}

TEST(VolumeThrottle, Reset_CorrectionFlag_Value_Test)
{
    VolumeThrottle volumeThrottle;
    // reset sets the correction flag to 0, so expected value is 0
    volumeThrottle.Reset();
    uint32_t expectedCorrectionFlagVal = 0;
    uint32_t correctionReturned;
    correctionReturned = volumeThrottle.CorrectionType();
    ASSERT_EQ(expectedCorrectionFlagVal, correctionReturned);
}

TEST(VolumeThrottle, SetResetFlag_Check_Reset_Setter)
{
    // check for setter function to increase code coverage
    bool resetFlagVal = true;

    VolumeThrottle volumeThrottle;
    volumeThrottle.SetResetFlag(resetFlagVal);
}

TEST(VolumeThrottle, Setter_Getter_Check)
{
    // check the getter and setter for correctionFlag
    uint32_t correctionSet = 1;
    uint32_t correctionReturned;
    // set value
    VolumeThrottle volumeThrottle;
    volumeThrottle.SetCorrectionType(correctionSet);
    // get value
    correctionReturned = volumeThrottle.CorrectionType();

    // values should be equal
    ASSERT_EQ(correctionSet, correctionReturned);
}

TEST(VolumeThrottle, CorrectionType_)
{
}

TEST(VolumeThrottle, GetCorrectionValue_iops_True)
{
    bool iops = true;
    int64_t expected = IOPS_CORRECTION_UNIT / PARAMETER_COLLECTION_INTERVAL;
    int64_t actual;

    VolumeThrottle volumeThrottle;
    actual = volumeThrottle.GetCorrectionValue(iops);
    ASSERT_EQ(expected, actual);
}

TEST(VolumeThrottle, GetCorrectionValue_iops_False)
{
    bool iops = false;
    int64_t expected = ((BW_CORRECTION_UNIT * M_KBYTES * M_KBYTES) / PARAMETER_COLLECTION_INTERVAL);
    int64_t actual;

    VolumeThrottle volumeThrottle;
    actual = volumeThrottle.GetCorrectionValue(iops);
    ASSERT_EQ(expected, actual);
}
} // namespace pos
