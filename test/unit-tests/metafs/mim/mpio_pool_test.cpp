/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/metafs/mim/mpio_pool.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MpioPool, AllocAndReleaseForSsd)
{
    const uint32_t COUNT = 10;
    const int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    EXPECT_EQ(pool->GetCapacity(), COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->TryAlloc(static_cast<MpioType>(i), MetaStorageType::SSD, index, true, arrayId);
        }

        // check read mpio list
        EXPECT_TRUE(pool->IsEmpty(static_cast<MpioType>(i)));

        // check free read mpio
        Mpio* temp = pool->TryAlloc(static_cast<MpioType>(i), MetaStorageType::SSD, COUNT + 1, true, arrayId);
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
    const int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    EXPECT_EQ(pool->GetCapacity(), COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = pool->TryAlloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, index, false, arrayId);
        }

        // check read mpio list
        EXPECT_TRUE(pool->IsEmpty(static_cast<MpioType>(i)));

        // check free read mpio
        Mpio* temp = pool->TryAlloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, COUNT + 1, false, arrayId);
        EXPECT_EQ(temp, nullptr);

        for (uint32_t index = 0; index < COUNT; index++)
        {
            pool->Release(mpioList[index]);
        }
    }

    delete pool;
}

TEST(MpioPool, CheckCounter)
{
    const uint32_t COUNT = 10;
    const int arrayId = 0;
    Mpio* mpioList_r[10] = { 0, };
    Mpio* mpioList_w[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);

    EXPECT_EQ(pool->GetCapacity(), COUNT);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Read), COUNT);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Write), COUNT);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Read), 0);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Write), 0);

    uint32_t popCount = 4;

    for (uint32_t index = 0; index < popCount; index++)
    {
        mpioList_r[index] = pool->TryAlloc(MpioType::Read, MetaStorageType::SSD, index, true, arrayId);
        ASSERT_NE(mpioList_r[index], nullptr);
        mpioList_w[index] = pool->TryAlloc(MpioType::Write, MetaStorageType::SSD, index, true, arrayId);
        ASSERT_NE(mpioList_w[index], nullptr);
    }

    EXPECT_EQ(pool->GetCapacity(), COUNT);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Read), COUNT - popCount);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Write), COUNT - popCount);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Read), popCount);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Write), popCount);

    for (uint32_t index = 0; index < popCount; index++)
    {
        pool->Release(mpioList_r[index]);
        pool->Release(mpioList_w[index]);
    }

    EXPECT_EQ(pool->GetCapacity(), COUNT);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Read), COUNT);
    EXPECT_EQ(pool->GetFreeCount(MpioType::Write), COUNT);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Read), 0);
    EXPECT_EQ(pool->GetUsedCount(MpioType::Write), 0);

    delete pool;
}

TEST(MpioPool, AllocAndReleaseForNvRamCache)
{
#if MPIO_CACHE_EN
    const uint32_t COUNT = 10;
    const int arrayId = 0;
    Mpio* mpioList[10] = { 0, };
    MpioPool* pool = new MpioPool(COUNT);
    uint32_t index = 0;

    EXPECT_EQ(pool->GetCapacity(), COUNT);

    for (uint32_t idx = 0; idx < COUNT; idx++)
    {
        Mpio* m = pool->TryAlloc(MpioType::Write, MetaStorageType::NVRAM, 0, true, arrayId);

        if (nullptr == mpioList[index])
            mpioList[index] = m;
        else if (m != mpioList[index])
            mpioList[++index] = m;
    }
    EXPECT_EQ(index, COUNT - 1);

    // check write mpio list, not hit all the mpios
    EXPECT_TRUE(pool->IsEmpty(MpioType::Write));

    // check free read mpio
    Mpio* temp = pool->TryAlloc(MpioType::Read, MetaStorageType::NVRAM, COUNT + 1, true, arrayId);
    EXPECT_NE(temp, nullptr);

    pool->Release(temp);

    for (index = 0; index < COUNT; index++)
    {
        if (nullptr == mpioList[index])
            continue;

        pool->Release(mpioList[index]);
    }

    delete pool;
#endif
}

} // namespace pos
