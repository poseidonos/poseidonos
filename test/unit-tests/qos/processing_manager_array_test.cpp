#include "src/qos/processing_manager_array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_avg_compute_mock.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::ReturnRef;

namespace pos
{
TEST(QosProcessingManagerArray, QosProcessingManagerArray_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray qosProcessingManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
}

TEST(QosProcessingManagerArray, QosProcessingManagerArray_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray* qosProcessingManagerArray = new QosProcessingManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    delete qosProcessingManagerArray;
}

TEST(QosProcessingManagerArray, Execute_Run)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    std::map<uint32_t, map<uint32_t, uint32_t>> &volReactorMap = mockQoscontext.GetActiveVolumeReactors();
    volReactorMap[1][1] = 1;
    std::map<uint32_t, uint32_t> &activeVolMap = mockQoscontext.GetActiveVolumes();
    activeVolMap[1] = 1;
    uint32_t arrayId = 0;
    uint32_t volId = 1;
    QosUserPolicy currUserPolicy;
    AllVolumeUserPolicy& currAllVolumePolicy = currUserPolicy.GetAllVolumeUserPolicy();
    VolumeUserPolicy volPolicy;
    volPolicy.SetMaxBandwidth(100);
    volPolicy.SetMaxIops(100);
    volPolicy.SetMinIops(10);
    currAllVolumePolicy.InsertVolumeUserPolicy(arrayId, volId, volPolicy);
    ON_CALL(mockQoscontext, GetQosUserPolicy()).WillByDefault(ReturnRef(currUserPolicy));
    QosParameters parameters;
    AllVolumeParameter& allVolumeParam = parameters.GetAllVolumeParameter();
    VolumeParameter volParam;
    volParam.SetAvgBandwidth(100);
    volParam.SetAvgIops(100);
    allVolumeParam.InsertVolumeParameter(arrayId, volId, volParam);
    ON_CALL(mockQoscontext, GetQosParameters()).WillByDefault(ReturnRef(parameters));
    QosProcessingManagerArray qosProcessingManagerArray(arrayId, &mockQoscontext, &mockQosManager);
    qosProcessingManagerArray.Execute(volId);
}

TEST(QosProcessingManagerArray, Initilize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray qosProcessingManagerArray(arrayIndex, &mockQoscontext, &mockQosManager);
    int length = 100;
    NiceMock<MockMovingAvgCompute> mockMovingAvgCompute(length);

    // EXPECT_CALL(mockMovingAvgCompute, Initilize);
    qosProcessingManagerArray.Initilize();
}

} // namespace pos
