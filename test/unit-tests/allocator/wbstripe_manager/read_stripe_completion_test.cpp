#include "src/allocator/wbstripe_manager/read_stripe_completion.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/io_submit_interface/i_io_submit_handler_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace pos
{
TEST(ReadStripeCompletion, _DoSpecificJob_testIfReturnTrueWhenIoSubmitSuccess)
{
    StripeAddr writeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 5 };

    std::vector<void*> buffers;
    for (int i = 0; i < 5; i++) // assume we have only 5 chunks per stripe
    {
        buffers.push_back(malloc(BLOCKS_IN_CHUNK * CHUNK_SIZE));
    }

    CallbackSmartPtr callback(new NiceMock<MockCallback>(false));
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;

    ReadStripeCompletion readStripeCompletion(writeAddr, buffers, callback, 0, &ioSubmitHandler);

    LogicalBlkAddr expectedAddr = {
        .stripeId = 5,
        .offset = 0 };
    EXPECT_CALL(ioSubmitHandler, SubmitAsyncIO(
        IODirection::WRITE,
        _, expectedAddr, BLOCKS_IN_CHUNK,
        PartitionType::WRITE_BUFFER, _, 0)).WillOnce(Return(IOSubmitHandlerStatus::SUCCESS));

    bool success = readStripeCompletion.Execute();
    EXPECT_EQ(success, true);

    for (auto b : buffers)
    {
        free(b);
    }
}

TEST(ReadStripeCompletion, _DoSpecificJob_testIfReturnFalseWhenIoSubmitFails)
{
    StripeAddr writeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 5 };

    std::vector<void*> buffers;
    for (int i = 0; i < 5; i++) // assume we have only 5 chunks per stripe
    {
        buffers.push_back(malloc(BLOCKS_IN_CHUNK * CHUNK_SIZE));
    }

    CallbackSmartPtr callback(new NiceMock<MockCallback>(false));
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;

    ReadStripeCompletion readStripeCompletion(writeAddr, buffers, callback, 0, &ioSubmitHandler);

    LogicalBlkAddr expectedAddr = {
        .stripeId = 5,
        .offset = 0 };
    EXPECT_CALL(ioSubmitHandler, SubmitAsyncIO(
        IODirection::WRITE,
        _, expectedAddr, BLOCKS_IN_CHUNK,
        PartitionType::WRITE_BUFFER, _, 0)).WillOnce(Return(IOSubmitHandlerStatus::FAIL));

    bool success = readStripeCompletion.Execute();
    EXPECT_EQ(success, false);

    for (auto b : buffers)
    {
        free(b);
    }
}
} // namespace pos
