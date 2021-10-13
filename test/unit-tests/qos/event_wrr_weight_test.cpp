#include "src/qos/event_wrr_weight.h"

#include <gtest/gtest.h>

#include "src/qos/qos_common.h"

namespace pos
{
TEST(QosEventWrrWeight, QosEventWrrWeight_Constructor_One_Stack)
{
    QosEventWrrWeight qosEventWrrWeight();
}
TEST(QosEventWrrWeight, QosEventWrrWeight_Constructor_One_Heap)
{
    QosEventWrrWeight* qosEventWrrWeight = new QosEventWrrWeight();
    delete qosEventWrrWeight;
}
TEST(QosEventWrrWeight, Check_Resetter_Flag_and_Correction)
{
    QosEventWrrWeight qosEventWrrWeight;

    BackendEvent event = BackendEvent_Start;
    bool expectedResetFlag = 0, actualResetFlag;

    actualResetFlag = qosEventWrrWeight.IsReset(event);
    ASSERT_EQ(actualResetFlag, expectedResetFlag);

    uint32_t expectedCorrectionType = QosCorrectionDir_NoChange;
    uint32_t actualCorrectionType = qosEventWrrWeight.CorrectionType(event);
    ASSERT_EQ(expectedCorrectionType, actualCorrectionType);
}
TEST(QosEventWrrWeight, Check_Setter_Event_Weight)
{
    QosEventWrrWeight qosEventWrrWeight;

    BackendEvent event = BackendEvent_Start;
    int32_t weight = 2;
    qosEventWrrWeight.SetEventWrrWeight(event, weight);
}

TEST(QosEventWrrWeight, Check_Getter_resetFlag)
{
    QosEventWrrWeight qosEventWrrWeight;

    bool expectedResetFlag = 0, actualResetFlag;
    BackendEvent event = BackendEvent_Start;
    actualResetFlag = qosEventWrrWeight.IsReset(event);
    ASSERT_EQ(actualResetFlag, expectedResetFlag);
}

TEST(QosEventWrrWeight, Check_Setter_Getter_CorrectionType)
{
    QosEventWrrWeight qosEventWrrWeight;
    uint32_t corrType = QosCorrectionDir_Increase;
    BackendEvent event = BackendEvent_Start;
    qosEventWrrWeight.SetCorrectionType(event, corrType);

    uint32_t actualCorrectionType;
    actualCorrectionType = qosEventWrrWeight.CorrectionType(event);
    ASSERT_EQ(corrType, actualCorrectionType);
}

} // namespace pos
