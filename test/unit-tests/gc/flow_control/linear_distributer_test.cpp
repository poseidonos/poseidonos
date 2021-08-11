#include <gtest/gtest.h>
#include "src/gc/flow_control/linear_distributer.h"
#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/flow_control/flow_control_configuration_mock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace pos {

class LinearDistributerTestFixture : public ::testing::Test
{
public:
    LinearDistributerTestFixture(void)
    {
    }

    virtual ~LinearDistributerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        arrayName = "POSArray";
        mockIArrayInfo = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*mockIArrayInfo, GetName()).WillRepeatedly(Return(arrayName));
        partitionLogicalSize = {.minWriteBlkCnt = 0, /* not interesting */
                                .blksPerChunk = 64,
                                .blksPerStripe = 2048,
                                .chunksPerStripe = 32,
                                .stripesPerSegment = 1024,
                                .totalStripes = 32,
                                .totalSegments = 32768};

        EXPECT_CALL(*mockIArrayInfo, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        mockFlowControlConfiguration = new NiceMock<MockFlowControlConfiguration>(mockIArrayInfo, nullptr);
        uint32_t totalTokenInStripe = partitionLogicalSize.stripesPerSegment;
        uint32_t totalToken = partitionLogicalSize.stripesPerSegment * partitionLogicalSize.blksPerStripe;
        EXPECT_CALL(*mockFlowControlConfiguration, GetTotalTokenInStripe()).WillRepeatedly(Return(totalTokenInStripe));
        EXPECT_CALL(*mockFlowControlConfiguration, GetTotalToken()).WillRepeatedly(Return(totalToken));

        mockIContextManager = new NiceMock<MockIContextManager>;
        EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillRepeatedly(Return(20));
        EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillRepeatedly(Return(5));

        linearDistributer = new LinearDistributer(mockIArrayInfo, mockFlowControlConfiguration, mockIContextManager);
    }
    virtual void
    TearDown(void)
    {
        delete linearDistributer;
        delete mockIArrayInfo;
        delete mockFlowControlConfiguration;
        delete mockIContextManager;
    }

protected:
    LinearDistributer* linearDistributer;

    NiceMock<MockIArrayInfo>* mockIArrayInfo;
    PartitionLogicalSize partitionLogicalSize;
    NiceMock<MockFlowControlConfiguration>* mockFlowControlConfiguration;
    NiceMock<MockIContextManager>* mockIContextManager;

    std::string arrayName;
};

TEST_F(LinearDistributerTestFixture, LinearDistributer_testFreeSegmentMoreThanThreshold)
{
    // Given: Total token is 2097152 & GC Threshold is 20
    // When: number of free segment is 30
    uint32_t userTokenActual, gcTokenActual;
    std::tie(userTokenActual, gcTokenActual) = linearDistributer->Distribute(30);
    // Then: Can find FlowControl with GetFlowControl
    uint32_t userTokenExpected = 2097152;
    uint32_t gcTokenExpected = 0;
    EXPECT_EQ(userTokenExpected, userTokenActual);
    EXPECT_EQ(gcTokenActual, gcTokenExpected);
}

TEST_F(LinearDistributerTestFixture, LinearDistributer_testFreeSegmentLessThanThreshold)
{
    // Given: Total token is 2097152 & GC Threshold is 20
    // When: number of free segment is 30
    uint32_t userTokenActual, gcTokenActual;
    std::tie(userTokenActual, gcTokenActual) = linearDistributer->Distribute(20);
    // Then: Can find FlowControl with GetFlowControl
    uint32_t userTokenExpected = 1048576;
    uint32_t gcTokenExpected = 1048576;
    EXPECT_EQ(userTokenExpected, userTokenActual);
    EXPECT_EQ(gcTokenActual, gcTokenExpected);
}

TEST_F(LinearDistributerTestFixture, LinearDistributer_testFreeSegmentLessThanUrgentThreshold)
{
    // Given: Total token is 2097152 & GC Threshold is 20
    // When: number of free segment is 30
    uint32_t userTokenActual, gcTokenActual;
    std::tie(userTokenActual, gcTokenActual) = linearDistributer->Distribute(5);
    // Then: Can find FlowControl with GetFlowControl
    uint32_t userTokenExpected = 0;
    uint32_t gcTokenExpected = 2097152;
    EXPECT_EQ(userTokenExpected, userTokenActual);
    EXPECT_EQ(gcTokenActual, gcTokenExpected);
}

}  // namespace pos
