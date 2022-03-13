#include "src/io/frontend_io/aio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/nvmf_transport.h"
#include "spdk/pos.h"
#include "src/bio/flush_io.h"
#include "src/include/pos_event_id.h"
#include "src/include/smart_ptr_type.h"
#include "src/io/frontend_io/flush_command_handler.h"
#include "src/spdk_wrapper/spdk.h"
#include "test/unit-tests/array_components/components_info_mock.h"
#include "test/unit-tests/array_mgmt/array_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/bio/flush_io_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(IOCtx, IOCtx_Constructor_Stack)
{
    // Given

    // When : Create new IOCtx object in stack
    IOCtx ioctx;

    // Then : Check cnt value equals 0
    ASSERT_EQ(0, ioctx.cnt);
}

TEST(IOCtx, IOCtx_Constructor_Heap)
{
    // Given

    // When : Create new IOCtx object in heap
    IOCtx* ioctx = new IOCtx();

    // Then : Release memory
    ASSERT_EQ(0, ioctx->cnt);
    delete ioctx;
}

} // namespace pos

namespace pos
{
TEST(AioCompletion, AioCompletion_Constructor_ThreeArgumentWithFlushIo_Stack)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    FlushIoSmartPtr flushIo(new NiceMock<MockFlushIo>(0));

    // When : Create new object in stack
    AioCompletion aioCompletion(flushIo, posIo, ioContext);

    // Then : Do nothing
}

TEST(AioCompletion, AioCompletion_Constructor_ThreeArgumentWithFlushIo_Heap)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    FlushIoSmartPtr flushIo(new NiceMock<MockFlushIo>(0));

    // When : Create new object in heap
    AioCompletion* aioCompletion = new AioCompletion(flushIo, posIo, ioContext);

    // Then : Release memory
    delete aioCompletion;
}

TEST(AioCompletion, AioCompletion_Constructor_FourArgumentWithFlushIo_Stack)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    FlushIoSmartPtr flushIo(new NiceMock<MockFlushIo>(0));
    MockEventFrameworkApi mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsSameReactorNow(_)).WillByDefault(Return(true));

    // When : Create new object in stack
    AioCompletion aioCompletion(flushIo, posIo, ioContext, &mockEventFrameworkApi);

    // Then : Do nothing
}

TEST(AioCompletion, AioCompletion_Constructor_ThreeArgumentWithVolumeIo_Stack)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));

    // When : Create new object in stack
    AioCompletion aioCompletion(volumeIo, posIo, ioContext);

    // Then : Do nothing
}

TEST(AioCompletion, AioCompletion_Constructor_FourArgumentWithVolumeIo_Stack)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    MockEventFrameworkApi mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, IsSameReactorNow(_)).WillByDefault(Return(true));

    // When : Create new object in stack
    AioCompletion aioCompletion(volumeIo, posIo, ioContext, &mockEventFrameworkApi);

    // Then : Do nothing
}

TEST(AioCompletion, AioCompletion_DoSpecificJob_IoTypeFlush)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    IOCtx ioContext;
    NiceMock<MockFlushIo>* mockFlushIo = new NiceMock<MockFlushIo>(0);
    FlushIoSmartPtr flushIo(mockFlushIo);
    MockEventFrameworkApi mockEventFrameworkApi;
    bool expect, actual;

    // When IO type is Flush and SameReactorNow
    posIo.ioType = IO_TYPE::FLUSH;
    posIo.complete_cb = nullptr;
    ON_CALL(*mockFlushIo, GetOriginCore()).WillByDefault(Return(0));
    ON_CALL(mockEventFrameworkApi, IsSameReactorNow(_)).WillByDefault(Return(true));
    AioCompletion aioCompletion(flushIo, posIo, ioContext, &mockEventFrameworkApi);
    aioCompletion.SetCallee(nullptr);

    // Then call _SendUserCompletion and return true
    expect = true;
    actual = aioCompletion.Execute();
    ASSERT_EQ(expect, actual);
}

} // namespace pos

