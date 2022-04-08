#include "src/io/backend_io/flush_read_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/flush_submission.h"
#include "src/logger/logger.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushReadCompletion, FlushReadCompletion_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New FlushReadCompletion object with 1 argument
    FlushReadCompletion flushReadCompletion(&mockStripe, 0);

    // Then: Do nothing
}

TEST(FlushReadCompletion, FlushReadCompletion_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New FlushReadCompletion object with 1 argument
    FlushReadCompletion* flushReadCompletion = new FlushReadCompletion(&mockStripe, 0);

    // Then: Release memory
    delete flushReadCompletion;
}

TEST(FlushReadCompletion, FlushReadCompletion_DoSpecificJob_NormalCase)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    FlushReadCompletion flushReadCompletion(&mockStripe, 0, &mockEventScheduler);
    bool actual, expected{true};

    // When: call by base class(Callback)' Execute()
    actual = flushReadCompletion.Execute();

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

} // namespace pos
