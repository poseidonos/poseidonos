#include "src/journal_manager/replay/replay_stripe_allocation.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplayStripeAllocation, ReplayStripeAllocation_testIfConstructedSuccessfully)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 102;
    StripeId wbLsid = 11;

    ReplayStripeAllocation stripeAllocationEvent(&stripeMap, &contextReplayer, &status, vsid, wbLsid);
}

TEST(ReplayStripeAllocation, Replay_testIfStripeAllocationIsReplayedSuccessfully)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 102;
    StripeId wbLsid = 11;

    ReplayStripeAllocation stripeAllocationEvent(&stripeMap, &contextReplayer, &status, vsid, wbLsid);

    EXPECT_CALL(stripeMap, SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA)).WillOnce(Return(0));
    EXPECT_CALL(contextReplayer, ReplayStripeAllocation(wbLsid, vsid));
    EXPECT_CALL(status, StripeAllocated);

    int result = stripeAllocationEvent.Replay();
    EXPECT_EQ(result, 0);
}

TEST(ReplayStripeAllocation, Replay_testIfReplayFailedWhenStripeMapUpdateFailed)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 102;
    StripeId wbLsid = 11;

    ReplayStripeAllocation stripeAllocationEvent(&stripeMap, &contextReplayer, &status, vsid, wbLsid);

    EXPECT_CALL(stripeMap, SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA)).WillOnce(Return(-1));
    EXPECT_CALL(contextReplayer, ReplayStripeAllocation(wbLsid, vsid)).Times(0);
    EXPECT_CALL(status, StripeAllocated).Times(0);

    int result = stripeAllocationEvent.Replay();
    EXPECT_EQ(result, -1);
}

TEST(ReplayStripeAllocation, GetType_testIfCorrectTypeIsReturned)
{
    ReplayStripeAllocation stripeAllocationEvent(nullptr, nullptr, nullptr, 0, 0);
    EXPECT_EQ(stripeAllocationEvent.GetType(), ReplayEventType::STRIPE_ALLOCATION);
}

} // namespace pos
