#include "src/journal_manager/replay/replay_stripe_flush.h"

#include <gtest/gtest.h>

#include "src/include/address_type.h"
#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(ReplayStripeFlush, ReplayStripeFlush_testIfConstructedSuccessfully)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;

    ReplayStripeFlush stripeFlushEvent(&contextReplayer, &status, 0, 0, 0);
}

TEST(ReplayStripeFlush, Replay_testIfStripeFlushIsAllRepalyedSuccessfully)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 102;
    StripeId wbLsid = 2;
    StripeId userLsid = 102;

    EXPECT_CALL(contextReplayer, ReplayStripeRelease(wbLsid));
    EXPECT_CALL(contextReplayer, ReplayStripeFlushed(userLsid));
    EXPECT_CALL(status, StripeFlushed);

    ReplayStripeFlush stripeFlushEvent(&contextReplayer, &status, vsid, wbLsid, userLsid);

    int ret = stripeFlushEvent.Replay();
    EXPECT_EQ(ret, 0);
}

TEST(ReplayStripeFlush, Replay_testIfStripeFlushIsRepalyedSuccessfullyWhenWbLsidIsNotKnown)
{
    NiceMock<MockIContextReplayer> contextReplayer;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 102;
    StripeId wbLsid = UNMAP_STRIPE;
    StripeId userLsid = 102;

    EXPECT_CALL(contextReplayer, ReplayStripeRelease).Times(0);
    EXPECT_CALL(contextReplayer, ReplayStripeFlushed(userLsid));
    EXPECT_CALL(status, StripeFlushed);

    ReplayStripeFlush stripeFlushEvent(&contextReplayer, &status, vsid, wbLsid, userLsid);

    int ret = stripeFlushEvent.Replay();
    EXPECT_EQ(ret, 0);
}

TEST(ReplayStripeFlush, GetType_testIfCorrectTypeIsReturned)
{
    ReplayStripeFlush stripeFlushEvent(nullptr, nullptr, 0, 0, 0);
    EXPECT_EQ(stripeFlushEvent.GetType(), ReplayEventType::STRIPE_FLUSH);
}

TEST(ReplayStripeFlush, GetVsid_testIfCorrectAddressIsReturned)
{
    StripeId vsid = 1002;
    StripeId wbLsid = 7;
    StripeId userLsid = 1002;

    ReplayStripeFlush stripeFlushEvent(nullptr, nullptr, vsid, wbLsid, userLsid);

    EXPECT_EQ(stripeFlushEvent.GetVsid(), vsid);
    EXPECT_EQ(stripeFlushEvent.GetWbLsid(), wbLsid);
    EXPECT_EQ(stripeFlushEvent.GetUserLsid(), userLsid);
}

} // namespace pos
