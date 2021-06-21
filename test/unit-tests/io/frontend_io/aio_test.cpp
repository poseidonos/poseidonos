#include "src/io/frontend_io/aio.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/pos.h"
#include "src/bio/flush_io.h"
#include "src/include/pos_event_id.h"
#include "src/include/smart_ptr_type.h"
#include "src/io/frontend_io/flush_command_handler.h"
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

    // When : ioType is not write or read or flush
    posIo.ioType = IO_TYPE::FLUSH + 1;

    // Then : Expect throw exception
    EXPECT_THROW(aio.SubmitAsyncIO(posIo), POS_EVENT_ID);
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
    aio.SubmitAsyncIO(posIo);
}

TEST(AIO, AIO_CompleteIOs_CompleteIOs)
{
    // Given
    AIO aio;

    // When : do CompleteIos
    aio.CompleteIOs();

    // Then : do nothing
}

} // namespace pos
