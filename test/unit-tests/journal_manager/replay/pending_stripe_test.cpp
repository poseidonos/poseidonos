#include "src/journal_manager/replay/pending_stripe.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(PendingStripe, PendingStripe_testIfConstructedSuccessfully)
{
    // Given
    int volId = 5;
    StripeId lsid = 100;
    VirtualBlkAddr tail = {
        .stripeId = 10,
        .offset = 3};

    // When
    PendingStripe stripe(volId, lsid, tail);

    // Then
    EXPECT_EQ(stripe.volumeId, volId);
    EXPECT_EQ(stripe.wbLsid, lsid);
    EXPECT_EQ(stripe.tailVsa, tail);
}

} // namespace pos
