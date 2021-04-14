#include "src/io/backend_io/flush_read_submission.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/backend_io/flush_read_completion.h"
#include "src/logger/logger.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/io_submit_interface/i_io_submit_handler_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
    
TEST(FlushReadSubmission, FlushReadSubmission_Constructor_OneArgument_Stack)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New flushReadSubmission object with 1 argument
    FlushReadSubmission flushReadSubmission(&mockStripe, "");

    // Then: Do nothing
}

TEST(FlushReadSubmission, FlushReadSubmission_Constructor_OneArgument_Heap)
{
    // Given
    NiceMock<MockStripe> mockStripe;

    // When: Try to Create New flushReadSubmission object with 1 argument
    FlushReadSubmission* flushReadSubmission = new FlushReadSubmission(&mockStripe, "");

    // Then: Release memory
    delete flushReadSubmission;
}

TEST(FlushReadSubmission, FlushReadSubmission_Constructor_TwoArguments)
{
    // Given
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIIOSubmitHandler>* ioSubmitHandler;

    // When: Try to Create New flushReadSubmission object with 2 arguments
    FlushReadSubmission flushReadSubmission(&mockStripe, ioSubmitHandler, "");

    // Then: Do nothing
}

TEST(FlushReadSubmission, FlushReadSubmission_Execute_NormalCase)
{
    // Given : iosubmit handler returns SUCCESS
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;
    ON_CALL(ioSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::SUCCESS));
    FlushReadSubmission flushReadSubmission(&mockStripe, &ioSubmitHandler, "");
    bool actual, expected{true};

    // When: Execute()
    actual = flushReadSubmission.Execute();

    // Then: Expect result as true
    ASSERT_EQ(actual, expected);
}

TEST(FlushReadSubmission, FlushReadSubmission_Execute_NormalCase_VectorCheck)
{
    // Given : iosubmit handler returns SUCCESS, and add vector data
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;
    ON_CALL(ioSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::SUCCESS));

    FlushReadSubmission flushReadSubmission(&mockStripe, &ioSubmitHandler, "");
    bool actual, expected{true};

    NiceMock<std::vector<void*>> vectors;
    vectors.push_back((void*)0x1);
    vectors.push_back((void*)0x2);
    ON_CALL(mockStripe, DataBufferBegin()).WillByDefault(Return(vectors.begin()));
    ON_CALL(mockStripe, DataBufferEnd()).WillByDefault(Return(vectors.end()));

    // When: Execute()
    actual = flushReadSubmission.Execute();

    // Then: Expect result as True
    ASSERT_EQ(actual, expected);
}

TEST(FlushReadSubmission, FlushReadSubmission_Execute_ErrorCase)
{
    // Given : iosubmit handler returns TRYLOCK_FAIL
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;
    ON_CALL(ioSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::TRYLOCK_FAIL));
    FlushReadSubmission flushReadSubmissionTryFail(&mockStripe, &ioSubmitHandler, "");
    bool actual, expected{false};

    // When: Execute()
    actual = flushReadSubmissionTryFail.Execute();

    // Then: Expect result as False
    ASSERT_EQ(actual, expected);

    // Given : iosubmit handler returns FAIL_IN_SYSTEM_STOP
    ON_CALL(ioSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, _)).WillByDefault(Return(IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP));
    FlushReadSubmission flushReadSubmissionSystemFail(&mockStripe, &ioSubmitHandler, "");

    // When: Execute()
    actual = flushReadSubmissionSystemFail.Execute();

    // Then: Expect result as False
    ASSERT_EQ(actual, expected);
}

} // namespace pos
