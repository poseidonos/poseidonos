#include "src/array/service/io_locker/stripe_locker_normal_state.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
namespace pos
{
TEST(StripeLockerNormalState, StripeLockerNormalState_testIfTryLockSuccess)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;

    // When
    bool ret = slns.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_testIfTryLockTwiceSuccess)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.TryLock(StripeLockInfo(sid));

    // When
    bool ret = slns.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_testIfTryLockAgainAfterUnlock)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.TryLock(StripeLockInfo(sid));
    slns.Unlock(sid);
    // When
    bool ret = slns.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_testIfTryLockAfterUnlockNotExist)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.Unlock(sid);

    // When
    bool ret = slns.TryLock(StripeLockInfo(sid));

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_TestForExistsItem)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.TryLock(StripeLockInfo(sid));

    // When
    bool ret = slns.Exists(sid);

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_TestForNotExistItem)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;

    // When
    bool ret = slns.Exists(sid);

    // Then
    ASSERT_FALSE(ret);
}

TEST(StripeLockerNormalState, StripeLockerNormalState_TestExistsAfterUnlock)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.TryLock(StripeLockInfo(10));
    slns.Unlock(10);

    // When
    bool ret = slns.Exists(sid);

    // Then
    ASSERT_FALSE(ret);
}


TEST(StripeLockerNormalState, StripeLockerNormalState_Count)
{
    // Given
    StripeLockerNormalState slns;
    StripeId sid = 10;
    slns.TryLock(StripeLockInfo(10));
    slns.TryLock(StripeLockInfo(10));
    slns.TryLock(StripeLockInfo(20));
    slns.TryLock(StripeLockInfo(20));
    slns.TryLock(StripeLockInfo(30));
    slns.TryLock(StripeLockInfo(30));

    // When
    uint32_t ret = slns.Count();

    // Then
    ASSERT_EQ(6, ret);
}
} // namespace pos