namespace pos
{
TEST(AIO, AIO_Constructor_Stack)
{
    // Given

    // When : Create aio in stack
    AIO aio();

    // Then : Do nothing
}

TEST(AIO, AIO_Constructor_Heap)
{
    // Given

    // When : Create aio in heap
    AIO* aio = new AIO();

    // Then : Do nothing
    delete aio;
}

TEST(AIO, AIO_SubmitAsyncIO_IoTypeInvalidThrow)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;
    AIO aio;

    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Deallocate;
    // Then : Expect throw exception
    EXPECT_THROW(aio.SubmitAsyncIO(volIo), POS_EVENT_ID);
}

TEST(AIO, AIO_SubmitAsyncIO_IoTypeFlush)
{
    // Given
    pos_io posIo;
    posIo.array_id = 0;

    // When : ioType is flush
    posIo.ioType = IO_TYPE::FLUSH;
    AIO aio;

    // Then : SubmitAsyncIo done
    aio.SubmitFlush(posIo);
}

TEST(AIO, AIO_CompleteIOs_CompleteIOs)
{
    // Given
    AIO aio;

    // When : do CompleteIos
    aio.CompleteIOs();

    // Then : do nothing
}

TEST(AIO, AIO_SubmitAsyncAdmin_IoTypeGetLogPage)
{
    // Given
    AIO aio;
    pos_io posIo;
    spdk_bdev_io bio;
    spdk_nvmf_request nvmfRequest;
    nvmf_h2c_msg nvmfMsg;
    spdk_nvme_cmd nvmeCmd;
    NiceMock<MockArrayManager>* mockArrayManager = new NiceMock<MockArrayManager>();
    NiceMock<MockIArrayInfo>* mockArrayInfo = new NiceMock<MockIArrayInfo>();
    MockComponentsInfo mockComponentsInfo{mockArrayInfo, nullptr};

    char testArrayName[16] = {""};

    // When : Call SubmitAsyncAdmin
    posIo.ioType = IO_TYPE::GET_LOG_PAGE;
    posIo.arrayName = testArrayName;
    nvmeCmd.cdw10 = SPDK_NVME_LOG_ERROR;
    nvmfMsg.nvme_cmd = nvmeCmd;
    nvmfRequest.cmd = &nvmfMsg;
    bio.internal.caller_ctx = &nvmfRequest;
    posIo.context = &bio;

    ON_CALL(*mockArrayManager, GetInfo(_)).WillByDefault(Return(&mockComponentsInfo));
    ON_CALL(*mockArrayInfo, GetArrayManager()).WillByDefault(Return(nullptr));
    EventSchedulerSingleton::Instance()->DequeueEvents();

    aio.SubmitAsyncAdmin(posIo, mockArrayManager);

    // Then : check eventscheduler queue
    std::queue<EventSmartPtr> queueList = EventSchedulerSingleton::Instance()->DequeueEvents();
    ASSERT_EQ(1, queueList.size());

    EventSchedulerSingleton::Instance()->DequeueEvents();
    delete mockArrayManager;
    delete mockArrayInfo;
}

TEST(AdminCompletion, AdminCompletion_Stack)
{
    // Given
    pos_io posIo;
    IOCtx ioContext;

    // When : Create adminComplition on stack
    AdminCompletion adminCompletion(&posIo, ioContext, 0);

    // Then : do nothing
}

TEST(AdminCompletion, AdminCompletion_Heap)
{
    // Given
    pos_io posIo;
    IOCtx ioContext;

    // When : Create adminComplition on heap
    AdminCompletion* adminCompletion = new AdminCompletion(&posIo, ioContext, 0);

    // Then : Release adminComplition
    delete adminCompletion;
}

TEST(AdminCompletion, AdminCompletion_DoSpecificJob)
{
    // Given
    pos_io posIo;
    IOCtx ioContext;
    bool expected;

    // When : Call DoSpecificJob
    AdminCompletion adminCompletion(&posIo, ioContext, EventFrameworkApiSingleton::Instance()->GetCurrentReactor());
    posIo.complete_cb = [](struct pos_io* io, int status) { return; };
    expected = adminCompletion.Execute();

    // Then : check function works
    ASSERT_EQ(true, expected);
}

} // namespace pos
