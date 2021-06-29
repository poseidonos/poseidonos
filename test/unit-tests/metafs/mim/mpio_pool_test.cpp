#include "src/metafs/mim/mpio_pool.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MpioPool, AllocAndReleaseForSsd)
{
    const uint32_t COUNT = 10;
    int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::SSD, index, true, arrayId);
        }

        // check read mpio list
        bool isEmpty = pool->IsEmpty(static_cast<MpioType>(i));
        EXPECT_EQ(isEmpty, true);

        // check free read mpio
        Mpio* temp = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::SSD, COUNT + 1, true, arrayId);
        EXPECT_EQ(temp, nullptr);

        for (uint32_t index = 0; index < COUNT; index++)
        {
            pool->Release(mpioList[index]);
        }
    }

    delete pool;
}

TEST(MpioPool, AllocAndReleaseForNvRam)
{
    const uint32_t COUNT = 10;
    int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, index, false, arrayId);
        }

        // check read mpio list
        bool isEmpty = pool->IsEmpty(static_cast<MpioType>(i));
        EXPECT_EQ(isEmpty, true);

        // check free read mpio
        Mpio* temp = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, COUNT + 1, false, arrayId);
        EXPECT_EQ(temp, nullptr);

        for (uint32_t index = 0; index < COUNT; index++)
        {
            pool->Release(mpioList[index]);
        }
    }

    delete pool;
}

TEST(MpioPool, AllocAndReleaseForNvRamCache)
{
    const uint32_t COUNT = 10;
    int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);
    uint32_t index = 0;

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (uint32_t idx = 0; idx < COUNT; idx++)
    {
        Mpio* m = pool->Alloc(MpioType::Write, MetaStorageType::NVRAM, 0, true, arrayId);

        if (nullptr == mpioList[index])
            mpioList[index] = m;
        else if (m != mpioList[index])
            mpioList[++index] = m;
    }
    EXPECT_EQ(index, COUNT - 1);

    // check write mpio list, not hit all the mpios
    bool isEmpty = pool->IsEmpty(MpioType::Write);
    EXPECT_TRUE(isEmpty);

    // check free read mpio
    Mpio* temp = pool->Alloc(MpioType::Read, MetaStorageType::NVRAM, COUNT + 1, true, arrayId);
    EXPECT_NE(temp, nullptr);

    pool->Release(temp);

    for (index = 0; index < COUNT; index++)
    {
        if (nullptr == mpioList[index])
            continue;

        pool->Release(mpioList[index]);
    }

    delete pool;
}

} // namespace pos
