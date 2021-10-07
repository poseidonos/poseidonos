#include "src/device/unvme/unvme_mgmt.h"

#include <gtest/gtest.h>

#include "lib/spdk/lib/nvme/nvme_internal.h"
#include "src/device/unvme/unvme_device_context.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_nvme_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/nvme_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{

TEST(UnvmeMgmt, Open_testIfDeviceContextIsNullptr)
{
    // Given
    UnvmeMgmt unvmeMgmt;

    // When
    bool ret = unvmeMgmt.Open(nullptr);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UnvmeMgmt, Open_testIfNamespaceIsNullptr)
{
    // Given
    UnvmeMgmt unvmeMgmt;
    UnvmeDeviceContext devCtx;
    devCtx.ns = nullptr;

    // When
    bool ret = unvmeMgmt.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UnvmeMgmt, Open_testIfIoQpairIsAlreadyExist)
{
    // Given
    UnvmeMgmt unvmeMgmt;
    UnvmeDeviceContext devCtx;
    struct spdk_nvme_ns ns;
    struct spdk_nvme_qpair qpair;
    devCtx.ns = &ns;
    devCtx.ioQPair = &qpair;

    // When
    bool ret = unvmeMgmt.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UnvmeMgmt, Open_testIfFailedToAllocateIoQpair)
{
    // Given
    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrAllocIoQpair).WillOnce(Return(nullptr));
    UnvmeMgmt unvmeMgmt(mockCaller);
    UnvmeDeviceContext devCtx;
    struct spdk_nvme_ns ns;
    devCtx.ns = &ns;
    devCtx.ioQPair = nullptr;

    // When
    bool ret = unvmeMgmt.Open(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}


TEST(UnvmeMgmt, Close_testIfDeviceContextIsNullptr)
{
    // Given
    UnvmeMgmt unvmeMgmt;

    // When
    bool ret = unvmeMgmt.Close(nullptr);

    // Then
    EXPECT_TRUE(ret);
}

TEST(UnvmeMgmt, Close_testIfIoQpairIsNullptr)
{
    // Given
    UnvmeMgmt unvmeMgmt;
    UnvmeDeviceContext devCtx;
    devCtx.ioQPair = nullptr;

    // When
    bool ret = unvmeMgmt.Close(&devCtx);

    // Then
    EXPECT_TRUE(ret);
}

TEST(UnvmeMgmt, Close_testIfFailedToFreeIoQpair)
{
    // Given
    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeCtrlrFreeIoQpair).WillOnce(Return(-1));
    UnvmeMgmt unvmeMgmt(mockCaller);
    UnvmeDeviceContext devCtx;
    struct spdk_nvme_qpair qpair;
    devCtx.ioQPair = &qpair;

    // When
    bool ret = unvmeMgmt.Close(&devCtx);

    // Then
    EXPECT_FALSE(ret);
}

TEST(UnvmeMgmt, ScanDevs_testIfSpdkInitAlreadyDone)
{
    // Given
    UnvmeMgmt unvmeMgmt(nullptr, true);

    // When
    int addedDeviceCount = unvmeMgmt.ScanDevs(nullptr, nullptr, nullptr);

    // Then
    EXPECT_EQ(addedDeviceCount, 0);
}

TEST(UnvmeMgmt, ScanDevs_testIfFailedToInitController)
{
    // Given
    string monitorName = "UnvmeMgmtTest";
    MockNvme mockNvme(monitorName);
    EXPECT_CALL(mockNvme, InitController).WillOnce(Return(nullptr));
    UnvmeMgmt unvmeMgmt;

    // When
    int addedDeviceCount = unvmeMgmt.ScanDevs(nullptr, &mockNvme, nullptr);

    // Then
    EXPECT_EQ(addedDeviceCount, 0);
}

TEST(UnvmeMgmt, ScanDevs_testIfDeviceSectorSizeIsNotAllowed)
{
    // Given
    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeNsGetSectorSize).WillOnce(Return(0));

    string monitorName = "UnvmeMgmtTest";
    MockNvme mockNvme(monitorName);
    struct NsEntry entry;
    auto nsList = std::list<struct NsEntry*>();
    nsList.push_back(&entry);
    EXPECT_CALL(mockNvme, InitController).WillOnce(Return(&nsList));

    UnvmeMgmt unvmeMgmt(mockCaller);

    // When
    int addedDeviceCount = unvmeMgmt.ScanDevs(nullptr, &mockNvme, nullptr);

    // Then
    EXPECT_EQ(addedDeviceCount, 0);
}

TEST(UnvmeMgmt, ScanDevs_testIfFoundExistingDevice)
{
    // Given
    string monitorName = "UnvmeMgmtTest";
    MockNvme mockNvme(monitorName);
    struct NsEntry entry;
    auto nsList = std::list<struct NsEntry*>();
    nsList.push_back(&entry);
    EXPECT_CALL(mockNvme, InitController).WillOnce(Return(&nsList));

    NiceMock<MockSpdkNvmeCaller>* mockCaller = new NiceMock<MockSpdkNvmeCaller>();
    EXPECT_CALL(*mockCaller, SpdkNvmeNsGetSectorSize).WillOnce(Return(512));

    shared_ptr<MockUBlockDevice> mockUblock =
        make_shared<MockUBlockDevice>("", 0, nullptr);
    EXPECT_CALL(*mockUblock, GetName).WillOnce(Return("unvme-ns-0"));
    vector<UblockSharedPtr> devs;
    devs.push_back(mockUblock);

    UnvmeMgmt unvmeMgmt(mockCaller);

    // When
    int addedDeviceCount = unvmeMgmt.ScanDevs(&devs, &mockNvme, nullptr);

    // Then
    EXPECT_EQ(addedDeviceCount, 0);
}

} // namespace pos
