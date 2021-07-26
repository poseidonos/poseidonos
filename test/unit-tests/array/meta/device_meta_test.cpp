#include "src/array/meta/device_meta.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(DeviceMeta, DeviceMeta_testIfAllocationOperatorWorks)
{
    // Given : original device meta data
    DeviceMeta origMeta;
    origMeta.state = ArrayDeviceState::NORMAL;
    origMeta.uid = "sampleUid";

    // When : testing operator =
    DeviceMeta testMeta;
    testMeta = origMeta;

    // Then :
    ASSERT_EQ(origMeta.state, testMeta.state);
    ASSERT_EQ(origMeta.uid, testMeta.uid);
}

} // namespace pos
