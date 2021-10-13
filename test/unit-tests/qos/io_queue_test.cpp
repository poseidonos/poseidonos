#include "src/qos/io_queue.h"

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

TEST(IoQueue, IoQueue_Constructor_One_Stack_pos_io)
{
    IoQueue<pos_io*> ioQueue();
}
TEST(IoQueue, IoQueue_Constructor_One_Heap_pos_io)
{
    IoQueue<pos_io*>* ioQueue = new IoQueue<pos_io*>;
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

TEST(IoQueue, Check_EnqueueDequeue_pos_io)
{
    IoQueue<pos_io*> ioQueue;
    //struct pos_io io = {4, 1, nullptr, 1, 0, 0, nullptr, nullptr};
    struct pos_io io;
    io.ioType = 4;
    uint32_t id1 = 0;
    uint32_t id2 = 0;
    ioQueue.EnqueueIo(id1, id2, &io);

    struct pos_io* ioReturned = ioQueue.DequeueIo(id1, id2);
    int ioType = ioReturned->ioType;
    ASSERT_EQ(ioType, 4);
}

} // namespace pos
