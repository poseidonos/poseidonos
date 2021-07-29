#include <gtest/gtest.h>
#include "src/gc/flow_control/flow_control_configuration.h"
#include "src/array_models/dto/partition_logical_size.h"

#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/master_context/config_manager_mock.h>

using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace pos {

ACTION_P(SetArg2ToBoolAndReturn0, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 0;
}

ACTION_P(SetArg2ToBoolAndReturn1, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 1;
}

ACTION_P(SetArg2ToStringAndReturn0, stringValue)
{
    *static_cast<std::string*>(arg2) = stringValue;
    return 0;
}

ACTION_P(SetArg2ToStringAndReturn1, stringValue)
{
    *static_cast<std::string*>(arg2) = stringValue;
    return 1;
}

ACTION_P(SetArg2ToUint32AndReturn0, uint32Value)
{
    *static_cast<uint32_t*>(arg2) = uint32Value;
    return 0;
}
ACTION_P(SetArg2ToUint32AndReturn1, uint32Value)
{
    *static_cast<uint32_t*>(arg2) = uint32Value;
    return 1;
}

ACTION_P(SetArg2ToUint64AndReturn0, uint64Value)
{
    *static_cast<uint64_t*>(arg2) = uint64Value;
    return 0;
}

ACTION_P(SetArg2ToUint64AndReturn1, uint64Value)
{
    *static_cast<uint64_t*>(arg2) = uint64Value;
    return 1;
}

class FlowControlConfigurationTestFixture : public ::testing::Test
{
public:
    FlowControlConfigurationTestFixture(void)
    {
    }

    virtual ~FlowControlConfigurationTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        mockIArrayInfo = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*mockIArrayInfo, GetSizeInfo(_)).WillOnce(Return(&partitionLogicalSize));
        partitionLogicalSize = {.minWriteBlkCnt = 0, /* no interesting */
                                .blksPerChunk = 64,
                                .blksPerStripe = 2048,
                                .chunksPerStripe = 32,
                                .stripesPerSegment = 1024,
                                .totalStripes = 32,
                                .totalSegments = 32768};
        mockConfigManager = new NiceMock<MockConfigManager>;

        flowControlConfiguration = new FlowControlConfiguration(mockIArrayInfo, mockConfigManager);
    }
    virtual void
    TearDown(void)
    {
        delete flowControlConfiguration;
        delete mockIArrayInfo;
        delete mockConfigManager;
    }
protected:
    FlowControlConfiguration* flowControlConfiguration;

    NiceMock<MockIArrayInfo>* mockIArrayInfo;
    NiceMock<MockConfigManager>* mockConfigManager;
    PartitionLogicalSize partitionLogicalSize;
};

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testFailedToReadEnable)
{
    // When: Read "enable" failed
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn1(false));
    // Then: flow control disabled
    flowControlConfiguration->ReadConfig();
    bool expected_bool = false;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::DISABLE;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testFailedToReadDefaultSetting)
{
    // When: enable flow control but failed to read "default"
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn1(true));
    // Then: flow control enabled and use default setting
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::LINEAR;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testLinearConfig)
{
    // When: enable flow control & use linear strategy
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(false));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "refill_timeout_in_msec", _, _)).WillRepeatedly(SetArg2ToUint64AndReturn0(1000));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "total_token_in_stripe", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(1024));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "strategy", _, _)).WillRepeatedly(SetArg2ToStringAndReturn0("linear"));
    // Then: flow control enabled and use default linear setting
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::LINEAR;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testFailedToReadStateConfig)
{
    // When: enable flow control & but fails to read state config
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(false));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "refill_timeout_in_msec", _, _)).WillRepeatedly(SetArg2ToUint64AndReturn1(1000));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "total_token_in_stripe", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn1(1024));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "strategy", _, _)).WillRepeatedly(SetArg2ToStringAndReturn1("state"));
    // Then: Use linear config
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::LINEAR;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testStateConfigWithRightValue)
{
    // When: enable flow control & use linear strategy
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(false));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "refill_timeout_in_msec", _, _)).WillRepeatedly(SetArg2ToUint64AndReturn0(1000));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "total_token_in_stripe", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(1024));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "strategy", _, _)).WillRepeatedly(SetArg2ToStringAndReturn0("state"));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(25));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(10));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(20));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(5));
    // Then: Use state config
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::STATE;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);

    uint64_t expected_uint64 = 1000 * 1000000ULL;
    uint64_t actual_uint64 = flowControlConfiguration->GetForceResetTimeout();
    EXPECT_EQ(expected_uint64, actual_uint64);

    uint32_t expected_uint32 = 1024 * 2048;
    uint32_t actual_uint32 = flowControlConfiguration->GetTotalToken();
    EXPECT_EQ(expected_uint32, actual_uint32);

    expected_uint32 = 1024;
    actual_uint32 = flowControlConfiguration->GetTotalTokenInStripe();
    EXPECT_EQ(expected_uint32, actual_uint32);

    expected_uint32 = 25;
    actual_uint32 = flowControlConfiguration->GetTargetPercent();
    EXPECT_EQ(expected_uint32, actual_uint32);

    expected_uint32 = 10;
    actual_uint32 = flowControlConfiguration->GetUrgentPercent();
    EXPECT_EQ(expected_uint32, actual_uint32);

    expected_uint32 = 20;
    actual_uint32 = flowControlConfiguration->GetTargetSegment();
    EXPECT_EQ(expected_uint32, actual_uint32);

    expected_uint32 = 5;
    actual_uint32 = flowControlConfiguration->GetUrgentSegment();
    EXPECT_EQ(expected_uint32, actual_uint32);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testStateConfigWithWrongValue)
{
    // When: enable flow control & use linear strategy
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(false));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "refill_timeout_in_msec", _, _)).WillRepeatedly(SetArg2ToUint64AndReturn0(1000));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "total_token_in_stripe", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(1024));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "strategy", _, _)).WillRepeatedly(SetArg2ToStringAndReturn0("state"));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(10));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(25));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(5));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(20));
    // Then: Use state config
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::LINEAR;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

TEST_F(FlowControlConfigurationTestFixture, FlowControlConfiguration_testStateConfigReadFail)
{
    // When: enable flow control & use linear strategy
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "enable", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(true));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "use_default", _, _)).WillRepeatedly(SetArg2ToBoolAndReturn0(false));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "refill_timeout_in_msec", _, _)).WillRepeatedly(SetArg2ToUint64AndReturn0(1000));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "total_token_in_stripe", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn0(1024));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "strategy", _, _)).WillRepeatedly(SetArg2ToStringAndReturn0("state"));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn1(25));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_percent", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn1(10));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_target_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn1(5));
    EXPECT_CALL(*mockConfigManager, GetValue("flow_control", "flow_control_urgent_segment", _, _)).WillRepeatedly(SetArg2ToUint32AndReturn1(20));
    // Then: Use state config
    flowControlConfiguration->ReadConfig();
    bool expected_bool = true;
    bool actual_bool = flowControlConfiguration->GetEnable();
    EXPECT_EQ(expected_bool, actual_bool);

    FlowControlStrategy expected_strategy = FlowControlStrategy::LINEAR;
    FlowControlStrategy actual_strategy = flowControlConfiguration->GetFlowControlStrategy();
    EXPECT_EQ(expected_strategy, actual_strategy);
}

}  // namespace pos
