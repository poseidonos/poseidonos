#include "src/metadata/stripe_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(StripeMapUpdate, StripeMapUpdate_testIfConstructedSuccessfully)
{
    NiceMock<MockStripe> stripe;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;

    StripeMapUpdate stripeMapUpdate(&stripe, &stripeMap, &segmentCtx);

    StripeMapUpdate* stripeMapUpdateInHeap = new StripeMapUpdate(&stripe, &stripeMap, &segmentCtx);
    delete stripeMapUpdateInHeap;
}

TEST(StripeMapUpdate, _DoSpecificJob_testIfMetadataUpdatedSuccessfully)
{
    NiceMock<MockStripe> stripe;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;

    StripeMapUpdate stripeMapUpdate(&stripe, &stripeMap, &segmentCtx);

    StripeId vsid = 215;
    StripeId userLsid = 215;

    ON_CALL(stripe, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(stripe, GetUserLsid).WillByDefault(Return(userLsid));

    EXPECT_CALL(stripeMap, SetLSA(vsid, userLsid, IN_USER_AREA)).Times(1);

    bool result = stripeMapUpdate.Execute();
    EXPECT_EQ(result, true);
}

} // namespace pos
