#include "src/metafs/mim/mpio_handler.h"
#include "test/unit-tests/metafs/mim/metafs_io_q_mock.h"
#include "test/unit-tests/metafs/mim/mpio_pool_mock.h"
#include "test/unit-tests/metafs/mim/write_mpio_mock.h"
#include <gtest/gtest.h>

using ::testing::Return;

namespace pos
{
TEST(MpioHandler, Normal)
{
    const int MAX_COUNT = 32 * 1024;

    MockMpioPool* pool = new MockMpioPool(100);
    EXPECT_CALL(*pool, GetPoolSize);
    EXPECT_CALL(*pool, ReleaseCache).WillRepeatedly(Return());

    MockWriteMpio* mpio = new MockWriteMpio(this);
    EXPECT_CALL(*mpio, ExecuteAsyncState).WillRepeatedly(Return());

    MockMetaFsIoQ<Mpio*> doneQ;
    EXPECT_CALL(doneQ, Init);
    EXPECT_CALL(doneQ, Enqueue).WillRepeatedly(Return(true));
    EXPECT_CALL(doneQ, Dequeue).WillRepeatedly(Return(mpio));

    MpioHandler* handler = new MpioHandler(0, 0, &doneQ);
    handler->BindMpioPool(pool);

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->EnqueuePartialMpio(mpio);
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        handler->BottomhalfMioProcessing();
    }

    delete handler;
    delete pool;
    delete mpio;
}

} // namespace pos
