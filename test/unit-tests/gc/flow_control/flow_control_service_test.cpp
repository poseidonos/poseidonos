#include <gtest/gtest.h>
#include <string>
#include "src/gc/flow_control/flow_control_service.h"
 #include <test/unit-tests/gc/flow_control/flow_control_mock.h>
 
using ::testing::Test;
using ::testing::Return;
using ::testing::_;
using ::testing::NiceMock;

namespace pos {

class FlowControlServiceTestFixture : public ::testing::Test
{
public:
    FlowControlServiceTestFixture(void)
    {
    }

    virtual ~FlowControlServiceTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        arrayName = "POSArray";
        flowControlService = new FlowControlService();
    }
    virtual void
    TearDown(void)
    {
        delete flowControlService;
    }
protected:
    FlowControlService* flowControlService;
    std::string arrayName;
};

TEST_F(FlowControlServiceTestFixture, FlowControlService_testRegister)
{
    // Given: A FlowControlService
    // When: Register new FlowControl
    NiceMock<MockFlowControl>* mockFlowControl = new NiceMock<MockFlowControl>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    flowControlService->Register(arrayName, mockFlowControl);
    // Then: Can find FlowControl with GetFlowControl
    FlowControl* expected = mockFlowControl;
    FlowControl* actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);
    delete mockFlowControl;
}

TEST_F(FlowControlServiceTestFixture, FlowControlService_testRegisterTwice)
{
    // Given: A FlowControlService
    // When: Register a FlowControl with array name
    NiceMock<MockFlowControl>* mockFlowControl = new NiceMock<MockFlowControl>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    flowControlService->Register(arrayName, mockFlowControl);
    // Then: Can find FlowControl with GetFlowControl
    FlowControl* expected = mockFlowControl;
    FlowControl* actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);

    // When: Register a different FlowControl with the same array name
    NiceMock<MockFlowControl>* newMockFlowControl = new NiceMock<MockFlowControl>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    flowControlService->Register(arrayName, newMockFlowControl);
    // Then: Does not replace old flow control with new flow control
    expected = mockFlowControl;
    actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);

    delete mockFlowControl;
    delete newMockFlowControl;
}

TEST_F(FlowControlServiceTestFixture, FlowControlService_testUnRegister)
{
    // Given: A FlowControlService
    // When: Register new FlowControl
    NiceMock<MockFlowControl>* mockFlowControl = new NiceMock<MockFlowControl>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    flowControlService->Register(arrayName, mockFlowControl);
    // Then: Can find FlowControl with GetFlowControl
    FlowControl* expected = mockFlowControl;
    FlowControl* actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);
    // When: UnRegister the FlowControl
    flowControlService->UnRegister(arrayName);
    // Then: Cannot find FlowControl with GetFlowControl
    expected = nullptr;
    actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);

    delete mockFlowControl;
}

TEST_F(FlowControlServiceTestFixture, FlowControlService_testUnRegisterWhichNotRegistered)
{
    // Given: A FlowControlService
    // When: Register new FlowControl
    NiceMock<MockFlowControl>* mockFlowControl = new NiceMock<MockFlowControl>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    flowControlService->Register(arrayName, mockFlowControl);
    // Then: Can find FlowControl with GetFlowControl
    FlowControl* expected = mockFlowControl;
    FlowControl* actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);
    // When: UnRegister FlowControl with different array name
    std::string newArrayName = "POSArray2";
    flowControlService->UnRegister(newArrayName);
    // Then: Registered FlowControl still exists
    expected = mockFlowControl;
    actual = flowControlService->GetFlowControl(arrayName);
    EXPECT_EQ(expected, actual);

    delete mockFlowControl;
}

}  // namespace pos
