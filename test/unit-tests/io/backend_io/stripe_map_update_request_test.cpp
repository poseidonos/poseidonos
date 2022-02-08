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
#include "test/unit-tests/io/backend_io/flush_completion_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/meta_service/i_meta_updater_mock.h"

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
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = new NiceMock<MockFlushCompletion>(&mockStripe, 0);
    CallbackSmartPtr event(mockFlushCompletion);

    // When: Try to Create New StripeMapUpdateRequest object with 3 arguments
    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);

    // Then: Do nothing
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_ExistErrorCount)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = new NiceMock<MockFlushCompletion>(&mockStripe, 0);
    CallbackSmartPtr event(mockFlushCompletion);
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);
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
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = new NiceMock<MockFlushCompletion>(&mockStripe, 0);
    CallbackSmartPtr event(mockFlushCompletion);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);
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

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_CompletionEventNull)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = nullptr;
    CallbackSmartPtr event(mockFlushCompletion);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);
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
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = new NiceMock<MockFlushCompletion>(&mockStripe, 0);
    CallbackSmartPtr event(mockFlushCompletion);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);
    bool actual, expected{false};

    stripeMapUpdateRequest.InformError(IOErrorType::SUCCESS);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInWriteBufferArea(_)).WillByDefault(Return(true));
    ON_CALL(mockMetaUpdater, UpdateStripeMap).WillByDefault(Return(-1));
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    // When: Try to Execute() when userArea false
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

TEST(StripeMapUpdateRequest, StripeMapUpdateRequest_DoSpecificJob_MapUpdatePostponed)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIStripeMap> mockIStripeMap;
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockFlushCompletion>* mockFlushCompletion = new NiceMock<MockFlushCompletion>(&mockStripe, 0);
    CallbackSmartPtr event(mockFlushCompletion);

    StripeMapUpdateRequest stripeMapUpdateRequest(&mockStripe, &mockIStripeMap,
        &mockMetaUpdater, &mockEventScheduler, event, 0);
    bool actual, expected{false};

    stripeMapUpdateRequest.InformError(IOErrorType::SUCCESS);
    StripeAddr stripeAddr;
    ON_CALL(mockStripe, GetVsid()).WillByDefault(Return(0));
    ON_CALL(mockIStripeMap, GetLSA(_)).WillByDefault(Return(stripeAddr));
    ON_CALL(mockIStripeMap, IsInWriteBufferArea(_)).WillByDefault(Return(true));

    // return JOURNAL_LOG_GROUP_FULL error code.
    ON_CALL(mockMetaUpdater, UpdateStripeMap).WillByDefault(Return(3030));
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    // When: Try to Execute() when userArea false
    actual = stripeMapUpdateRequest.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

} // namespace pos
