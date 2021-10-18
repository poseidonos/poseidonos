#include "src/device/base/device_context.h"

#include <gtest/gtest.h>

#include "test/unit-tests/device/base/io_context_mock.h"
#include "test/unit-tests/lib/system_timeout_checker_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

using namespace pos;

TEST(DeviceContext, DeviceContextDestructor_testIfDesturctedProperly)
{
    // Given
    DeviceContext* devCtx = new DeviceContext();

    // When
    delete devCtx;
}

TEST(DeviceContext, DeviceContextDestructor_testIfTimeoutCheckerIsNullptr)
{
    // Given
    SystemTimeoutChecker* timeoutChecker = nullptr;
    DeviceContext* devCtx = new DeviceContext(timeoutChecker);

    // When
    delete devCtx;
}

TEST(DeviceContext, AddPendingError_testIfTimeoutIsNotReached)
{
    // Given
    NiceMock<MockIOContext> mockIoCtx;
    EXPECT_CALL(mockIoCtx, SetErrorKey).Times(1);
    EXPECT_CALL(mockIoCtx, AddPendingErrorCount).Times(1);
    NiceMock<MockSystemTimeoutChecker>* mockChecker
        = new NiceMock<MockSystemTimeoutChecker>();
    EXPECT_CALL(*mockChecker, CheckTimeout).WillOnce(Return(false));

    DeviceContext devCtx(mockChecker);

    // When
    devCtx.AddPendingError(mockIoCtx);

    // Then
    uint32_t pendingErrorCount = devCtx.GetPendingErrorCount();
    EXPECT_EQ(1, pendingErrorCount);
}

TEST(DeviceContext, AddPendingError_testIfTimeoutReachedAndErrorIsNotAvailable)
{
    // Given
    NiceMock<MockIOContext> mockIoCtx;
    EXPECT_CALL(mockIoCtx, SetErrorKey).Times(1);
    EXPECT_CALL(mockIoCtx, AddPendingErrorCount).Times(1);
    list<IOContext*>::iterator iter;
    auto errorKey = make_pair(iter, false);
    EXPECT_CALL(mockIoCtx, GetErrorKey).WillOnce(Return(errorKey));
    NiceMock<MockSystemTimeoutChecker>* mockChecker
        = new NiceMock<MockSystemTimeoutChecker>();
    EXPECT_CALL(*mockChecker, CheckTimeout).WillOnce(Return(true));

    DeviceContext devCtx(mockChecker);

    // When
    devCtx.AddPendingError(mockIoCtx);

    // Then
    uint32_t pendingErrorCount = devCtx.GetPendingErrorCount();
    EXPECT_EQ(1, pendingErrorCount);
}

TEST(DeviceContext, GetPendingError_testIfPendingErrorListIsEmpty)
{
    // Given
    DeviceContext devCtx;

    // When
    IOContext* ret = devCtx.GetPendingError();

    // Then
    EXPECT_EQ(nullptr, ret);
}

TEST(DeviceContext, GetPendingError_testIfTimeoutReachedAndErrorIsNotReady)
{
    // Given
    NiceMock<MockSystemTimeoutChecker>* mockChecker
        = new NiceMock<MockSystemTimeoutChecker>();
    EXPECT_CALL(*mockChecker, CheckTimeout)
    .WillOnce(Return(false))
    .WillOnce(Return(true));
    DeviceContext devCtx(mockChecker);
    NiceMock<MockIOContext> mockIoCtx;
    list<IOContext*>::iterator iter;
    auto errorKey = make_pair(iter, false);
    EXPECT_CALL(mockIoCtx, GetErrorKey).WillOnce(Return(errorKey));

    devCtx.AddPendingError(mockIoCtx);

    // When
    IOContext* ret = devCtx.GetPendingError();

    // Then
    EXPECT_EQ(nullptr, ret);
}

TEST(DeviceContext, RemovePendingError_testIfErrorIsNotAvailable)
{
    // Given
    NiceMock<MockSystemTimeoutChecker>* mockChecker
        = new NiceMock<MockSystemTimeoutChecker>();
    EXPECT_CALL(*mockChecker, CheckTimeout).WillOnce(Return(false));
    DeviceContext devCtx(mockChecker);
    NiceMock<MockIOContext> mockIoCtx;
    list<IOContext*>::iterator iter;
    auto errorKey = make_pair(iter, false);
    EXPECT_CALL(mockIoCtx, GetErrorKey).WillOnce(Return(errorKey));

    devCtx.AddPendingError(mockIoCtx);

    // When
    devCtx.RemovePendingError(mockIoCtx);

    // Then
    EXPECT_EQ(1, devCtx.GetPendingErrorCount());
}

TEST(DeviceContext, RemovePendingError_testIfPendingErrorRemovedProperly)
{
    // Given
    NiceMock<MockSystemTimeoutChecker>* mockChecker
        = new NiceMock<MockSystemTimeoutChecker>();
    EXPECT_CALL(*mockChecker, CheckTimeout).WillOnce(Return(false));
    DeviceContext devCtx(mockChecker);
    NiceMock<MockIOContext> mockIoCtx;
    list<IOContext*>::iterator iter;
    EXPECT_CALL(mockIoCtx, SetErrorKey).WillOnce(
        [&iter](list<IOContext*>::iterator it)
        {
            iter = it;
        });
    EXPECT_CALL(mockIoCtx, GetErrorKey).WillOnce(
        [&iter]()
        {
            return make_pair(iter, true);
        });
    devCtx.AddPendingError(mockIoCtx);

    // When
    devCtx.RemovePendingError(mockIoCtx);

    // Then
    EXPECT_EQ(0, devCtx.GetPendingErrorCount());
}
