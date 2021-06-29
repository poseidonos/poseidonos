#include "src/metafs/mim/mdpage_buf_pool.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MDPageBufPool, MDPageBufPool_Normal)
{
    const uint32_t COUNT = 10;
    MDPageBufPool* pool = new MDPageBufPool(COUNT);
    void* bufList[COUNT] = { 0, };

    pool->Init();

    size_t size = pool->GetCount();
    EXPECT_EQ(size, COUNT);

    for (int i = 0; i < COUNT; i++)
    {
        bufList[i] = pool->PopNewBuf();
    }

    size = pool->GetCount();
    EXPECT_EQ(size, 0);

    for (int i = 0; i < COUNT; i++)
    {
        pool->FreeBuf(bufList[i]);
    }

    size = pool->GetCount();
    EXPECT_EQ(size, COUNT);

    delete pool;
}

} // namespace pos
