#include "src/io/general_io/array_unlocking.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array/service/io_locker/i_io_locker_mock.h"

using ::testing::_;
namespace pos
{
TEST(ArrayUnlocking, ArrayUnlocking_Constructor)
{
    // Given : User Data and simple partition type
    std::set<IArrayDevice*> devs;
    StripeId stripeId = 0;
    MockIIOLocker mockIIOLocker;
    // When : Constructor
    ArrayUnlocking arrayUnlocking(devs, stripeId, &mockIIOLocker, 0);
}

TEST(ArrayUnlocking, ArrayUnlocking_DoSpecificJob)
{
    // Given : User Data and simple partition type
    std::set<IArrayDevice*> devs;
    StripeId stripeId = 0;
    MockIIOLocker mockIIOLocker;
    // Then : Execute and check result
    ArrayUnlocking arrayUnlocking(devs, stripeId, &mockIIOLocker, 0);
    Callback* callback = &arrayUnlocking;
    EXPECT_CALL(mockIIOLocker, Unlock(devs, _));
    bool actual = callback->Execute();
    ASSERT_EQ(actual, true);
}

} // namespace pos
