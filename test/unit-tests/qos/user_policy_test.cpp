#include "src/qos/user_policy.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(QosUserPolicy, QosUserPolicy_Constructor_One_Stack)
{
    QosUserPolicy qosUserPolicy();
}

TEST(QosUserPolicy, QosUserPolicy_Constructor_One_Heap)
{
    QosUserPolicy* qosUserPolicy = new QosUserPolicy();
    delete qosUserPolicy;
}

TEST(QosUserPolicy, Reset)
{
    QosUserPolicy qosUserPolicy;
    qosUserPolicy.Reset();
}

TEST(QosUserPolicy, GetAllVolumeUserPolicy)
{
    QosUserPolicy qosUserPolicy;
    AllVolumeUserPolicy allVolUserPolicy = qosUserPolicy.GetAllVolumeUserPolicy();
    ASSERT_NE(&allVolUserPolicy, NULL);
}

TEST(QosUserPolicy, GetRebuildUserPolicy)
{
    QosUserPolicy qosUserPolicy;
    RebuildUserPolicy rebuildPolicy = qosUserPolicy.GetRebuildUserPolicy();
    ASSERT_NE(&rebuildPolicy, NULL);
}

TEST(QosUserPolicy, EqualOperator_Test)
{
    QosUserPolicy qosUserPolicy1;
    QosUserPolicy qosUserPolicy2;
    bool equal = (qosUserPolicy1 == qosUserPolicy2);
    ASSERT_EQ(equal, true);
}

} // namespace pos
