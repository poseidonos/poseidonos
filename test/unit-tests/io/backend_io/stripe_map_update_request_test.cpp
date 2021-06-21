#include "src/io/backend_io/stripe_map_update_request.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/stripe/stripe.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "test/unit-tests/allocator/event/stripe_put_event_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/backend_io/stripe_map_update_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New StripeMapUpdateRequest object with 1 argument
    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, 0);

    // Then: Do nothing
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New StripeMapUpdateRequest object with 1 argument
    StripeMapUpdateRequest* stripeMapUpdateRequest = new StripeMapUpdateRequest(&mockStripe, 0);

    // Then: Release memory
    delete stripeMapUpdateRequest;
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_Constructor_ThreeArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripeMapUpdate>* mockStripeMapUpdate = new NiceMock<MockStripeMapUpdate>(&mockStripe, 0);
    EventSmartPtr event(mockStripeMapUpdate);

    // When: Try to Create New StripeMapUpdateRequest object with 3 arguments
    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockEventScheduler, event, 0);

    // Then: Do nothing
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_ExistErrorCount)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripeMapUpdate>* mockStripeMapUpdate = new NiceMock<MockStripeMapUpdate>(&mockStripe, 0);
    EventSmartPtr event(mockStripeMapUpdate);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockEventScheduler, event, 0);
    stripeMapUpdateRequest.InformError(IOErrorType::GENERIC_ERROR);
    bool actual, expected{true};

    // When: Try to Execute() when _GetErrorCount() > 0 true
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_NonUserArea)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripeMapUpdate>* mockStripeMapUpdate = new NiceMock<MockStripeMapUpdate>(&mockStripe, 0);
    EventSmartPtr event(mockStripeMapUpdate);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockEventScheduler, event, 0);
    bool actual, expected{true};

    stripeMapUpdateRequest.InformError(IOErrorType::SUCCESS);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInWriteBufferArea(_)).WillByDefault(Return(false));

    // When: Try to Execute() when userArea false
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_StripeMapUpdateNull)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    MockStripeMapUpdate* mockStripeMapUpdate = nullptr;
    EventSmartPtr event(mockStripeMapUpdate);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockEventScheduler, event, 0);
    bool actual, expected{true};

    stripeMapUpdateRequest.InformError(IOErrorType::SUCCESS);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInWriteBufferArea(_)).WillByDefault(Return(true));

    // When: Try to Execute() when userArea false
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_MapUpdateFail)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripeMapUpdate>* mockStripeMapUpdate = new NiceMock<MockStripeMapUpdate>(&mockStripe, 0);
    EventSmartPtr event(mockStripeMapUpdate);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockEventScheduler, event, 0);
    bool actual, expected{true};

    stripeMapUpdateRequest.InformError(IOErrorType::SUCCESS);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInWriteBufferArea(_)).WillByDefault(Return(true));
    ON_CALL(*mockStripeMapUpdate, Execute()).WillByDefault(Return(false));
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);

    // When: Try to Execute() when userArea false
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

} // namespace pos
