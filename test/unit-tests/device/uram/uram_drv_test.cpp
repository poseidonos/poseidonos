#include "src/device/uram/uram_drv.h"

#include <gtest/gtest.h>

#include "src/device/uram/uram_device_context.h"
#include "test/unit-tests/device/uram/uram_io_context_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_bdev_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "lib/spdk/lib/thread/thread_internal.h"

using testing::_;
using testing::NiceMock;
using testing::Return;
using namespace pos;

TEST(UramDrv, UramDrvDestructor_testDestructUramDrvIfspdkCallerIsNullptr)
{
    // Given
    SpdkBdevCaller* bdevCaller = nullptr;
    SpdkThreadCaller* threadCaller = nullptr;
    UramDrv* uramDrv = new UramDrv(bdevCaller, threadCaller);

    // When
    delete uramDrv;
    delete threadCaller;

    // Then
    EXPECT_EQ(bdevCaller, nullptr);
    EXPECT_EQ(threadCaller, nullptr);
}

TEST(UramDrv, ScanDevs_testIfNoUramOnlyOtherDevicesExist)
{
    // Given
    NiceMock<MockSpdkBdevCaller>* bdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    struct spdk_bdev bdev1, bdev2;
    bdev1.product_name = "NotUram1";
    bdev2.product_name = "NotUram2";

    EXPECT_CALL(*bdevCaller, SpdkBdevFirst).WillOnce(Return(&bdev1));
    int callCount = 0;
    EXPECT_CALL(*bdevCaller, SpdkBdevNext)
        .WillOnce(Return(&bdev2))
        .WillOnce(Return(nullptr));
    UramDrv uramDrv(bdevCaller);

    // When
    int addedDeviceCount = uramDrv.ScanDevs(nullptr);

    // Then
    EXPECT_EQ(0, addedDeviceCount);
}

TEST(UramDrv, Open_testIfDeviceContextIsNull)
{
    // Given
    UramDrv uramDrv;

    // When
    bool ret = uramDrv.ScanDevs(nullptr);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Open_testIfDeviceIsAlreadyOpened)
{
    // Given
    UramDrv uramDrv;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = true;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Open_testIfCurrentThreadIsNotReactor)
{
    // Given
    NiceMock<MockEventFrameworkApi> eventFrameworkApi;
    EXPECT_CALL(eventFrameworkApi, IsReactorNow).WillOnce(Return(false));

    UramDrv uramDrv(nullptr, nullptr, &eventFrameworkApi);
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_TRUE(ret);
}

TEST(UramDrv, Open_testIfBdevInDevCtxIsNullPtr)
{
    // Given
    NiceMock<MockEventFrameworkApi> eventFrameworkApi;
    EXPECT_CALL(eventFrameworkApi, IsReactorNow).WillOnce(Return(true));

    NiceMock<MockSpdkBdevCaller>* mockSpdkBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevGetByName)
        .WillOnce(Return(nullptr));

    UramDrv uramDrv(mockSpdkBdevCaller, nullptr, &eventFrameworkApi);
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Open_testIfBdevOpenExtInSpdkIsFailed)
{
    // Given
    NiceMock<MockEventFrameworkApi> eventFrameworkApi;
    EXPECT_CALL(eventFrameworkApi, IsReactorNow).WillOnce(Return(true));

    struct spdk_bdev bdev;
    NiceMock<MockSpdkBdevCaller>* mockSpdkBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevGetByName)
        .WillOnce(Return(&bdev));
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevOpenExt).WillOnce(Return(-1));

    UramDrv uramDrv(mockSpdkBdevCaller, nullptr, &eventFrameworkApi);
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Open_testIfGetIoChannelFailed)
{
    // Given
    NiceMock<MockEventFrameworkApi> eventFrameworkApi;
    EXPECT_CALL(eventFrameworkApi, IsReactorNow).WillOnce(Return(true));

    struct spdk_bdev bdev;
    NiceMock<MockSpdkBdevCaller>* mockSpdkBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevGetByName)
        .WillOnce(Return(&bdev));
    int bdev_desc = 1;
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevOpenExt)
        .WillOnce([&bdev_desc](
                      const char* bdev_name,
                      bool write,
                      spdk_bdev_event_cb_t event_cb,
                      void* event_ctx,
                      struct spdk_bdev_desc** desc) -> int {
            *desc = reinterpret_cast<struct spdk_bdev_desc*>(&bdev_desc);
            return 0;
        });
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevGetIoChannel)
        .WillOnce(Return(nullptr));

    UramDrv uramDrv(mockSpdkBdevCaller, nullptr, &eventFrameworkApi);
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_EQ(devCtx.bdev_desc, nullptr);
    EXPECT_TRUE(ret);
}

