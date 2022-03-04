#include <gtest/gtest.h>
#include "src/gc/flow_control/flow_control.h"
#include "src/array_models/dto/partition_logical_size.h"
#include "src/include/pos_event_id.h"

#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/lib/system_timeout_checker_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/flow_control/flow_control_service_mock.h>
#include <test/unit-tests/gc/flow_control/token_distributer_mock.h>
#include <test/unit-tests/gc/flow_control/flow_control_configuration_mock.h>
#include <test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h>
#include <test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace pos {
class FlowControlTestFixture : public ::testing::Test
{
public:
    FlowControlTestFixture(void)
    {
    }

    virtual ~FlowControlTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mockIArrayInfo = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*mockIArrayInfo, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        mockIContextManager = new NiceMock<MockIContextManager>;
        partitionLogicalSize = {.minWriteBlkCnt = 0, /* no interesting */
                                .blksPerChunk = 64,
                                .blksPerStripe = 2048,
                                .chunksPerStripe = 32,
                                .stripesPerSegment = 1024,
                                .totalStripes = 32,
                                .totalSegments = 32768};
        mockSystemTimeoutChecker = new NiceMock<MockSystemTimeoutChecker>;
        mockFlowControlService = new NiceMock<MockFlowControlService>;
        mockTokenDistributer = new NiceMock<MockTokenDistributer>(nullptr, nullptr, nullptr);
        mockFlowControlConfiguration = new NiceMock<MockFlowControlConfiguration>(mockIArrayInfo, nullptr);

        flowControl = new FlowControl(mockIArrayInfo, mockIContextManager,
            mockSystemTimeoutChecker, mockFlowControlService, mockTokenDistributer,
            mockFlowControlConfiguration);

        mockSegmentCtx = new NiceMock<MockSegmentCtx>;
        mockGcCtx = new NiceMock<MockGcCtx>;
    }

    virtual void
    TearDown(void)
    {
        delete flowControl;
        delete mockIArrayInfo;
        delete mockIContextManager;
        delete mockFlowControlService;
        delete mockSegmentCtx;
        delete mockGcCtx;
    }
protected:
    FlowControl* flowControl;

    NiceMock<MockIArrayInfo>* mockIArrayInfo;
    NiceMock<MockIContextManager>* mockIContextManager;
    PartitionLogicalSize partitionLogicalSize;
    NiceMock<MockSystemTimeoutChecker>* mockSystemTimeoutChecker;
    NiceMock<MockFlowControlService>* mockFlowControlService;
    NiceMock<MockTokenDistributer>* mockTokenDistributer;
    NiceMock<MockFlowControlConfiguration>* mockFlowControlConfiguration;
    NiceMock<MockSegmentCtx>* mockSegmentCtx;
    NiceMock<MockGcCtx>* mockGcCtx;
};

TEST_F(FlowControlTestFixture, Init_testInit)
{
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillOnce(Return(FlowControlStrategy::LINEAR));

    int expected = 0;
    int actual = flowControl->Init();

    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, Dispose_testDispose)
{
    flowControl->Dispose();
}

TEST_F(FlowControlTestFixture, Shutdown_testShutdown)
{
    flowControl->Shutdown();
}

TEST_F(FlowControlTestFixture, Flush_testFlush)
{
    flowControl->Flush();
}

TEST_F(FlowControlTestFixture, GetToken_testGetTokenWithNegativeToken)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillOnce(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: User requests negative number of tokens
    FlowControlType type = FlowControlType::USER;
    // Then: Return 0 tokens
    int expected, token;
    token = -10;
    expected = 0;
    int actual = flowControl->GetToken(type, token);

    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, GetToken_testUserGetTokenWhenFlowControlDisabled)
{
    // Given: FlowControl disabled
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::DISABLE));
    flowControl->Init();
    // When: User requests positive number of tokens
    FlowControlType type = FlowControlType::USER;
    // Then: Return requested tokens
    int expected, token;
    expected = token = 30;
    int actual = flowControl->GetToken(type, token);

    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, GetToken_testWhenFreeSegmentMoreThanGCThreshold)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: number of free segments > gc normal threshold
    EXPECT_CALL(*mockSegmentCtx, GetNumOfFreeSegmentWoLock()).WillOnce(Return(30));
    // Then: Return requested tokens
    FlowControlType type = FlowControlType::USER;
    int expected, token;
    expected = token = 30;
    int actual = flowControl->GetToken(type, token);

    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, GetToken_testWhenFreeSegmentLessThanGCThresholdLinear)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: number of free segments < gc normal threshold, token more than 0
    EXPECT_CALL(*mockSegmentCtx, GetNumOfFreeSegmentWoLock()).WillRepeatedly(Return(15));
    EXPECT_CALL(*mockTokenDistributer, Distribute(15)).WillOnce(Return(std::make_tuple(100, 100)));
    // Then: Return requested tokens
    FlowControlType type = FlowControlType::USER;
    int expected, token;
    expected = token = 30;
    int actual = flowControl->GetToken(type, token);

    EXPECT_EQ(expected, actual);
}


