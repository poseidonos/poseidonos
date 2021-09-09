#include "src/journal_manager/replay/replay_stripe_map_update.h"

#include <gtest/gtest.h>

#include "src/include/address_type.h"
#include "test/unit-tests/journal_manager/statistics/stripe_replay_status_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplayStripeMapUpdate, ReplayStripeMapUpdate_testIfConstructedSuccessfully)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 201;
    StripeAddr dest = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 100};
    ReplayStripeMapUpdate stripeMapUpdateEvent(&stripeMap, &status, vsid, dest);
}

TEST(ReplayStripeMapUpdate, Replay_testIfStripeMapIsUpdatedProperly)
{
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockStripeReplayStatus> status;
    StripeId vsid = 201;
    StripeAddr dest = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 100};

    EXPECT_CALL(stripeMap, SetLSA(vsid, dest.stripeId, dest.stripeLoc)).WillOnce(Return(0));

    ReplayStripeMapUpdate stripeMapUpdateEvent(&stripeMap, &status, vsid, dest);
    int result = stripeMapUpdateEvent.Replay();
}

TEST(ReplayStripeMapUpdate, GetType_testIfCorrectTypeIsReturned)
{
    StripeAddr dummy = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = UNMAP_STRIPE};
    ReplayStripeMapUpdate stripeMapUpdateEvent(nullptr, nullptr, 0, dummy);

    EXPECT_EQ(stripeMapUpdateEvent.GetType(), ReplayEventType::STRIPE_MAP_UPDATE);
}

} // namespace pos
