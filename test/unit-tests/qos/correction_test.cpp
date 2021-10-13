#include "src/qos/correction.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(QosCorrection, QosCorrection_Constructor_One_Stack)
{
    QosCorrection qosCorrection();
}
TEST(QosCorrection, QosCorrection_Constructor_One_Heap)
{
    QosCorrection* qosCorrection = new QosCorrection();
    delete qosCorrection;
}

TEST(QosCorrection, Check_Reset)
{
    QosCorrection qosCorrection;
    qosCorrection.Reset();
    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    uint32_t size = allVolumeThrottle.GetVolumeThrottleMap().size();
    ASSERT_EQ(size, 0);
}

TEST(QosCorrection, Check_Getter_GetVolumeThrottlePolicy)
{
    QosCorrection qosCorrection;

    AllVolumeThrottle& allVolumeThrottle = qosCorrection.GetVolumeThrottlePolicy();
    ASSERT_NE(&allVolumeThrottle, NULL);
}

TEST(QosCorrection, Check_Getter_EventWrrWeightPolicy)
{
    QosCorrection qosCorrection;
    QosEventWrrWeight& eventWrrWeight = qosCorrection.GetEventWrrWeightPolicy();
    ASSERT_NE(&eventWrrWeight, NULL);
}

TEST(QosCorrection, Check_Getter_Setter_CorrectionType)
{
    QosCorrection qosCorrection;
    QosCorrectionType correction = QosCorrection_VolumeThrottle;
    bool correctionValue = true;
    qosCorrection.SetCorrectionType(correction, correctionValue);

    qos_correction_type returnCorrectionType = qosCorrection.GetCorrectionType();

    ASSERT_EQ(returnCorrectionType.volumeThrottle, correctionValue);
}

} // namespace pos
