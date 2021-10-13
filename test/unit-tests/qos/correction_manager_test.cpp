#include "src/qos/correction_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosCorrectionManager, QosCorrectionManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext);
}

TEST(QosCorrectionManager, QosCorrectionManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosCorrectionManager* qosCorrectionManager = new QosCorrectionManager(&mockQoscontext);
    delete qosCorrectionManager;
}

TEST(QosCorrectionManager, Check_Execute_Getter_Setter_Internal_Manager)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext);
    qosCorrectionManager.Execute();
    QosInternalManagerType expectedManager = QosInternalManager_Monitor;
    QosInternalManagerType actualManager = qosCorrectionManager.GetNextManagerType();
    ASSERT_EQ(expectedManager, actualManager);
}

TEST(QosCorrectionManager, Check_Execute_Volume_Wrr_Policy)
{
    NiceMock<MockQosContext> mockQoscontext;
    QosCorrectionManager qosCorrectionManager(&mockQoscontext);
    QosCorrection& correction = mockQoscontext.GetQosCorrection();
    correction.SetCorrectionType(QosCorrection_VolumeThrottle, true);
    correction.SetCorrectionType(QosCorrection_EventWrr, true);
    std::map<uint32_t, uint32_t>& activeVolumeMap = mockQoscontext.GetActiveVolumes();
    activeVolumeMap[1] = 1;
    std::map<uint32_t, map<uint32_t, uint32_t>>& volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    qosCorrectionManager.Execute();
}

} // namespace pos
