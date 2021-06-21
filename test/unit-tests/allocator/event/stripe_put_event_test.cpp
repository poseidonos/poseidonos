#include "src/allocator/event/stripe_put_event.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(StripePutEvent, StripePutEvent_)
{
}

TEST(StripePutEvent, Execute_TestIfOkToFree)
{
    // given
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    NiceMock<MockIWBStripeAllocator>* wbAllocator = new NiceMock<MockIWBStripeAllocator>();
    StripePutEvent stripePutEvent(wbAllocator, *stripe, 5);

    // given 1.
    EXPECT_CALL(*stripe, IsOkToFree).WillOnce(Return(true));
    EXPECT_CALL(*wbAllocator, FreeWBStripeId(5));
    // when 1.
    bool ret = stripePutEvent.Execute();
    // then 1.
    EXPECT_EQ(true, ret);

    // given 2.
    EXPECT_CALL(*stripe, IsOkToFree).WillOnce(Return(false));
    // when 2.
    ret = stripePutEvent.Execute();
    // then 2.
    EXPECT_EQ(false, ret);

    delete wbAllocator;
    delete stripe;
}

} // namespace pos