TEST_F(FlowControlTestFixture, GetToken_testWhenFreeSegmentLessThanGCThresholdState)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::STATE));
    flowControl->Init();
    // When: number of free segments < gc normal threshold, token more than 0
    EXPECT_CALL(*mockSegmentCtx, GetNumOfFreeSegmentWoLock()).WillRepeatedly(Return(15));
    EXPECT_CALL(*mockTokenDistributer, Distribute(15)).WillOnce(Return(std::make_tuple(100, 100)));
    // Then: Return requested tokens
    FlowControlType type = FlowControlType::USER;
    int expected, token;
    expected = token = 30;
    int actual = flowControl->GetToken(type, token);

    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, GetToken_testForceRefill)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: number of free segments < gc normal threshold, fails to get token
    EXPECT_CALL(*mockSegmentCtx, GetNumOfFreeSegmentWoLock()).WillRepeatedly(Return(15));
    EXPECT_CALL(*mockTokenDistributer, Distribute(15)).WillOnce(Return(std::make_tuple(0, 100)));
    // Then: try to forceReset & refill token
    FlowControlType type = FlowControlType::USER;
    int expected, token, actual;
    token = 30;
    expected = 0;
    actual = flowControl->GetToken(type, token); // fill up buckets bucket=(0,100), oldBucket=(0,0)
    EXPECT_EQ(expected, actual);

    actual = flowControl->GetToken(type, token); // fill up old bucket bucket=(0,100), oldBucket=(0,100)
    EXPECT_EQ(expected, actual);

    EXPECT_CALL(*mockSystemTimeoutChecker, SetTimeout(_)).WillOnce(Return());
    actual = flowControl->GetToken(type, token); // start systemTimeoutChecker bucket=(0,100), oldBucket=(0,100)
    EXPECT_EQ(expected, actual);

    FlowControlType otherType = FlowControlType::GC;
    expected = 30;
    actual = flowControl->GetToken(otherType, token); // other type gets token bucket=(0,70), oldBucket=(0,100)
    EXPECT_EQ(expected, actual);

    expected = 0;
    actual = flowControl->GetToken(type, token); // as bucket != oldBucket, force reset fails once bucket=(0,70), oldBucket=(0,70)
    EXPECT_EQ(expected, actual);

    EXPECT_CALL(*mockSystemTimeoutChecker, SetTimeout(_)).WillOnce(Return());
    actual = flowControl->GetToken(type, token); // start systemTimeoutChecker bucket=(0,70), oldBucket=(0,70)
    EXPECT_EQ(expected, actual);

    EXPECT_CALL(*mockSystemTimeoutChecker, CheckTimeout()).WillOnce(Return(false));
    actual = flowControl->GetToken(type, token); // fail to pass CheckTimeOut
    EXPECT_EQ(expected, actual);

    EXPECT_CALL(*mockSystemTimeoutChecker, CheckTimeout()).WillOnce(Return(true));
    EXPECT_CALL(*mockTokenDistributer, Distribute(15)).WillOnce(Return(std::make_tuple(100, 100)));
    expected = token;
    actual = flowControl->GetToken(type, token); // force reset triggered & token refilled
    EXPECT_EQ(expected, actual);
}

TEST_F(FlowControlTestFixture, ReturnToken_whenFlowControlDisabled)
{
    // Given: FlowControl disabled
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::DISABLE));
    flowControl->Init();
    // When: User tries to return token
    // Then: Nothing happens
    FlowControlType type = FlowControlType::USER;
    int token = 30;
    flowControl->ReturnToken(type, token);
}

TEST_F(FlowControlTestFixture, ReturnToken_testReturnTokenWithNegativeToken)
{
    // Given: FlowControl disabled
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: User tries to return negative number of token
    // Then: Nothing happens
    FlowControlType type = FlowControlType::USER;
    int token = -10;
    flowControl->ReturnToken(type, token);
}

TEST_F(FlowControlTestFixture, ReturnToken_testReturnToken)
{
    // Given: FlowControl enabled & use default configuration
    EXPECT_CALL(*mockIContextManager, GetSegmentCtx).WillRepeatedly(Return(mockSegmentCtx));
    EXPECT_CALL(*mockIContextManager, GetGcCtx).WillRepeatedly(Return(mockGcCtx));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_NORMAL_GC)).WillOnce(Return(20));
    EXPECT_CALL(*mockIContextManager, GetGcThreshold(GcMode::MODE_URGENT_GC)).WillOnce(Return(5));
    EXPECT_CALL(*mockIArrayInfo, GetName()).WillOnce(Return("POSArray"));
    EXPECT_CALL(*mockFlowControlService, Register(_, _)).WillOnce(Return());

    EXPECT_CALL(*mockFlowControlConfiguration, ReadConfig()).WillOnce(Return());
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalToken()).WillOnce(Return(partitionLogicalSize.blksPerStripe * partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration,
        GetTotalTokenInStripe()).WillOnce(Return(partitionLogicalSize.stripesPerSegment));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetSegment()).WillOnce(Return(20));
    EXPECT_CALL(*mockFlowControlConfiguration, GetTargetPercent()).WillOnce(Return(25));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentSegment()).WillOnce(Return(5));
    EXPECT_CALL(*mockFlowControlConfiguration, GetUrgentPercent()).WillOnce(Return(10));
    EXPECT_CALL(*mockFlowControlConfiguration, GetFlowControlStrategy()).WillRepeatedly(Return(FlowControlStrategy::LINEAR));
    flowControl->Init();
    // When: User tries to return token
    // Then: nothing happens
    FlowControlType type = FlowControlType::USER;
    int token = 30;
    flowControl->ReturnToken(type, token);
}

TEST_F(FlowControlTestFixture, InitDistributer_testInitDistributer)
{
    EXPECT_CALL(*mockTokenDistributer, Init()).WillOnce(Return());
    flowControl->InitDistributer();
}

}  // namespace pos
