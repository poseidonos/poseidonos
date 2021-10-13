#include "src/qos/throttling_policy_deficit.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(ThrottlingPolicyDeficit, Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    ThrottlingPolicyDeficit throttlingPolicy(&mockQoscontext);
}

TEST(ThrottlingPolicyDeficit, Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    ThrottlingPolicyDeficit* throttlingPolicy = new ThrottlingPolicyDeficit(&mockQoscontext);
    delete throttlingPolicy;
}

TEST(ThrottlingPolicyDeficit, GetNewWeight_Test)
{
}

TEST(ThrottlingPolicyDeficit, GetCorrectionType_Test)
{
}

TEST(ThrottlingPolicyDeficit, Reset_Test)
{
}

TEST(ThrottlingPolicyDeficit, IncrementCycleCount_Test)
{
}

} // namespace pos
