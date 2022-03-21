#include "src/admin/smart_log_page_handler.h"

#include <gtest/gtest.h>

#include "spdk/pos.h"
#include "test/unit-tests/admin/disk_query_manager_mock.h"
#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/array/device/array_device_manager_mock.h"
#include "test/unit-tests/array/device/i_array_device_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/device/i_dev_info_mock.h"
#include "test/unit-tests/device/i_io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(SmartLogPageHandler, SmartLogPageHandler_Contructor_One_Stack)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockSmartLogMgr> smartLogMgr;

    SmartLogPageHandler smartLogPageHandler(&cmd, &ibofIo, &buffer, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
}

TEST(SmartLogPageHandler, SmartLogPageHandler_Contructor_One_Heap)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockSmartLogMgr> smartLogMgr;

    SmartLogPageHandler* smartLogPageHandler = new SmartLogPageHandler(&cmd, &ibofIo, &buffer, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    delete smartLogPageHandler;
}
TEST(SmartLogPageHandler, Execute_Return_true)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockSmartLogMgr> smartLogMgr;
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    SmartLogPageHandler smartLogPageHandler(&cmd, &ibofIo, &buffer, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr, &mockEventScheduler);

    bool actual, expected = true;
    actual = smartLogPageHandler.Execute();
    ASSERT_EQ(actual, expected);
}
TEST(SmartLogPageHandler, Execute_Return_true_null_buffer)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page* buffer = nullptr;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockSmartLogMgr> smartLogMgr;
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    SmartLogPageHandler smartLogPageHandler(&cmd, &ibofIo, buffer, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr, &mockEventScheduler);

    bool actual, expected = true;
    actual = smartLogPageHandler.Execute();
    ASSERT_EQ(actual, expected);
}
TEST(SmartLogPageHandler, Execute_Return_true_null_buffer_reenqueue)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    uint32_t originCore = 0;
    pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    struct spdk_nvme_cmd cmd;
    struct spdk_nvme_health_information_page buffer;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));

    SmartLogPageHandler smartLogPageHandler(&cmd, &ibofIo, &buffer, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr, &mockEventScheduler);

    NiceMock<MockDiskQueryManager> mockDiskQueryManager(&cmd, &buffer, &ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    ON_CALL(mockDiskQueryManager, Execute()).WillByDefault(Return(false));

    bool actual, expected = true;
    actual = smartLogPageHandler.Execute();
    ASSERT_EQ(actual, expected);
}

} // namespace pos
