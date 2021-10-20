#include "src/admin/disk_query_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "spdk/nvme_spec.h"
#include "spdk/pos.h"
#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/array/device/array_device_manager_mock.h"
#include "test/unit-tests/array/device/i_array_device_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/i_dev_info_mock.h"
#include "test/unit-tests/device/i_io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"

using ::testing::_;
using testing::NiceMock;

namespace pos
{
TEST(GetLogPageContext, GetLogPageContext_Constructor_One)
{
    void* buffer;
    uint16_t lid = SPDK_NVME_LOG_HEALTH_INFORMATION;
    GetLogPageContext getLogPageContext(buffer, lid);
}
} // namespace pos

namespace pos
{
TEST(DiskQueryManager, DiskQueryManager_Contructor_One)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
}

TEST(DiskQueryManager, DiskQueryManager_Contructor_One_Heap)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    DiskQueryManager* diskQueryManager = new DiskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    delete diskQueryManager;
}
TEST(DiskQueryManager, Execute_smartOpc)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.cdw10 = 0x7F0002;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = true;
    bool actual = diskQueryManager.Execute();
    ASSERT_EQ(expected, actual);
}

TEST(DiskQueryManager, Execute_otherOpc)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_IDENTIFY;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = false;
    bool actual = diskQueryManager.Execute();
    ASSERT_EQ(expected, actual);
}

TEST(DiskQueryManager, SendSmartCommandtoDisk_Return)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.cdw10 = 0x7F0002;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = true, actual;
    string devName = "testDevice";

    MockUBlockDevice* rawPtr = new MockUBlockDevice(devName, 1024, nullptr);
    UblockSharedPtr mockDev = shared_ptr<MockUBlockDevice>(rawPtr);
    vector<UblockSharedPtr> devices;
    devices.push_back(mockDev);

    actual = diskQueryManager.SendSmartCommandtoDisk();
    ASSERT_EQ(expected, actual);
}

TEST(DiskQueryManager, SendSmartCommandtoDisk_DeviceSizeZero)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.cdw10 = 0x7F0002;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = true, actual;
    vector<UblockSharedPtr> devices;
    actual = diskQueryManager.SendSmartCommandtoDisk();
    ASSERT_EQ(expected, actual);
}

TEST(DiskQueryManager, SendLogPagetoDisk_smartLid)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.cdw10 = 0x7F0002;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = true, actual;
    actual = diskQueryManager.SendLogPagetoDisk(&cmd);
    ASSERT_EQ(expected, actual);
}

TEST(DiskQueryManager, SendLogPagetoDisk_otherLid)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.cdw10 = 0x7F0004;
    DiskQueryManager diskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    bool expected = false, actual;
    actual = diskQueryManager.SendLogPagetoDisk(&cmd);
    ASSERT_EQ(expected, actual);
}

} // namespace pos
