#include "src/journal_manager/replay/replay_event_factory.h"

#include <gtest/gtest.h>

#include "src/journal_manager/replay/replay_stripe_flush.h"

namespace pos
{
TEST(ReplayEventFactory, CreateBlockWriteReplayEvent_)
{
}

TEST(ReplayEventFactory, CreateStripeMapUpdateReplayEvent_)
{
}

TEST(ReplayEventFactory, CreateStripeFlushReplayEvent_testIfReplayEventIsCreatedProperly)
{
    // Given
    ReplayEventFactory factory(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    StripeId vsid = 200;
    StripeId wbLsid = 10;
    StripeId userLsid = 200;

    // When
    ReplayEvent* replayEvent = factory.CreateStripeFlushReplayEvent(vsid, wbLsid, userLsid);

    // Then
    ReplayStripeFlush* stripeFlushEvent = dynamic_cast<ReplayStripeFlush*>(replayEvent);
    EXPECT_EQ(stripeFlushEvent->GetVsid(), vsid);
    EXPECT_EQ(stripeFlushEvent->GetWbLsid(), wbLsid);
    EXPECT_EQ(stripeFlushEvent->GetUserLsid(), userLsid);

    delete replayEvent;
}

TEST(ReplayEventFactory, CreateStripeAllocationReplayEvent_)
{
}

TEST(ReplayEventFactory, CreateSegmentAllocationReplayEvent_)
{
}

} // namespace pos
