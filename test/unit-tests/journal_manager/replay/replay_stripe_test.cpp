#include "src/journal_manager/replay/replay_stripe.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/replay/replay_event_factory_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_event_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class ReplayStripeSpy : public ReplayStripe
{
public:
    using ReplayStripe::ReplayStripe;

    ReplayEventList
    GetReplayEventList(void)
    {
        return replayEvents;
    }
    bool
    GetReplaySegmentInfo(void)
    {
        return replaySegmentInfo;
    }

    void
    CreateSegmentAllocationEvent(void)
    {
        _CreateSegmentAllocationEvent();
    }
    void
    CreateStripeAllocationEvent(void)
    {
        _CreateStripeAllocationEvent();
    }
    void
    CreateStripeFlushReplayEvent(void)
    {
        _CreateStripeFlushReplayEvent();
    }
};

TEST(ReplayStripe, ReplayStripe_testIfConstructedSuccessfully)
{
    // Test constructor for product code
    ReplayStripe stripeForProductCode(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Test constructor for unit test code
    {
        NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
        NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
        ReplayStripe stripeForUt(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);
    }

    {
        NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
        NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
        ReplayStripe* stripe = new ReplayStripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

        delete stripe;
    }
}

TEST(ReplayStripe, AddLog_testAddLogWhenSegInfoFlushed)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;

    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    ReplayLog replayLog = {
        .time = 1,
        .log = nullptr,
        .segInfoFlushed = true};
    EXPECT_CALL(*status, RecordLogFoundTime(replayLog.time));

    // When
    stripe.AddLog(replayLog);

    // Then
    EXPECT_EQ(stripe.GetReplaySegmentInfo(), false);
}

TEST(ReplayStripe, AddLog_testAddLogWhenSegInfoNotFlushed)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;

    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    ReplayLog replayLog = {
        .time = 1,
        .log = nullptr,
        .segInfoFlushed = false};
    EXPECT_CALL(*status, RecordLogFoundTime(replayLog.time));

    // When
    stripe.AddLog(replayLog);

    // Then
    EXPECT_EQ(stripe.GetReplaySegmentInfo(), true);
}

TEST(ReplayStripe, Replay_testWhenAllReplayEventSuccess)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 10; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    // When
    ReplayStripe stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    int actualResult = stripe.Replay();

    // Then
    EXPECT_EQ(actualResult, 0);
}

TEST(ReplayStripe, Replay_testWhenSomeReplayEventsFail)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    NiceMock<MockReplayEvent>* failedEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*failedEvent, Replay).WillOnce(Return(-1));
    replayEvents.push_back(failedEvent);

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).Times(0);
        replayEvents.push_back(replayEvent);
    }

    // When
    ReplayStripe stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    int actualResult = stripe.Replay();

    // Then
    EXPECT_TRUE(actualResult < 0);
}

TEST(ReplayStripe, GetVsid_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayStripe stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    StripeId expected = 100;
    ON_CALL(*status, GetVsid).WillByDefault(Return(expected));

    // When
    StripeId actual = stripe.GetVsid();

    // Then
    EXPECT_EQ(expected, actual);
}

TEST(ReplayStripe, GetVolumeId_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayStripe stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    int expected = 5;
    ON_CALL(*status, GetVolumeId).WillByDefault(Return(expected));

    // When
    int actual = stripe.GetVolumeId();

    // Then
    EXPECT_EQ(expected, actual);
}

TEST(ReplayStripe, DeleteBlockMapReplayEvents_testIfAllBlockEventsDeleted)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, GetType).WillOnce(Return(ReplayEventType::BLOCK_MAP_UPDATE));
        replayEvents.push_back(replayEvent);
    }

    NiceMock<MockReplayEvent>* stripeFlushReplayEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*stripeFlushReplayEvent, GetType).WillOnce(Return(ReplayEventType::STRIPE_MAP_UPDATE));
    replayEvents.push_back(stripeFlushReplayEvent);

    // When
    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    stripe.DeleteBlockMapReplayEvents();

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 1);
    EXPECT_EQ(actual.front(), stripeFlushReplayEvent);
}

TEST(ReplayStripe, _CreateSegmentAllocationEvent_testIfEventAddedToTheFront)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        replayEvents.push_back(replayEvent);
    }

    NiceMock<MockReplayEvent>* segmentAllocationEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocationEvent));

    // When
    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    stripe.CreateSegmentAllocationEvent();

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 6); // 1 replay event should be added
    EXPECT_EQ(actual.front(), segmentAllocationEvent);
}

TEST(ReplayStripe, _CreateStripeAllocationEvent_testIfEventAddedToTheFront)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        replayEvents.push_back(replayEvent);
    }

    NiceMock<MockReplayEvent>* stripeAllocationEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).WillOnce(Return(stripeAllocationEvent));

    // When
    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    stripe.CreateStripeAllocationEvent();

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 6); // 1 replay event should be added
    EXPECT_EQ(actual.front(), stripeAllocationEvent);
}

TEST(ReplayStripe, _CreateStripeFlushReplayEvent_testIfEventsAreAddedToTheLast)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        replayEvents.push_back(replayEvent);
    }

    NiceMock<MockReplayEvent>* stripeMapUpdateEvent = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeFlushEvent = new NiceMock<MockReplayEvent>;

    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).WillOnce(Return(stripeMapUpdateEvent));
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).WillOnce(Return(stripeFlushEvent));

    // When
    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    stripe.CreateStripeFlushReplayEvent();

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 7); // 2 replay events should be added

    EXPECT_EQ(actual.back(), stripeFlushEvent);
    actual.pop_back();

    EXPECT_EQ(actual.back(), stripeMapUpdateEvent);
}

TEST(ReplayStripe, _CreateStripeFlushReplayEvent_testIfStripeFlushEventIsNotAdded)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    ReplayEventList replayEvents;

    NiceMock<MockReplayEvent>* stripeMapUpdateEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).WillOnce(Return(stripeMapUpdateEvent));
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).Times(0);

    ReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, &replayEvents);
    ReplayLog replayLog = {
        .time = 0,
        .log = nullptr,
        .segInfoFlushed = true};
    stripe.AddLog(replayLog);

    // When
    stripe.CreateStripeFlushReplayEvent();

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 1); // 1 replay events should be added
    EXPECT_EQ(actual.back(), stripeMapUpdateEvent);
}

} // namespace pos
