#include "src/qos/resource.h"

#include <gtest/gtest.h>
/*#include <gmock/gmock.h>
#include "test/unit-tests/qos/resource_array_mock.h"
using ::testing::_;
using ::testing::NiceMock;
*/
namespace pos
{
TEST(QosResource, QosResource_Constructore_One_Stack)
{
    QosResource qosResource;
}

TEST(QosResource, QosResource_Constructore_One_Heap)
{
    QosResource* qosResource = new QosResource();
    delete qosResource;
}

TEST(QosResource, Check_Reset_and_Getter)
{
    QosResource qosResource;
    qosResource.Reset();
    //NiceMock<MockResourceArray> mockResourceArray ;//= new NiceMock<MockResourceArray>();
    //EXPECT_CALL(mockResourceArray, Reset);
    int arrayId = 0;
    ResourceNvramStripes resourceNvramStripes = qosResource.GetResourceNvramStripes(arrayId);
    ResourceArray resourceArray = qosResource.GetResourceArray(arrayId);
    ResourceCpu resourceCpu = qosResource.GetResourceCpu();
    ASSERT_NE(&resourceNvramStripes, NULL);
    ASSERT_NE(&resourceArray, NULL);
    ASSERT_NE(&resourceCpu, NULL);
}

} // namespace pos