TEST(UramDrv, Close_testIfDeviceContextIsNullptr)
{
    // Given
    UramDrv uramDrv;

    // When
    bool ret = uramDrv.Close(nullptr);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Close_testIfDeviceAlreadyClosed)
{
    // Given
    UramDrv uramDrv;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Close(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UramDrv, Close_testIfBdevIoChannelAndBdevDescAreNullptr)
{
    // Given
    UramDrv uramDrv;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = true;
    devCtx.bdev_io_channel = nullptr;
    devCtx.bdev_desc = nullptr;

    // When
    bool ret = uramDrv.Close(&devCtx);

    // Then
    EXPECT_TRUE(ret);
}

TEST(UramDrv, SubmitIO_testIfSubmssionAbortDirToUramAndIoChannelIsNullptr)
{
    // Given
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.bdev_desc = nullptr;
    devCtx.bdev_io_channel = nullptr;
    NiceMock<MockUramIOContext>* mockIoCtx = new NiceMock<MockUramIOContext>();
    EXPECT_CALL(*mockIoCtx, GetDeviceContext)
        .Times(3)
        .WillRepeatedly(Return(&devCtx));
    EXPECT_CALL(*mockIoCtx, GetOpcode).WillOnce(Return(UbioDir::Abort));
    EXPECT_CALL(*mockIoCtx, GetStartByteOffset).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetByteCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetBuffer).WillOnce(nullptr);
    NiceMock<MockSpdkBdevCaller>* mockBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockBdevCaller, SpdkBdevGetIoChannel)
        .WillOnce(Return(nullptr));

    UramDrv uramDrv(mockBdevCaller);

    // When
    int ret = uramDrv.SubmitIO(mockIoCtx);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UramDrv, SubmitIO_testIfReadSubmissionFailedWithEnomemError)
{
    // Given
    struct spdk_io_channel channel;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.bdev_desc = nullptr;
    devCtx.bdev_io_channel = &channel;
    NiceMock<MockUramIOContext>* mockIoCtx = new NiceMock<MockUramIOContext>();
    EXPECT_CALL(*mockIoCtx, GetDeviceContext).WillOnce(Return(&devCtx));
    EXPECT_CALL(*mockIoCtx, GetOpcode).WillOnce(Return(UbioDir::Read));
    EXPECT_CALL(*mockIoCtx, GetStartByteOffset).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetByteCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetBuffer).WillOnce(nullptr);
    EXPECT_CALL(*mockIoCtx, GetRetryCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, RequestRetry).WillOnce(Return(true));
    NiceMock<MockSpdkBdevCaller>* mockBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockBdevCaller, SpdkBdevRead).WillOnce(Return(-ENOMEM));

    UramDrv uramDrv(mockBdevCaller);

    // When
    int ret = uramDrv.SubmitIO(mockIoCtx);

    // Then
    EXPECT_EQ(ret, 0);

    // Teardown
    delete mockIoCtx;
}

