#include "src/array/service/io_locker/stripe_locker_busy_state.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
namespace pos
{
TEST(StripeLockerBusyState, StripeLockerBusyState_testIfTryLockSuccess)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;

    // When
    bool ret = slbs.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerBusyState, StripeLockerBusyState_testIfTryLockFail)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.TryLock(StripeLockInfo(sid));
    // When
    bool ret = slbs.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_FALSE(ret);
}


TEST(StripeLockerBusyState, StripeLockerBusyState_testIfTryLockAgainAfterUnlock)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.TryLock(StripeLockInfo(sid));
    slbs.Unlock(sid);
    // When
    bool ret = slbs.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerBusyState, StripeLockerBusyState_testIfTryLockAfterUnlockNotExist)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.Unlock(sid);

    // When
    bool ret = slbs.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerBusyState, StripeLockerBusyState_TestForExistsItem)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.TryLock(StripeLockInfo(sid));

    // When
    bool ret = slbs.Exists(sid);

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerBusyState, StripeLockerBusyState_TestForNotExistItem)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;

    // When
    bool ret = slbs.Exists(sid);

    // Then
    ASSERT_FALSE(ret);
}

TEST(StripeLockerBusyState, StripeLockerBusyState_TestExistsAfterUnlock)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.TryLock(StripeLockInfo(sid));
    slbs.Unlock(sid);

    // When
    bool ret = slbs.Exists(sid);

    // Then
    ASSERT_FALSE(ret);
}


TEST(StripeLockerBusyState, StripeLockerBusyState_Count)
{
    // Given
    StripeLockerBusyState slbs;
    StripeId sid = 10;
    slbs.TryLock(StripeLockInfo(10));
    slbs.TryLock(StripeLockInfo(10));
    slbs.TryLock(StripeLockInfo(20));
    slbs.TryLock(StripeLockInfo(20));
    slbs.TryLock(StripeLockInfo(30));
    slbs.TryLock(StripeLockInfo(30));

    // When
    uint32_t ret = slbs.Count();

    // Then
    ASSERT_EQ(3, ret);
}
} // namespace pos

