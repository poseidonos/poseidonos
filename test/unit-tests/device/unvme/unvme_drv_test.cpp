/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/device/unvme/unvme_drv.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/bio/ubio_mock.h"
#include "test/unit-tests/device/unvme/unvme_cmd_mock.h"
#include "test/unit-tests/device/unvme/unvme_device_context_mock.h"
#include "test/unit-tests/device/unvme/unvme_io_context_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_nvme_caller_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(UnvmeDrv,
    CompleteErrors_testAdminCommandFailedWithOutOfMemoryAndExceedRetryLimit)
{
    // Given
    NiceMock<MockUnvmeCmd>* mockUnvmeCmd = new NiceMock<MockUnvmeCmd>();
    EXPECT_CALL(*mockUnvmeCmd, RequestIO).WillOnce(Return(-ENOMEM));
    NiceMock<MockSpdkNvmeCaller>* mockSpdkNvmeCaller =
        new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockSpdkNvmeCaller, SpdkNvmeCtrlrProcessAdminCompletions)
        .WillOnce(Return(0));
    EXPECT_CALL(*mockSpdkNvmeCaller, SpdkNvmeQpairProcessCompletions)
        .WillOnce(Return(0));
    NiceMock<MockUnvmeIOContext> mockIoCtx;
    EXPECT_CALL(mockIoCtx, GetOutOfMemoryRetryCount)
        .WillRepeatedly(Return(UNVME_DRV_OUT_OF_MEMORY_RETRY_LIMIT));
    EXPECT_CALL(mockIoCtx, IsAdminCommand).WillOnce(Return(true));
    NiceMock<MockUnvmeDeviceContext> mockDevCtx;
    mockDevCtx.ioCompletionCount = 0;
    EXPECT_CALL(mockDevCtx, GetPendingError).WillOnce(Return(&mockIoCtx));

    EXPECT_CALL(mockIoCtx, GetDeviceContext)
        .Times(3)
        .WillRepeatedly(Return(&mockDevCtx));

    UnvmeDrv unvmeDrv(mockUnvmeCmd, mockSpdkNvmeCaller);

    // When
    uint32_t ret = unvmeDrv.CompleteErrors(&mockDevCtx);

    // Then
    EXPECT_EQ(ret, mockDevCtx.ioCompletionCount);
}

TEST(UnvmeDrv, CompleteErrors_testIfIoCtxIsnull)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevCtx;
    mockDevCtx.ioCompletionCount = 0;
    EXPECT_CALL(mockDevCtx, GetPendingError).WillOnce(Return(nullptr));
    UnvmeDrv unvmeDrv;

    // When
    int ret = unvmeDrv.CompleteErrors(&mockDevCtx);

    // Then
    EXPECT_EQ(ret, mockDevCtx.ioCompletionCount);
}

TEST(UnvmeDrv, DeviceAttached_testIfAttachEventIsNull)
{
    // Given
    int expectedEventId = EID(UNVME_SSD_ATTACH_NOTIFICATION_FAILED);
    UnvmeDrv unvmeDrv;

    // When
    int ret = unvmeDrv.DeviceAttached(nullptr, 0, nullptr);

    // Then
    EXPECT_EQ(ret, expectedEventId);
}

TEST(UnvmeDrv, DeviceDetached_testIfDetachEventIsNull)
{
    // Given
    int expectedEventId = EID(UNVME_SSD_DETACH_NOTIFICATION_FAILED);
    UnvmeDrv unvmeDrv;

    // When
    int ret = unvmeDrv.DeviceDetached("");

    // Then
    EXPECT_EQ(ret, expectedEventId);
}

TEST(UnvmeDrv, CompleteErrors_testIfRetryCountIsNotZero)
{
    // Given
    NiceMock<MockUnvmeCmd>* mockUnvmeCmd = new NiceMock<MockUnvmeCmd>();
    EXPECT_CALL(*mockUnvmeCmd, RequestIO).WillOnce(Return(0));
    NiceMock<MockSpdkNvmeCaller>* mockSpdkNvmeCaller =
        new NiceMock<MockSpdkNvmeCaller>();
    NiceMock<MockUnvmeIOContext> mockIoCtx;
    EXPECT_CALL(mockIoCtx, GetOutOfMemoryRetryCount).WillOnce(Return(0));
    EXPECT_CALL(mockIoCtx, CheckAndDecreaseErrorRetryCount)
        .WillOnce(Return(true));
    NiceMock<MockUnvmeDeviceContext> mockDevCtx;
    mockDevCtx.ioCompletionCount = 0;
    EXPECT_CALL(mockDevCtx, GetPendingError).WillOnce(Return(&mockIoCtx));

    UnvmeDrv unvmeDrv(mockUnvmeCmd, mockSpdkNvmeCaller);

    // When
    uint32_t ret = unvmeDrv.CompleteErrors(&mockDevCtx);

    // Then
    EXPECT_EQ(ret, mockDevCtx.ioCompletionCount);
}

} // namespace pos
