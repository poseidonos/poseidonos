#include "src/admin/admin_command_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "lib/spdk/include/spdk/bdev_module.h"
#include "spdk/pos.h"
#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/admin/smart_log_page_handler_mock.h"
#include "test/unit-tests/array/device/array_device_manager_mock.h"
#include "test/unit-tests/array/device/i_array_device_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/device/i_dev_info_mock.h"
#include "test/unit-tests/device/i_io_dispatcher_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using ::testing::_;
using testing::NiceMock;
using testing::Return;
namespace pos
{
TEST(AdminCommandHandler, AdminCommandHandler_Constructor_Heap_Nine_Args)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;

    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    AdminCommandHandler* adminCommandHandler = new AdminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr);
    delete req;
    delete bioPos;
    delete adminCommandHandler;
}
TEST(AdminCommandHandler, AdminCommandHandler_Constructor_Heap_eight_Args)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;

    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    AdminCommandHandler* adminCommandHandler = new AdminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr);
    delete req;
    delete bioPos;
    delete adminCommandHandler;
}

TEST(AdminCommandHandler, AdminCommandHandler_Constructor_Stack_eight_Args)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;

    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr);
    delete req;
    delete bioPos;
}

TEST(AdminCommandHandler, AdminCommandHandler_Constructor_Stack_Nine_args)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> smartLogMgr;
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;

    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &smartLogMgr);
    delete req;
    delete bioPos;
}

TEST(AdminCommandHandler, Execute_Run_SmartEnabledFalse)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.nvme_cmd.cdw10 = 0x7F0002;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));

    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(false));

    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr);

    bool expected = true, actual;
    actual = adminCommandHandler.Execute();
    delete req;
    delete bioPos;
}

TEST(AdminCommandHandler, Execute_Run_SmartEnabledTrue)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.nvme_cmd.cdw10 = 0x7F0002;
    req->cmd = &cmd;
    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));
    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr, &mockEventScheduler);

    bool expected = true, actual;
    actual = adminCommandHandler.Execute();
    delete req;
    delete bioPos;
}

TEST(AdminCommandHandler, Execute_Run_SmartEnabledTrue_ExecuteReturnFalse)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.nvme_cmd.cdw10 = 0x7F0002;
    req->cmd = &cmd;
    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr);

    ON_CALL(mockSmartLogMgr, GetSmartLogEnabled()).WillByDefault(Return(true));

    NiceMock<MockSmartLogPageHandler> mockSmartLogPageHandler(&req->cmd->nvme_cmd, &ibofIo, req->data, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr);
    ON_CALL(mockSmartLogPageHandler, Execute()).WillByDefault(Return(false));

    bool expected = true, actual;
    actual = adminCommandHandler.Execute();
    delete req;
    delete bioPos;
}
TEST(AdminCommandHandler, Execute_Run_OtherLid)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    cmd.nvme_cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
    cmd.nvme_cmd.cdw10 = 0x7F0004;
    req->cmd = &cmd;

    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));

    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr);

    bool expected = true, actual;
    actual = adminCommandHandler.Execute();
    delete req;
    delete bioPos;
}

TEST(AdminCommandHandler, Execute_Run_OtherIoType)
{
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockIDevInfo> devInfo;
    NiceMock<MockIIODispatcher> ioDispatcher;
    NiceMock<MockIArrayDevMgr> arrayDevMgr(NULL);
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    uint32_t originCore = 0;

    struct spdk_nvmf_request* req = new struct spdk_nvmf_request();
    struct spdk_bdev_io* bioPos = new struct spdk_bdev_io();
    union nvmf_h2c_msg cmd = {};
    struct pos_io ibofIo;

    ibofIo.ioType = IO_TYPE::READ;
    req->cmd = &cmd;
    bioPos->internal.caller_ctx = (void*)req;
    ibofIo.context = (void*)bioPos;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    AdminCommandHandler adminCommandHandler(&ibofIo, originCore, callback, &arrayInfo, &devInfo, &ioDispatcher, &arrayDevMgr, &mockSmartLogMgr);

    bool expected = true, actual;
    actual = adminCommandHandler.Execute();
    delete req;
    delete bioPos;
}

} // namespace pos
