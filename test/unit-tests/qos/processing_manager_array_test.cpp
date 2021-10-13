#include "src/qos/processing_manager_array.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_avg_compute_mock.h"
#include "test/unit-tests/qos/qos_context_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosProcessingManagerArray, QosProcessingManagerArray_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray qosProcessingManagerArray(arrayIndex, &mockQoscontext);
}

TEST(QosProcessingManagerArray, QosProcessingManagerArray_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray* qosProcessingManagerArray = new QosProcessingManagerArray(arrayIndex, &mockQoscontext);
    delete qosProcessingManagerArray;
}

TEST(QosProcessingManagerArray, Execute_)
{
}

TEST(QosProcessingManagerArray, Initilize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    QosProcessingManagerArray qosProcessingManagerArray(arrayIndex, &mockQoscontext);
    int length = 100;
    NiceMock<MockMovingAvgCompute> mockMovingAvgCompute(length);

    //EXPECT_CALL(mockMovingAvgCompute, Initilize);
    qosProcessingManagerArray.Initilize();
}

} // namespace pos
