#include "src/journal_manager/replay/user_replay_stripe.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/replay/active_user_stripe_replayer_mock.h"
#include "test/unit-tests/journal_manager/replay/active_wb_stripe_replayer_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_event_factory_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_event_mock.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class UserReplayStripeSpy : public UserReplayStripe
{
public:
    using UserReplayStripe::UserReplayStripe;

    ReplayEventList
    GetReplayEventList(void)
    {
        return replayEvents;
    }
};

TEST(UserReplayStripe, UserReplayStripe_)
{
    // Test constructor for product code
    UserReplayStripe stripeForProductCode(0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // Test constructor for unit test
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    UserReplayStripe stripeForUt(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);
}

TEST(UserReplayStripe, AddLog_testIfBlockLogIsAdded)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    UserReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    ON_CALL(*log, GetType).WillByDefault(Return(LogType::BLOCK_WRITE_DONE));
    ON_CALL(*log, GetData).WillByDefault(Return((char*)log));

    NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*status, BlockLogFound);
    EXPECT_CALL(*factory, CreateBlockWriteReplayEvent).WillOnce(Return(replayEvent));

    // When
    ReplayLog replayLog = {
        .time = 0,
        .log = log,
        .segInfoFlushed = false};
    stripe.AddLog(replayLog);

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 1);
    EXPECT_EQ(actual.front(), replayEvent);

    delete log;
}

TEST(UserReplayStripe, AddLog_testIfStripeLogIsAdded)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    UserReplayStripeSpy stripe(nullptr, nullptr, nullptr, nullptr, status, factory, nullptr);

    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    ON_CALL(*log, GetType).WillByDefault(Return(LogType::STRIPE_MAP_UPDATED));
    ON_CALL(*log, GetData).WillByDefault(Return((char*)log));

    NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*status, StripeLogFound);

    // When
    ReplayLog replayLog = {
        .time = 0,
        .log = log,
        .segInfoFlushed = false};
    stripe.AddLog(replayLog);

    // Then
    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.size(), 0);

    delete log;
}

TEST(UserReplayStripe, Replay_testIfFlushedStripeIsReplayed)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    StripeAddr unmapStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = UNMAP_STRIPE};

    ON_CALL(stripeMap, GetLSA).WillByDefault(Return(unmapStripe));
    ON_CALL(*status, IsFlushed).WillByDefault(Return(true));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeAllocation = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeMapUpdate = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeFlush = new NiceMock<MockReplayEvent>;

    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));
    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).WillOnce(Return(stripeAllocation));
    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).WillOnce(Return(stripeMapUpdate));
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).WillOnce(Return(stripeFlush));

    EXPECT_CALL(wbStripeReplayer, Update);
    EXPECT_CALL(userStripeReplayer, Update);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_EQ(result, 0);

    auto actual = stripe.GetReplayEventList();

    EXPECT_EQ(actual.front(), segmentAllocation);
    actual.pop_front();
    EXPECT_EQ(actual.front(), stripeAllocation);
    actual.pop_front();

    EXPECT_EQ(actual.back(), stripeFlush);
    actual.pop_back();
    EXPECT_EQ(actual.back(), stripeMapUpdate);
    actual.pop_back();

    EXPECT_EQ(actual.size(), 5);
}

TEST(UserReplayStripe, Replay_testIfFlushedStripeIsReplayedWhenMappedToWbStripe)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    StripeId userLsid = 1000;
    StripeId wbLsid = 1352;
    StripeAddr wbStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = wbLsid};
    StripeAddr userStripe = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = userLsid};

    ON_CALL(stripeMap, GetLSA).WillByDefault(Return(wbStripe));
    ON_CALL(*status, IsFlushed).WillByDefault(Return(true));
    ON_CALL(*status, GetWbLsid).WillByDefault(Return(wbStripe.stripeId));
    ON_CALL(*status, GetUserLsid).WillByDefault(Return(userStripe.stripeId));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeMapUpdate = new NiceMock<MockReplayEvent>;
    NiceMock<MockReplayEvent>* stripeFlush = new NiceMock<MockReplayEvent>;

    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));
    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).WillOnce(Return(stripeMapUpdate));
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).WillOnce(Return(stripeFlush));

    EXPECT_CALL(wbStripeReplayer, Update);
    EXPECT_CALL(userStripeReplayer, Update);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_EQ(result, 0);

    auto actual = stripe.GetReplayEventList();
    EXPECT_EQ(actual.front(), segmentAllocation);
    actual.pop_front();

    EXPECT_EQ(actual.back(), stripeFlush);
    actual.pop_back();

    EXPECT_EQ(actual.back(), stripeMapUpdate);
    actual.pop_back();

    EXPECT_EQ(actual.size(), 5);
}

