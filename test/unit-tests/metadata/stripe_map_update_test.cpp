#include "src/metadata/stripe_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
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
    NiceMock<MockIContextManager> contextManager;

    StripeMapUpdate stripeMapUpdate(&stripe, &stripeMap, &contextManager);
}

TEST(StripeMapUpdate, _DoSpecificJob_testIfMetadataUpdatedSuccessfully)
{
    NiceMock<MockStripe> stripe;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIContextManager> contextManager;

    StripeMapUpdate stripeMapUpdate(&stripe, &stripeMap, &contextManager);

    StripeId vsid = 215;
    StripeId userLsid = 215;

    ON_CALL(stripe, GetVsid).WillByDefault(Return(vsid));
    ON_CALL(stripe, GetUserLsid).WillByDefault(Return(userLsid));

    EXPECT_CALL(stripeMap, SetLSA(vsid, userLsid, IN_USER_AREA)).Times(1);
    EXPECT_CALL(contextManager, UpdateOccupiedStripeCount(userLsid)).Times(1);

    bool result = stripeMapUpdate.Execute();
    EXPECT_EQ(result, true);
}

} // namespace pos
