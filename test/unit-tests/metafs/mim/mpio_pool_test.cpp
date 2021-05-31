#include "src/metafs/mim/mpio_pool.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MpioPool, AllocAndReleaseForSsd)
{
    const uint32_t COUNT = 10;
    std::string arrayName = "TESTARRAY";
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::SSD, index, true, arrayName);
        }

        // check read mpio list
        bool isEmpty = pool->IsEmpty(static_cast<MpioType>(i));
        EXPECT_EQ(isEmpty, true);

        // check free read mpio
        Mpio* temp = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::SSD, COUNT + 1, true, arrayName);
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
    std::string arrayName = "TESTARRAY";
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, index, false, arrayName);
        }

        // check read mpio list
        bool isEmpty = pool->IsEmpty(static_cast<MpioType>(i));
        EXPECT_EQ(isEmpty, true);

        // check free read mpio
        Mpio* temp = pool->Alloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, COUNT + 1, false, arrayName);
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
    std::string arrayName = ""; // in this ut, mpio.io.arrayName is ""
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);
    uint32_t index = 0;

    size_t size = pool->GetPoolSize();
    EXPECT_EQ(size, COUNT);

    for (uint32_t idx = 0; idx < COUNT; idx++)
    {
        Mpio* m = pool->Alloc(MpioType::Write, MetaStorageType::NVRAM, 0, true, arrayName);

        if (nullptr == mpioList[index])
            mpioList[index] = m;
        else if (m != mpioList[index])
            mpioList[++index] = m;
    }

    // std::cout << "index=" << index << std::endl;
    EXPECT_LT(index, COUNT);

    // check read mpio list
    bool isEmpty = pool->IsEmpty(MpioType::Write);
    EXPECT_EQ(isEmpty, false);

    // check free read mpio
    Mpio* temp = pool->Alloc(MpioType::Read, MetaStorageType::NVRAM, COUNT + 1, true, arrayName);
    EXPECT_NE(temp, nullptr);

    for (uint32_t index = 0; index < COUNT; index++)
    {
        if (nullptr != mpioList[index])
            break;

        pool->Release(mpioList[index]);
    }

    pool->ReleaseCache();

    delete pool;
}

} // namespace pos