TEST(UramDrv, SubmitIO_testIfRequestRetryFailed)
{
    // Given
    struct spdk_io_channel channel;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.bdev_desc = nullptr;
    devCtx.bdev_io_channel = &channel;
    NiceMock<MockUramIOContext>* mockIoCtx = new NiceMock<MockUramIOContext>();
    EXPECT_CALL(*mockIoCtx, GetDeviceContext)
        .Times(3)
        .WillRepeatedly(Return(&devCtx));
    EXPECT_CALL(*mockIoCtx, GetOpcode).WillOnce(Return(UbioDir::Read));
    EXPECT_CALL(*mockIoCtx, GetStartByteOffset).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetByteCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetBuffer).WillOnce(nullptr);
    EXPECT_CALL(*mockIoCtx, GetRetryCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, RequestRetry).WillOnce(Return(false));
    NiceMock<MockSpdkBdevCaller>* mockBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockBdevCaller, SpdkBdevRead).WillOnce(Return(-ENOMEM));

    UramDrv uramDrv(mockBdevCaller);

    // When
    int ret = uramDrv.SubmitIO(mockIoCtx);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UramDrv, SubmitIO_testIfRetryCountIsExceeded)
{
    // Given
    struct spdk_io_channel channel;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.bdev_desc = nullptr;
    devCtx.bdev_io_channel = &channel;
    NiceMock<MockUramIOContext>* mockIoCtx = new NiceMock<MockUramIOContext>();
    EXPECT_CALL(*mockIoCtx, GetDeviceContext)
        .Times(3)
        .WillRepeatedly(Return(&devCtx));
    EXPECT_CALL(*mockIoCtx, GetOpcode).WillOnce(Return(UbioDir::Read));
    EXPECT_CALL(*mockIoCtx, GetStartByteOffset).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetByteCount).WillOnce(Return(0));
    EXPECT_CALL(*mockIoCtx, GetBuffer).WillOnce(nullptr);
    EXPECT_CALL(*mockIoCtx, GetRetryCount).WillOnce(Return(1));
    NiceMock<MockSpdkBdevCaller>* mockBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockBdevCaller, SpdkBdevRead).WillOnce(Return(-ENOMEM));

    UramDrv uramDrv(mockBdevCaller);

    // When
    int ret = uramDrv.SubmitIO(mockIoCtx);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UramDrv, CompleteError_testIfCompleteErrorCalledProperly)
{
    // Given
    UramDrv uramDrv;

    // When
    int ret = uramDrv.CompleteErrors(nullptr);

    // Then
    EXPECT_EQ(0, ret);
}

TEST(UramDrv, SubmitIO_testIfRetryCallbackTriggerdProperly)
{
    // Given
    struct spdk_io_channel channel;
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.bdev_desc = nullptr;
    devCtx.bdev_io_channel = &channel;
    //NiceMock<MockUramIOContext>* mockIoCtx = new NiceMock<MockUramIOContext>();
    NiceMock<MockUramIOContext> mockIoCtx;
    EXPECT_CALL(mockIoCtx, GetDeviceContext).WillOnce(Return(&devCtx));
    EXPECT_CALL(mockIoCtx, GetOpcode).WillOnce(Return(UbioDir::Read));
    EXPECT_CALL(mockIoCtx, GetStartByteOffset).WillOnce(Return(0));
    EXPECT_CALL(mockIoCtx, GetByteCount).WillOnce(Return(0));
    EXPECT_CALL(mockIoCtx, GetBuffer).WillOnce(nullptr);
    EXPECT_CALL(mockIoCtx, GetRetryCount).WillOnce(Return(0));
    EXPECT_CALL(mockIoCtx, RequestRetry).WillOnce([](spdk_bdev_io_wait_cb callbackFunc) {
        callbackFunc(nullptr);
        return true;
    });
    NiceMock<MockSpdkBdevCaller>* mockBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockBdevCaller, SpdkBdevRead)
        .WillOnce(Return(-ENOMEM));

    UramDrv uramDrv(mockBdevCaller);

    // When
    int ret = uramDrv.SubmitIO(&mockIoCtx);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UramDrv, Open_testIfSpdkBdevOpenExtFailedAndCallbackTriggeredProperly)
{
    // Given
    NiceMock<MockEventFrameworkApi> eventFrameworkApi;
    EXPECT_CALL(eventFrameworkApi, IsReactorNow).WillOnce(Return(true));

    struct spdk_bdev bdev;
    NiceMock<MockSpdkBdevCaller>* mockSpdkBdevCaller =
        new NiceMock<MockSpdkBdevCaller>();
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevGetByName)
        .WillOnce(Return(&bdev));
    EXPECT_CALL(*mockSpdkBdevCaller, SpdkBdevOpenExt).WillOnce([](const char* bdev_name, bool write, spdk_bdev_event_cb_t event_cb, void* event_ctx, struct spdk_bdev_desc** desc) -> int {
        event_cb(spdk_bdev_event_type::SPDK_BDEV_EVENT_REMOVE,
            nullptr, nullptr);
        return -1;
    });

    UramDrv uramDrv(mockSpdkBdevCaller, nullptr, &eventFrameworkApi);
    UramDeviceContext devCtx("UtDevCtx");
    devCtx.opened = false;

    // When
    bool ret = uramDrv.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}