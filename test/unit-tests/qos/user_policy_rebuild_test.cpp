#include "src/qos/user_policy_rebuild.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"

namespace pos
{
TEST(RebuildUserPolicy, RebuildUserPolicy_Constructor_One_Stack)
{
    RebuildUserPolicy rebuildUserPolicy();
}

TEST(RebuildUserPolicy, RebuildUserPolicy_Constructor_One_Heap)
{
    RebuildUserPolicy* rebuildUserPolicy = new RebuildUserPolicy();
    delete rebuildUserPolicy;
}

TEST(RebuildUserPolicy, Reset)
{
    RebuildUserPolicy rebuildUserPolicy;
    rebuildUserPolicy.Reset();
    uint8_t setRebuildImpact = rebuildUserPolicy.GetRebuildImpact();
    ASSERT_EQ(setRebuildImpact, PRIORITY_DEFAULT);
}

TEST(RebuildUserPolicy, GetterSetterRebuildImpact_Valid)
{
    RebuildUserPolicy rebuildUserPolicy;
    rebuildUserPolicy.Reset();
    qos_backend_policy rebuildPolicy;
    rebuildPolicy.priorityImpact = PRIORITY_HIGH;
    rebuildUserPolicy.SetRebuildImpact(rebuildPolicy.priorityImpact);
    uint8_t setRebuildImpact = rebuildUserPolicy.GetRebuildImpact();
    ASSERT_EQ(setRebuildImpact, PRIORITY_HIGH);
}

TEST(RebuildUserPolicy, SetRebuildImpact_Invalid)
{
    RebuildUserPolicy rebuildUserPolicy;
    rebuildUserPolicy.SetRebuildImpact(10);
    uint8_t setRebuildImpact = rebuildUserPolicy.GetRebuildImpact();
    ASSERT_EQ(setRebuildImpact, PRIORITY_DEFAULT);
}

TEST(RebuildUserPolicy, SetRebuildImpact_Invalid_Less)
{
    RebuildUserPolicy rebuildUserPolicy;
    rebuildUserPolicy.SetRebuildImpact(0);
    uint8_t setRebuildImpact = rebuildUserPolicy.GetRebuildImpact();
    ASSERT_EQ(setRebuildImpact, PRIORITY_DEFAULT);
}

TEST(RebuildUserPolicy, SetPolicyChange)
{
    RebuildUserPolicy rebuildUserPolicy;
    rebuildUserPolicy.SetPolicyChange(true);
}

TEST(RebuildUserPolicy, EqualOperator_Test)
{
    RebuildUserPolicy rebuildUserPolicy1;
    RebuildUserPolicy rebuildUserPolicy2;
    rebuildUserPolicy1.SetRebuildImpact(10);
    rebuildUserPolicy2.SetRebuildImpact(10);
    bool equal = (rebuildUserPolicy1 == rebuildUserPolicy2);
    ASSERT_EQ(equal, true);
}

} // namespace pos
