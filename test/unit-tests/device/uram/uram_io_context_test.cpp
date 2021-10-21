#include "src/device/uram/uram_io_context.h"

#include <gtest/gtest.h>

#include "test/unit-tests/spdk_wrapper/caller/spdk_bdev_caller_mock.h"
#include "src/device/uram/uram_device_context.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(UramIOContext, Add_GetRetryCount_testIfRetryCountAddedProperly)
{
    // Given
    UramIOContext uramIoContext(nullptr, nullptr, 0);

    // When
    uramIoContext.AddRetryCount();
    uint32_t retryCount = uramIoContext.GetRetryCount();

    // Then
    EXPECT_EQ(retryCount, 1);
}

TEST(UramIOContext, RequestRetry_testRequestRetryProperly)
{
    // Given
    MockSpdkBdevCaller* mockCaller = new MockSpdkBdevCaller();
    EXPECT_CALL(*mockCaller, SpdkBdevQueueIoWait).WillOnce(Return(0));
    UramDeviceContext devCtx("");
    UramIOContext uramIoContext(&devCtx, nullptr, 0, mockCaller);

    // When
    bool ret = uramIoContext.RequestRetry(nullptr);

    // Then
    EXPECT_EQ(ret, true);
}

} // namespace pos
