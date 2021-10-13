#include "src/qos/parameters.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(QosParameters, QosParameters_Constructor_One_Stack)
{
    QosParameters qosParam();
}

TEST(QosParameters, QosParameters_Constructor_One_Heap)
{
    QosParameters* qosParam = new QosParameters();
    delete qosParam;
}

TEST(QosParameters, Check_Reset)
{
    QosParameters qosParam;
    qosParam.Reset();
}

TEST(QosParameters, Check_GetAllVolumeParameter)
{
    QosParameters qosParam;
    AllVolumeParameter allVolParam;
    allVolParam = qosParam.GetAllVolumeParameter();
}

TEST(QosParameters, Check_GetAllEventParameter)
{
    QosParameters qosParam;
    AllEventParameter allEventParam;
    allEventParam = qosParam.GetAllEventParameter();
}

} // namespace pos
