#include "src/admin/disk_smart_complete_handler.h"

#include <gtest/gtest.h>

#include "spdk/nvme_spec.h"
#include "spdk/pos.h"
#include "test/unit-tests/admin/smart_log_mgr_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"

using ::testing::_;
using testing::NiceMock;
using testing::Return;
namespace pos
{
TEST(DiskSmartCompleteHandler, DiskSmartCompleteHandler_Constructor_One)
{
    struct spdk_nvme_health_information_page* buffer;
    uint32_t volId = 0;
    uint32_t originCore = 0;
    pos_io ibofIo;
    uint32_t arrayId = 0;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockSmartLogMgr> smartLogMgr;
    DiskSmartCompleteHandler diskSmartCompleteHandler(buffer, volId, arrayId, originCore, &ibofIo, callback, &smartLogMgr);
}

TEST(DiskSmartCompleteHandler, DiskSmartCompleteHandler_Constructor_One_heap)
{
    struct spdk_nvme_health_information_page* buffer;
    uint32_t volId = 0;
    uint32_t originCore = 0;
    pos_io ibofIo;
    uint32_t arrayId = 0;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockSmartLogMgr> smartLogMgr;
    DiskSmartCompleteHandler* diskSmartCompleteHandler = new DiskSmartCompleteHandler(buffer, volId, arrayId, originCore, &ibofIo, callback, &smartLogMgr);
    delete diskSmartCompleteHandler;
}

TEST(DiskSmartCompleteHandler, _DoSpecificJob_Execute)
{
    struct spdk_nvme_health_information_page buffer;
    uint32_t volId = 0;
    uint32_t originCore = 0;
    pos_io ibofIo;
    uint32_t arrayId = 0;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockSmartLogMgr> smartLogMgr;
    DiskSmartCompleteHandler diskSmartCompleteHandler(&buffer, volId, arrayId, originCore, &ibofIo, callback, &smartLogMgr);
    bool expected = true, actual;
    actual = diskSmartCompleteHandler.Execute();
}
TEST(DiskSmartCompleteHandler, _DoSpecificJob_NonZeroReadWriteBytes)
{
    struct spdk_nvme_health_information_page buffer;
    uint32_t volId = 0;
    uint32_t originCore = 0;
    pos_io ibofIo;
    uint32_t arrayId = 0;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    DiskSmartCompleteHandler diskSmartCompleteHandler(&buffer, volId, arrayId, originCore, &ibofIo, callback, &mockSmartLogMgr);

    ON_CALL(mockSmartLogMgr, GetReadBytes(_, _)).WillByDefault(Return(1));
    ON_CALL(mockSmartLogMgr, GetWriteBytes(_, _)).WillByDefault(Return(1));
    buffer.temp_sensor[0] = 0;
    bool expected = true, actual;
    actual = diskSmartCompleteHandler.Execute();
}

TEST(DiskSmartCompleteHandler, _DoSpecificJob_ZeroReadWriteBytes)
{
    struct spdk_nvme_health_information_page buffer;
    uint32_t volId = 0;
    uint32_t originCore = 0;
    pos_io ibofIo;
    uint32_t arrayId = 0;
    ibofIo.ioType = IO_TYPE::GET_LOG_PAGE;
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockSmartLogMgr> mockSmartLogMgr;
    DiskSmartCompleteHandler diskSmartCompleteHandler(&buffer, volId, arrayId, originCore, &ibofIo, callback, &mockSmartLogMgr);

    ON_CALL(mockSmartLogMgr, GetReadBytes(_, _)).WillByDefault(Return(0));
    ON_CALL(mockSmartLogMgr, GetWriteBytes(_, _)).WillByDefault(Return(0));
    buffer.temp_sensor[0] = 2;
    bool expected = true, actual;
    actual = diskSmartCompleteHandler.Execute();
}

TEST(DiskSmartCompleteHandler, _AddComponentTemperature_)
{
}

TEST(DiskSmartCompleteHandler, _SetValfromSmartLogMgr_)
{
}

} // namespace pos
