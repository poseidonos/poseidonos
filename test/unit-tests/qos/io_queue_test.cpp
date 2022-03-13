#include "src/qos/io_queue.h"
#include "src/bio/volume_io.h"

#include <gtest/gtest.h>

#include <string>

#include "spdk/pos.h"

namespace pos
{
TEST(IoQueue, IoQueue_Constructor_One_Stack_Ubio)
{
    IoQueue<UbioSmartPtr> ioQueue();
}
TEST(IoQueue, IoQueue_Constructor_One_Heap_Ubio)
{
    IoQueue<UbioSmartPtr>* ioQueue = new IoQueue<UbioSmartPtr>;
    delete ioQueue;
}

TEST(IoQueue, Check_EnqueueDequeue_Ubio)
{
    IoQueue<UbioSmartPtr> ioQueue;
    int DATA_BUFFER_UNIT = 13;
    UbioSmartPtr ubio = make_shared<Ubio>(nullptr, DATA_BUFFER_UNIT, 1);
    uint32_t id1 = 0;
    uint32_t id2 = 0;
    ioQueue.EnqueueIo(id1, id2, ubio);

    UbioSmartPtr ubioReturned = ioQueue.DequeueIo(id1, id2);
    int arrayId = ubioReturned->GetArrayId();
    ASSERT_EQ(arrayId, 1);
}
} // namespace pos
