#include "src/metadata/stripe_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/stripe_manager/stripe_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/metadata/segment_context_updater_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(StripeMapUpdate, StripeMapUpdate_testIfConstructedSuccessfully)
{
    StripeSmartPtr stripe = StripeSmartPtr(new NiceMock<MockStripe>());
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockSegmentContextUpdater> segmentCtxUpdater;

    StripeMapUpdate stripeMapUpdate(stripe, &stripeMap, &segmentCtxUpdater);

    StripeMapUpdate* stripeMapUpdateInHeap = new StripeMapUpdate(stripe, &stripeMap, &segmentCtxUpdater);
    delete stripeMapUpdateInHeap;
}

TEST(StripeMapUpdate, _DoSpecificJob_testIfMetadataUpdatedSuccessfully)
{
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockSegmentContextUpdater> segmentCtxUpdater;

    StripeMapUpdate stripeMapUpdate(StripeSmartPtr(stripe), &stripeMap, &segmentCtxUpdater);

    StripeId vsid = 215;
    StripeId userLsid = 215;

    ON_CALL(*stripe, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(*stripe, GetUserLsid).WillByDefault(Return(userLsid));

    EXPECT_CALL(stripeMap, SetLSA(vsid, userLsid, IN_USER_AREA)).Times(1);

    bool result = stripeMapUpdate.Execute();
    EXPECT_EQ(result, true);
}

} // namespace pos
