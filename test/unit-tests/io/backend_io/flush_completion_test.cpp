#include "src/io/backend_io/flush_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/event/stripe_put_event.h"
#include "src/allocator/wb_stripe_manager/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushCompletion, FlushCompletion_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name{"arr_name"};

    // When: Try to Create New FlushCompletion object with 1 argument
    FlushCompletion flushCompletion(&mockStripe, arr_name);

    // Then: Do nothing
}

TEST(FlushCompletion, FlushCompletion_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    std::string arr_name{"arr_name"};

    // When: Try to Create New FlushCompletion object with 1 argument
    FlushCompletion* flushCompletion = new FlushCompletion(&mockStripe, arr_name);

    // Then: Release memory
    delete flushCompletion;
}

TEST(FlushCompletion, FlushCompletion_Constructor_ThreeArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};

    // When: Try to Create New FlushCompletion object with 3 arguments
    FlushCompletion flushCompletion(&mockStripe, &mockIStripeMap, &mockEventScheduler, arr_name);

    // Then: Do nothing
}

TEST(FlushCompletion, FlushCompletion_Execute_UserArea)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};
    FlushCompletion flushCompletion(&mockStripe, &mockIStripeMap, &mockEventScheduler, arr_name);
    bool actual, expected{true};
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInUserDataArea(_)).WillByDefault(Return(true));

    // When: Try to Execute() when userArea true
    actual = flushCompletion.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

TEST(FlushCompletion, FlushCompletion_Execute_NonUserArea)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};
    FlushCompletion flushCompletion(&mockStripe, &mockIStripeMap, &mockEventScheduler, arr_name);
    bool actual, expected{false};
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInUserDataArea(_)).WillByDefault(Return(false));

    // When: Try to Execute() when userArea false
    actual = flushCompletion.Execute();

    // Then: Receive result as false
    ASSERT_EQ(expected, actual);
}

} // namespace pos
