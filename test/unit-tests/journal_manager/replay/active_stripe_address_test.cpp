#include "src/journal_manager/replay/active_stripe_address.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ActiveStripeAddr, Reset_testIfALlReseted)
{
    // Given
    int volId = 0;
    VirtualBlkAddr tail = {
        .stripeId = 100,
        .offset = 0};
    StripeId wbLsid = 4;
    ActiveStripeAddr addr(volId, tail, wbLsid);

    // When
    addr.Reset();

    // Then
    EXPECT_EQ(addr.GetTail(), UNMAP_VSA);
    EXPECT_EQ(addr.GetWbLsid(), UNMAP_STRIPE);
    EXPECT_EQ(addr.GetVolumeId(), INT32_MAX);
}

TEST(ActiveStripeAddr, UpdateWbLsid_testIfWbLsidUpdated)
{
    // Given
    ActiveStripeAddr addr;

    // When
    StripeId wbLsid = 5;
    addr.UpdateWbLsid(wbLsid);

    // Then
    EXPECT_EQ(addr.GetWbLsid(), wbLsid);
}

TEST(ActiveStripeAddr, UpdateTail_testIfTailUpdated)
{
    // Given
    ActiveStripeAddr addr;

    // When
    VirtualBlkAddr tail = {
        .stripeId = 200,
        .offset = 120};
    addr.UpdateTail(tail);

    // Then
    EXPECT_EQ(addr.GetTail(), tail);
}

TEST(ActiveStripeAddr, IsValid_testWhenTailIsValid)
{
    // Given
    VirtualBlkAddr tail = {
        .stripeId = 100,
        .offset = 5};
    ActiveStripeAddr addr(0, tail, 0);

    // When, Then
    EXPECT_TRUE(addr.IsValid() == true);
}

TEST(ActiveStripeAddr, IsValid_testWhenTailIsNotValid)
{
    // Given
    VirtualBlkAddr tail = UNMAP_VSA;
    ActiveStripeAddr addr(0, tail, 0);

    // When, Then
    EXPECT_TRUE(addr.IsValid() == false);
}

} // namespace pos
