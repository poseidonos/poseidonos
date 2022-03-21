#include "src/allocator/wbstripe_manager/read_stripe.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/io_submit_interface/i_io_submit_handler_mock.h"

#include <gtest/gtest.h>

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

namespace pos
{
TEST(ReadStripe, _DoSpecificJob_testIfReturnTrueWhenIoSubmitSuccess)
{
    const uint32_t CHUNK_CNT = 5;
    StripeAddr readAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 10 };

    std::vector<void*> buffers;
    for (int i = 0; i < CHUNK_CNT; i++) // assume we have only 5 chunks per stripe
    {
        buffers.push_back(malloc(BLOCKS_IN_CHUNK * CHUNK_SIZE));
    }

    CallbackSmartPtr callback(new NiceMock<MockCallback>(false));
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;

    ReadStripe readStripe(readAddr, buffers, callback, 0, &ioSubmitHandler);

    LogicalBlkAddr expectedAddr = {
        .stripeId = 10,
        .offset = 0 };
    EXPECT_CALL(ioSubmitHandler, SubmitAsyncIO(
        IODirection::READ,
        _, expectedAddr, BLOCKS_IN_CHUNK * CHUNK_CNT,
        PartitionType::USER_DATA, _, 0)).WillOnce(Return(IOSubmitHandlerStatus::SUCCESS));

    bool success = readStripe.Execute();
    EXPECT_EQ(success, true);

    for (auto b : buffers)
    {
        free(b);
    }
}

TEST(ReadStripe, _DoSpecificJob_testIfReturnFalseWhenIoSubmitFails)
{
    const uint32_t CHUNK_CNT = 5;
    StripeAddr readAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 10 };

    std::vector<void*> buffers;
    for (int i = 0; i < CHUNK_CNT; i++) // assume we have only 5 chunks per stripe
    {
        buffers.push_back(malloc(BLOCKS_IN_CHUNK * CHUNK_SIZE));
    }

    CallbackSmartPtr callback(new NiceMock<MockCallback>(false));
    NiceMock<MockIIOSubmitHandler> ioSubmitHandler;

    ReadStripe readStripe(readAddr, buffers, callback, 0, &ioSubmitHandler);

    LogicalBlkAddr expectedAddr = {
        .stripeId = 10,
        .offset = 0 };
    EXPECT_CALL(ioSubmitHandler, SubmitAsyncIO(
        IODirection::READ,
        _, expectedAddr, BLOCKS_IN_CHUNK * CHUNK_CNT,
        PartitionType::USER_DATA, _, 0)).WillOnce(Return(IOSubmitHandlerStatus::FAIL));

    bool success = readStripe.Execute();
    EXPECT_EQ(success, false);

    for (auto b : buffers)
    {
        free(b);
    }
}
} // namespace pos
