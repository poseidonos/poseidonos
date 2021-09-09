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

TEST(ActiveStripeAddr, UpdateWbLsid_testIfItFailsWhenWbLsidUpdatedTwice)
{
    // Given
    ActiveStripeAddr addr;

    // When 1
    StripeId wbLsid = 5;
    addr.UpdateWbLsid(wbLsid);

    // Then 1
    EXPECT_EQ(addr.GetWbLsid(), wbLsid);

    // When 2
    addr.UpdateWbLsid(wbLsid);

    // Then 2: Nothing

    // When/Then 3
    ASSERT_DEATH(
        {
            addr.UpdateWbLsid(100);
        },
        "");
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

TEST(ActiveStripeAddr, UpdateTail_testLastTailOffsetIsUpdated)
{
    // Given
    ActiveStripeAddr addr;

    VirtualBlkAddr tail = {
        .stripeId = 200,
        .offset = 0};

    // When
    tail.offset = 20;
    addr.UpdateTail(tail);

    tail.offset = 50;
    addr.UpdateTail(tail);

    tail.offset = 0;
    addr.UpdateTail(tail);

    VirtualBlkAddr expected = {
        .stripeId = tail.stripeId,
        .offset = 50};

    EXPECT_EQ(addr.GetTail(), expected);
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
