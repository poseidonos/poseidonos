#include "src/io/general_io/array_unlocking.h"
#include "test/unit-tests/array/service/io_locker/i_io_locker_mock.h"

#include <gtest/gtest.h>

using ::testing::_;
namespace pos
{
TEST(ArrayUnlocking, ArrayUnlocking_Constructor)
{
    PartitionType partitionType = PartitionType::USER_DATA;
    StripeId stripeId = 0;
    const std::string str = "array";
    MockIIOLocker mockIIOLocker;
    ArrayUnlocking arrayUnlocking(partitionType, stripeId, str, &mockIIOLocker);
}

TEST(ArrayUnlocking, ArrayUnlocking_DoSpecificJob)
{
    PartitionType partitionType = PartitionType::USER_DATA;
    StripeId stripeId = 0;
    const std::string str = "array";
    MockIIOLocker mockIIOLocker;
    ArrayUnlocking arrayUnlocking(partitionType, stripeId, str, &mockIIOLocker);
    Callback *callback = &arrayUnlocking;
    EXPECT_CALL(mockIIOLocker, Unlock(_, _));
    bool actual = callback->Execute();
    ASSERT_EQ(actual, true);

}

} // namespace pos