TEST(UserReplayStripe, Replay_testIfFlushedStripeIsReplayedWhenMappedToUserStripe)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    StripeId wbLsid = 1352;
    StripeAddr userStripe = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 1000};

    ON_CALL(stripeMap, GetLSA).WillByDefault(Return(userStripe));
    ON_CALL(*status, IsFlushed).WillByDefault(Return(true));
    ON_CALL(*status, GetUserLsid).WillByDefault(Return(userStripe.stripeId));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));

    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).Times(0);

    EXPECT_CALL(wbStripeReplayer, Update);
    EXPECT_CALL(userStripeReplayer, Update);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_EQ(result, 0);

    auto actual = stripe.GetReplayEventList();

    EXPECT_EQ(actual.front(), segmentAllocation);
    actual.pop_front();

    EXPECT_EQ(actual.size(), 5);
}

TEST(UserReplayStripe, Replay_testIfUnflushedStripeIsReplayed)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    StripeAddr unmapStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = UNMAP_STRIPE};

    ON_CALL(stripeMap, GetLSA).WillByDefault(Return(unmapStripe));
    ON_CALL(*status, IsFlushed).WillByDefault(Return(false));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));

    NiceMock<MockReplayEvent>* stripeAllocation = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).WillOnce(Return(stripeAllocation));

    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).Times(0);

    EXPECT_CALL(wbStripeReplayer, Update);
    EXPECT_CALL(userStripeReplayer, Update);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_EQ(result, 0);
    auto actual = stripe.GetReplayEventList();

    EXPECT_EQ(actual.front(), segmentAllocation);
    actual.pop_front();
    EXPECT_EQ(actual.front(), stripeAllocation);
    actual.pop_front();

    EXPECT_EQ(actual.size(), 5);
}

TEST(UserReplayStripe, Replay_testIfUnflushedStripeIsReplayedWhenMapIsUpToDate)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

    ReplayEventList replayEvents;

    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockReplayEvent>* replayEvent = new NiceMock<MockReplayEvent>;
        EXPECT_CALL(*replayEvent, Replay).WillOnce(Return(0));
        replayEvents.push_back(replayEvent);
    }

    StripeAddr wbStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 1023};

    EXPECT_CALL(stripeMap, GetLSA).WillOnce(Return(wbStripe));
    EXPECT_CALL(*status, IsFlushed).WillOnce(Return(false));
    EXPECT_CALL(*status, GetWbLsid).WillOnce(Return(wbStripe.stripeId));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));

    EXPECT_CALL(*factory, CreateStripeAllocationReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeMapUpdateReplayEvent).Times(0);
    EXPECT_CALL(*factory, CreateStripeFlushReplayEvent).Times(0);

    EXPECT_CALL(wbStripeReplayer, Update);
    EXPECT_CALL(userStripeReplayer, Update);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_EQ(result, 0);

    auto actual = stripe.GetReplayEventList();

    EXPECT_EQ(actual.front(), segmentAllocation);
    actual.pop_front();

    EXPECT_EQ(actual.size(), 5);
}

TEST(UserReplayStripe, Replay_testWhenReplayEventFail)
{
    // Given
    NiceMock<MockStripeReplayStatus>* status = new NiceMock<MockStripeReplayStatus>;
    NiceMock<MockReplayEventFactory>* factory = new NiceMock<MockReplayEventFactory>;
    PendingStripeList dummyList;
    NiceMock<MockActiveWBStripeReplayer> wbStripeReplayer(dummyList);
    NiceMock<MockActiveUserStripeReplayer> userStripeReplayer;
    NiceMock<MockIStripeMap> stripeMap;

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

    StripeAddr wbStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 1023};
    EXPECT_CALL(stripeMap, GetLSA).WillOnce(Return(wbStripe));
    EXPECT_CALL(*status, GetWbLsid).WillOnce(Return(wbStripe.stripeId));
    EXPECT_CALL(*status, IsFlushed).WillOnce(Return(false));

    NiceMock<MockReplayEvent>* segmentAllocation = new NiceMock<MockReplayEvent>;
    EXPECT_CALL(*factory, CreateSegmentAllocationReplayEvent).WillOnce(Return(segmentAllocation));

    EXPECT_CALL(wbStripeReplayer, Update).Times(1);
    EXPECT_CALL(userStripeReplayer, Update).Times(1);

    // When
    UserReplayStripeSpy stripe(nullptr, &stripeMap, &wbStripeReplayer, &userStripeReplayer, status, factory, &replayEvents);
    int result = stripe.Replay();

    // Then
    EXPECT_TRUE(result < 0);
}

} // namespace pos
