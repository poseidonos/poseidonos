#include "src/qos/volume_policy.h"

#include "test/unit-tests/qos/qos_context_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;

namespace pos
{
TEST(VolumePolicy, VolumePolicy_Constructor_One_Stack)
{
    NiceMock<MockQosContext> qosContext;
    VolumePolicy volumePolicy(&qosContext);
}

TEST(VolumePolicy, VolumePolicy_Constructor_One_Heap)
{
    NiceMock<MockQosContext> qosContext;
    VolumePolicy* volPolicy = new VolumePolicy(&qosContext);
    delete volPolicy;
}

TEST(VolumePolicy, HandlePolicy)
{
    NiceMock<MockQosContext> qosContext;
    VolumePolicy volPolicy(&qosContext);
    volPolicy.HandlePolicy();
}

} // namespace pos
