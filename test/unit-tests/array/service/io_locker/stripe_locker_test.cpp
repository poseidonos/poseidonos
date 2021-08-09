#include "src/array/service/io_locker/stripe_locker.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::Return;
namespace pos
{
TEST(StripeLocker, StripeLocker_testIfTryLockInNormal)
{
    // Given
    StripeLocker sl;
    StripeId sid = 10;

    // When
    bool ret = sl.TryLock(sid);

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLocker, StripeLocker_testIfTryLockInBusyButOutOfBusyRange)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);
    // When
    StripeId sid = 10;
    bool ret = sl.TryLock(sid);

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLocker, StripeLocker_testIfTryLockInBusyRange)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);
    // When
    StripeId sid = 150;
    bool ret = sl.TryLock(sid);

    // Then
    ASSERT_FALSE(ret);
}

TEST(StripeLocker, StripeLocker_testIfTryLockAfterResetBusyRange)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);
    for (StripeId i = busyRangeLower; i <= busyRangeUpper; i++)
    {
        sl.Unlock(i);
    }
    sl.ResetBusyLock();

    // When
    StripeId sid = 150;
    bool ret = sl.TryLock(sid);

    // Then
    ASSERT_TRUE(ret);
}

TEST(StripeLocker, StripeLocker_testIfRefuseResetBusyRangeDueToRemainings)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);

    // When
    bool ret = sl.ResetBusyLock();

    // Then
    ASSERT_FALSE(ret);
}

TEST(StripeLocker, StripeLocker_testIfRefuseToTryBusyLockDueToRemainings)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    StripeId sidInBusyRange = 150;
    sl.TryLock(sidInBusyRange);

    // When
    bool ret = sl.TryBusyLock(busyRangeLower, busyRangeUpper);

    // Then
    ASSERT_FALSE(ret);
}

TEST(StripeLocker, StripeLocker_testIfUnlockInNormal)
{
    // Given
    StripeLocker sl;
    StripeId sid = 10;

    // When
    sl.TryLock(sid);

    // Then
    sl.Unlock(sid);
}

TEST(StripeLocker, StripeLocker_testIfUnlockInBusyButOutOfBusyRange)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);

    // When
    StripeId sid = 10;
    sl.TryLock(sid);

    // Then
    sl.Unlock(sid);
}

TEST(StripeLocker, StripeLocker_testIfUnlockInBusyRange)
{
    // Given
    StripeLocker sl;
    StripeId busyRangeLower = 100;
    StripeId busyRangeUpper = 200;
    sl.TryBusyLock(busyRangeLower, busyRangeUpper);

    // When
    StripeId sid = 150;

    // Then
    sl.Unlock(sid);
}

} // namespace pos


