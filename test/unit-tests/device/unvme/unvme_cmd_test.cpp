#include "src/device/unvme/unvme_cmd.h"

#include <gtest/gtest.h>

#include "src/spdk_wrapper/abort_context.h"
#include "test/unit-tests/device/unvme/unvme_device_context_mock.h"
#include "test/unit-tests/device/unvme/unvme_io_context_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_nvme_caller_mock.h"
#include "src/admin/disk_query_manager.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(UnvmeCmd, RequestIO_testIfUbioDirIsWriteUncorAndWorksProperly)
{
    // Given
    NiceMock<MockUnvmeIOContext> mockIoContext;
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::WriteUncor));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeNsGetCtrlr).Times(1);
    EXPECT_CALL(*mockCaller, SpdkNvmeNsGetId).Times(1);
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrCmdIoRaw).WillOnce(Return(0));

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsAbortAndWorksProperly)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockDevContext, IncAdminCommandCount).Times(1);

    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, SetAdminCommand).Times(1);
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::Abort));
    AbortContext abortContext(nullptr, nullptr, 0);
    EXPECT_CALL(mockIoContext, GetBuffer).WillOnce(Return(&abortContext));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrCmdAbort).WillOnce(Return(0));

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsAdminPassThAndWorksProperly)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockDevContext, IncAdminCommandCount).Times(1);

    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, SetAdminCommand).Times(1);
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::AdminPassTh));
    struct spdk_nvme_cmd cmd;
    cmd.opc = spdk_nvme_admin_opcode::SPDK_NVME_OPC_GET_LOG_PAGE;
    EXPECT_CALL(mockIoContext, GetBuffer).Times(2).WillRepeatedly(Return(&cmd));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrCmdAdminRaw).WillOnce(Return(0));

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsNvmeCliAndWorksProperly)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockDevContext, IncAdminCommandCount).Times(1);

    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, SetAdminCommand).Times(1);
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::NvmeCli));
    struct spdk_nvme_cmd cmd;
    cmd.opc = spdk_nvme_admin_opcode::SPDK_NVME_OPC_GET_LOG_PAGE;
    EXPECT_CALL(mockIoContext, GetBuffer).Times(3).WillRepeatedly(Return(&cmd));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrCmdGetLogPage).WillOnce(Return(0));

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsDeallocateAndLbaIsNotAligned)
{
    // Given
    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, GetStartSectorOffset).WillRepeatedly(Return(1));
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::Deallocate));
    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, -EFAULT);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsDeallocateAndSectorCountIsNotAligned)
{
    // Given
    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, GetSectorCount).WillRepeatedly(Return(1));
    NiceMock<MockUnvmeDeviceContext> mockDevContext;
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::Deallocate));
    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, -EFAULT);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsGetLogPageAndWorksProperly)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevContext;

    GetLogPageContext ctx(nullptr, SPDK_NVME_LOG_HEALTH_INFORMATION);
    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::GetLogPage));
    EXPECT_CALL(mockIoContext, GetBuffer).WillOnce(Return(&ctx));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrCmdGetLogPage).WillOnce(Return(0));
    EXPECT_CALL(*mockCaller, SpdkNvmeNsGetCtrlr).WillOnce(nullptr);

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, 0);
}

TEST(UnvmeCmd, RequestIO_testIfUbioDirIsGetLogPageAndPageContextIsInvalid)
{
    // Given
    NiceMock<MockUnvmeDeviceContext> mockDevContext;

    GetLogPageContext ctx(nullptr, 0);
    NiceMock<MockUnvmeIOContext> mockIoContext;
    EXPECT_CALL(mockIoContext, GetOpcode).WillOnce(Return(UbioDir::GetLogPage));
    EXPECT_CALL(mockIoContext, GetBuffer).WillOnce(Return(&ctx));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();

    UnvmeCmd unvmeCmd(mockCaller);

    // When
    int ret = unvmeCmd.RequestIO(&mockDevContext, nullptr, &mockIoContext);

    // Then
    EXPECT_EQ(ret, -1);
}

TEST(UnvmeCmd, UnvmeCmdDestructor_testIfSpdkNvmeCallerIsNullptr)
{
    // Given
    SpdkNvmeCaller* spdkCaller = nullptr;
    UnvmeCmd* unvmeCmd = new UnvmeCmd(spdkCaller);

    // When
    delete unvmeCmd;

    // Then
    EXPECT_EQ(spdkCaller, nullptr);
}


} // namespace pos
