#include "src/io/backend_io/stripe_map_update_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/allocator_service/allocator_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/backend_event.h"
#include "src/include/branch_prediction.h"
#include "src/io/backend_io/flush_completion.h"
#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
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
TEST(StripeMapUpdateCompletion, StripeMapUpdateCompletion_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe(true);
    std::string arr_name{"arr_name"};

    // When : Create StripeMapUpdateCompletion with single argument
    StripeMapUpdateCompletion stripeMapUpdateCompletion(&mockStripe, arr_name);

    // Then : Do nothing
}

TEST(StripeMapUpdateCompletion, StripeMapUpdateCompletion_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe(true);
    std::string arr_name{"arr_name"};

    // When : Create StripeMapUpdateCompletion with single argument
    StripeMapUpdateCompletion* stripeMapUpdateCompletion =
        new StripeMapUpdateCompletion(&mockStripe, arr_name);

    // Then : Release memory
    delete stripeMapUpdateCompletion;
}

TEST(StripeMapUpdateCompletion, StripeMapUpdateCompletion_Constructor_FourArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe(true);
    NiceMock<MockIContextManager> mockIContextManager;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};

    // When : Create StripeMapUpdateCompletion with four arguments
    StripeMapUpdateCompletion* stripeMapUpdateCompletion =
        new StripeMapUpdateCompletion(&mockStripe,
            &mockIContextManager,
            &mockIStripeMap,
            &mockEventScheduler,
            arr_name);

    // Then : Release memory
    delete stripeMapUpdateCompletion;
}

TEST(StripeMapUpdateCompletion, StripeMapUpdateCompletion_Execute_NormalCase)
{
    // Given
    NiceMock<MockStripe> mockStripe(true);
    NiceMock<MockIContextManager> mockIContextManager;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    std::string arr_name{"arr_name"};
    StripeMapUpdateCompletion stripeMapUpdateCompletion(&mockStripe,
        &mockIContextManager,
        &mockIStripeMap,
        &mockEventScheduler,
        arr_name);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetUserLsid()).WillByDefault(Return(0));
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, SetLSA(_, _, _)).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInUserDataArea(_)).WillByDefault(Return(false));
    ON_CALL(mockIContextManager, UpdateOccupiedStripeCount(_)).WillByDefault(Return());
    bool actual, expected{true};

    // When: FlushCompletion::Execute() returns true (Normal Path)
    actual = stripeMapUpdateCompletion.Execute();

    // Then: Execute normally
    ASSERT_EQ(expected, actual);
}

} // namespace pos
