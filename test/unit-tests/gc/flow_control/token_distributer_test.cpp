#include <gtest/gtest.h>
#include "src/gc/flow_control/token_distributer.h"
#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/flow_control/flow_control_configuration_mock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace pos {

class TokenDistributerTestFixture : public ::testing::Test
{
public:
    TokenDistributerTestFixture(void)
    {
    }

    virtual ~TokenDistributerTestFixture(void)
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

        mockIContextManager = new NiceMock<MockIContextManager>;
        tokenDistributer = new TokenDistributer(mockIArrayInfo, mockFlowControlConfiguration, mockIContextManager);
    }
    virtual void
    TearDown(void)
    {
        delete tokenDistributer;
        delete mockIArrayInfo;
        delete mockFlowControlConfiguration;
        delete mockIContextManager;
    }

protected:
    TokenDistributer* tokenDistributer;

    NiceMock<MockIArrayInfo>* mockIArrayInfo;
    PartitionLogicalSize partitionLogicalSize;
    NiceMock<MockFlowControlConfiguration>* mockFlowControlConfiguration;
    NiceMock<MockIContextManager>* mockIContextManager;

    std::string arrayName;
};

TEST_F(TokenDistributerTestFixture, TokenDistributer_testFreeSegmentMoreThanThreshold)
{
    // Given: Total token is 100 & GC Threshold is 20
    EXPECT_CALL(*mockFlowControlConfiguration, GetTotalToken()).WillOnce(Return(100));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    // When: number of free segment is 30
    uint32_t userTokenActual, gcTokenActual;
    std::tie(userTokenActual, gcTokenActual) = tokenDistributer->Distribute(30);
    // Then: Can find FlowControl with GetFlowControl
    uint32_t userTokenExpected = 100;
    uint32_t gcTokenExpected = 0;
    EXPECT_EQ(userTokenExpected, userTokenActual);
    EXPECT_EQ(gcTokenActual, gcTokenExpected);
}

TEST_F(TokenDistributerTestFixture, TokenDistributer_testFreeSegmentLessThanThreshold)
{
    // Given: Total token is 100 & GC Threshold is 20
    EXPECT_CALL(*mockFlowControlConfiguration, GetTotalToken()).WillOnce(Return(100));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    // When: number of free segment is 30
    uint32_t userTokenActual, gcTokenActual;
    std::tie(userTokenActual, gcTokenActual) = tokenDistributer->Distribute(20);
    // Then: Can find FlowControl with GetFlowControl
    uint32_t userTokenExpected = 50;
    uint32_t gcTokenExpected = 50;
    EXPECT_EQ(userTokenExpected, userTokenActual);
    EXPECT_EQ(gcTokenActual, gcTokenExpected);
}

}  // namespace pos
