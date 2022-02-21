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

#include "src/metafs/common/fifo_cache.h"

#include <gtest/gtest.h>

#include <memory>

#include "src/metafs/mim/mpio.h"
#include "test/unit-tests/metafs/mim/mpio_mock.h"

namespace pos
{
std::shared_ptr<MockMpio>
GetMockMpio(std::pair<int, MetaLpnType> input)
{
    MpioIoInfo ioInfo;
    ioInfo.arrayId = input.first;
    ioInfo.metaLpn = input.second;
    std::shared_ptr<MockMpio> mpio = std::make_shared<MockMpio>(nullptr);
    EXPECT_CALL(*mpio, Setup);
    mpio->Setup(ioInfo, true, false, nullptr);
    return mpio;
}

TEST(FifoCache, Find_testIfTheMethodCanRetriveCachedItem)
{
    const size_t CACHE_COUNT = 5;
    std::pair<int, MetaLpnType> pairInput;
    FifoCache<int, MetaLpnType, std::shared_ptr<Mpio>> test(CACHE_COUNT);

    ASSERT_TRUE(test.IsEmpty());

    pairInput = {0, 0};
    auto mpio = GetMockMpio(pairInput);
    auto result = test.Push(pairInput, mpio);
    EXPECT_FALSE(test.IsEmpty());
    EXPECT_EQ(nullptr, result);
    EXPECT_EQ(1, test.GetSize());

    EXPECT_EQ(mpio, test.Find(pairInput));

    pairInput = {0, 1};
    EXPECT_EQ(nullptr, test.Find(pairInput));

    pairInput = {1, 0};
    EXPECT_EQ(nullptr, test.Find(pairInput));
}

TEST(FifoCache, Insert_testIfTheMethodCanPushItemsCorrectly)
{
    const size_t CACHE_COUNT = 5;
    std::pair<int, MetaLpnType> pairInput;
    FifoCache<int, MetaLpnType, std::shared_ptr<Mpio>> test(CACHE_COUNT);

    ASSERT_TRUE(test.IsEmpty());

    pairInput = {0, 0};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(1, test.GetSize());

    pairInput = {0, 1};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(2, test.GetSize());

    // insert again
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(2, test.GetSize());
}

TEST(FifoCache, Insert_testIfTheMethodCanRemoveOlestItemsCorrectly)
{
    const size_t CACHE_COUNT = 3;
    std::pair<int, MetaLpnType> pairInput;
    FifoCache<int, MetaLpnType, std::shared_ptr<Mpio>> test(CACHE_COUNT);

    ASSERT_TRUE(test.IsEmpty());

    // the oldest
    pairInput = {0, 0};
    auto oldest = GetMockMpio(pairInput);
    EXPECT_EQ(nullptr, test.Push(pairInput, oldest));
    EXPECT_EQ(1, test.GetSize());

    pairInput = {0, 1};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(2, test.GetSize());

    pairInput = {0, 2};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(3, test.GetSize());

    // insert again
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(3, test.GetSize());

    // insert new
    pairInput = {0, 3};
    EXPECT_EQ(oldest, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(3, test.GetSize());
}

TEST(FifoCache, IsFull_testIfTheCacheCanShowWhetherItIsFull)
{
    const size_t CACHE_COUNT = 5;
    std::pair<int, MetaLpnType> pairInput;
    FifoCache<int, MetaLpnType, std::shared_ptr<Mpio>> test(CACHE_COUNT);

    ASSERT_TRUE(test.IsEmpty());

    // arrayId: 0, lpn: 0
    pairInput = {0, 0};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));

    // arrayId: 0, lpn: 1
    pairInput = {0, 1};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));

    // arrayId: 0, lpn: 2
    pairInput = {0, 2};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));

    // arrayId: 0, lpn: 3
    pairInput = {0, 3};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_FALSE(test.IsFull());

    // arrayId: 0, lpn: 4
    pairInput = {0, 4};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_TRUE(test.IsFull());
}

TEST(FifoCache, Pop_testIfTheMethodCanPopTheWantedItem)
{
    const size_t CACHE_COUNT = 5;
    std::pair<int, MetaLpnType> pairInput;
    FifoCache<int, MetaLpnType, std::shared_ptr<Mpio>> test(CACHE_COUNT);

    // arrayId: 0, lpn: 0
    pairInput = {0, 0};
    ASSERT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));

    // arrayId: 0, lpn: 1
    pairInput = {0, 1};
    auto mpio = GetMockMpio(pairInput);
    EXPECT_EQ(nullptr, test.Push(pairInput, mpio));

    // arrayId: 0, lpn: 2
    pairInput = {0, 2};
    EXPECT_EQ(nullptr, test.Push(pairInput, GetMockMpio(pairInput)));
    EXPECT_EQ(3, test.GetSize());

    pairInput = {0, 1};
    EXPECT_EQ(mpio, test.Remove(pairInput));
    EXPECT_EQ(2, test.GetSize());
}
} // namespace pos
