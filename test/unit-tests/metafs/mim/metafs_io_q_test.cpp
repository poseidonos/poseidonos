#include "src/metafs/mim/metafs_io_q.h"
#include "src/metafs/mim/metafs_io_request.h"
#include "test/unit-tests/metafs/mim/mio_mock.h"
#include "test/unit-tests/metafs/mim/mpio_mock.h"
#include "test/unit-tests/metafs/mim/mpio_pool_mock.h"
#include <gtest/gtest.h>

namespace pos
{
/*** MetaFsIoRequest* ***/
TEST(MetaFsIoQ_Msg, ConstructorAndDestructor0)
{
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>();
    delete q;
}

TEST(MetaFsIoQ_Msg, ConstructorAndDestructor1)
{
    const int WEIGHT = 100;
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>(WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Msg, CheckInit)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>(WEIGHT);
    q->Init("test", SIZE);
    EXPECT_TRUE(q->IsEmpty());
    EXPECT_TRUE(q->IsAllQEmpty());
    delete q;
}

TEST(MetaFsIoQ_Msg, CheckEnqueueAndDequeue)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>(WEIGHT);

    for (int i = 0; i < SIZE; i++)
    {
        MetaFsIoRequest* msg = new MetaFsIoRequest();
        EXPECT_TRUE(q->Enqueue(msg));
    }

    EXPECT_EQ(q->GetItemCnt(), SIZE);

    MetaFsIoRequest* msg = nullptr;
    while (nullptr != (msg = q->Dequeue()))
    {
        delete msg;
    }

    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

TEST(MetaFsIoQ_Msg, CheckWeight)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>(WEIGHT);
    q->SetWeightFactor(WEIGHT);
    EXPECT_EQ(q->GetWeightFactor(), WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Msg, CleanQ)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<MetaFsIoRequest*>* q = new MetaFsIoQ<MetaFsIoRequest*>(WEIGHT);

    for (int i = 0; i < SIZE; i++)
    {
        MetaFsIoRequest* msg = new MetaFsIoRequest();
        EXPECT_TRUE(q->Enqueue(msg));
    }

    q->CleanQEntry();
    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

/*** Mio* ***/
TEST(MetaFsIoQ_Mio, ConstructorAndDestructor0)
{
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>();
    delete q;
}

TEST(MetaFsIoQ_Mio, ConstructorAndDestructor1)
{
    const int WEIGHT = 100;
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>(WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Mio, CheckInit)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>(WEIGHT);
    q->Init("test", SIZE);
    EXPECT_TRUE(q->IsEmpty());
    EXPECT_TRUE(q->IsAllQEmpty());
    delete q;
}

TEST(MetaFsIoQ_Mio, CheckEnqueueAndDequeue)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>(WEIGHT);
    MockMpioPool* pool = new MockMpioPool(SIZE);

    for (int i = 0; i < SIZE; i++)
    {
        MockMio* msg = new MockMio(pool);
        EXPECT_TRUE(q->Enqueue(msg));
    }

    EXPECT_EQ(q->GetItemCnt(), SIZE);

    MockMio* msg = nullptr;
    while (nullptr != (msg = dynamic_cast<MockMio*>(q->Dequeue())))
    {
        delete msg;
    }

    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

TEST(MetaFsIoQ_Mio, CheckWeight)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>(WEIGHT);
    q->SetWeightFactor(WEIGHT);
    EXPECT_EQ(q->GetWeightFactor(), WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Mio, CleanQ)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mio*>* q = new MetaFsIoQ<Mio*>(WEIGHT);
    MockMpioPool* pool = new MockMpioPool(SIZE);

    for (int i = 0; i < SIZE; i++)
    {
        MockMio* msg = new MockMio(pool);
        EXPECT_TRUE(q->Enqueue(msg));
    }

    q->CleanQEntry();
    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

/*** Mpio* ***/
TEST(MetaFsIoQ_Mpio, ConstructorAndDestructor0)
{
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>();
    delete q;
}

TEST(MetaFsIoQ_Mpio, ConstructorAndDestructor1)
{
    const int WEIGHT = 100;
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>(WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Mpio, CheckInit)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>(WEIGHT);
    q->Init("test", SIZE);
    EXPECT_TRUE(q->IsEmpty());
    EXPECT_TRUE(q->IsAllQEmpty());
    delete q;
}

TEST(MetaFsIoQ_Mpio, CheckEnqueueAndDequeue)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>(WEIGHT);

    for (int i = 0; i < SIZE; i++)
    {
        MockMpio* msg = new MockMpio(nullptr);
        EXPECT_TRUE(q->Enqueue(msg));
    }

    EXPECT_EQ(q->GetItemCnt(), SIZE);

    MockMpio* msg = nullptr;
    while (nullptr != (msg = dynamic_cast<MockMpio*>(q->Dequeue())))
    {
        delete msg;
    }

    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

TEST(MetaFsIoQ_Mpio, CheckWeight)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>(WEIGHT);
    q->SetWeightFactor(WEIGHT);
    EXPECT_EQ(q->GetWeightFactor(), WEIGHT);
    delete q;
}

TEST(MetaFsIoQ_Mpio, CleanQ)
{
    const int WEIGHT = 100;
    const int SIZE = 200;
    MetaFsIoQ<Mpio*>* q = new MetaFsIoQ<Mpio*>(WEIGHT);

    for (int i = 0; i < SIZE; i++)
    {
        MockMpio* msg = new MockMpio(nullptr);
        EXPECT_TRUE(q->Enqueue(msg));
    }

    q->CleanQEntry();
    EXPECT_EQ(q->GetItemCnt(), 0);

    delete q;
}

} // namespace pos
