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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/lib/bitmap.h"

#include "gtest/gtest.h"

using namespace std;
using namespace pos;

#define TEST_COUNT 130

static const int BITMAP_SIZE = 1024;

class BitmapTest : public ::testing::Test
{
protected:
    virtual void
    SetUp(void)
    {
        test = new BitMapMutex(BITMAP_SIZE);
    }

    virtual void
    TearDown(void)
    {
        delete test;
    }

    BitMapMutex* test;
};

TEST_F(BitmapTest, ResetBit)
{
    uint32_t test_cnt = 10;
    for (uint32_t i = 0; i < test_cnt; i++)
    {
        test->SetBit(i);
    }
    EXPECT_EQ(test->GetNumBitsSet(), test_cnt);
    test->ResetBitmap();
    EXPECT_EQ(test->GetNumBitsSet(), 0);
}

TEST_F(BitmapTest, ClearBit)
{
    for (uint32_t i = 0; i < TEST_COUNT; i++)
    {
        EXPECT_TRUE(test->SetBit(i));
        EXPECT_EQ(test->GetNumBitsSet(), i + 1);
    }
    EXPECT_TRUE(test->ClearBits(0, TEST_COUNT - 1));
    EXPECT_EQ(test->GetNumBitsSet(), 0);
}

TEST_F(BitmapTest, FindFirstSetBit)
{
    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
    {
        EXPECT_TRUE(test->SetBit(i));
    }

    for (uint32_t i = 0; i < BITMAP_SIZE; i++)
    {
        EXPECT_TRUE(test->FindFirstSetBit(i) == i);
    }

    EXPECT_TRUE(test->ClearBit(BITMAP_SIZE - 1));
    EXPECT_TRUE(test->FindFirstSetBit(BITMAP_SIZE - 1) == test->GetNumBits());
}

TEST_F(BitmapTest, InvalidAccess)
{
    uint32_t bit = BITMAP_SIZE;

    EXPECT_TRUE(test->SetBit(bit) == false);
    EXPECT_TRUE(test->ClearBit(bit) == false);
    EXPECT_TRUE(test->FindFirstSetBit(bit) == test->GetNumBits());
}
