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

#include "src/allocator/allocator.h"

#include "gtest/gtest.h"
#include "src/io/frontend_io/write_submission.h"
#include "src/mapper/mapper.h"

using namespace std;
using namespace ibofos;

static const int TEST_COUNT = 130;
static const int TEST_VOL_ID = 1;

class AllocatorTest : public ::testing::Test
{
protected:
    virtual void
    SetUp()
    {
        // not expecting allocator meta will be loaded
        allocator = AllocatorSingleton::Instance();
    }

    virtual void
    TearDown()
    {
        AllocatorSingleton::ResetInstance();
    }

    VirtualBlks
    _AllocateBlksTester(int volumeId, uint32_t numBlks)
    {
        VirtualBlks blks = allocator->AllocateWriteBufferBlks(volumeId, numBlks);

        if (numBlks + blks.startVsa.offset > 128)
        {
            EXPECT_TRUE(blks.numBlks + blks.startVsa.offset == 128);
        }
        else
        {
            EXPECT_TRUE(blks.numBlks == numBlks);
        }
        return blks;
    }

    Allocator* allocator;
    const int STRIPES_PER_SEGMENT = 1024;
};

class AllocatorMetaTest : public AllocatorTest
{
protected:
    virtual void
    SetUp(void) override
    {
        allocator = AllocatorSingleton::Instance();
    }

    void
    _StoreMeta(void)
    {
        allocator->Store();
        AllocatorSingleton::ResetInstance();
    }

    void
    _LoadMeta(void)
    {
        allocator = AllocatorSingleton::Instance();
    }
};

TEST_F(AllocatorTest, AllocBlk)
{
    VirtualBlks blks = _AllocateBlksTester(TEST_VOL_ID, 130);
    allocator->InvalidateBlks(blks);

    blks = _AllocateBlksTester(TEST_VOL_ID, 3);
    allocator->InvalidateBlks(blks);
}

TEST_F(AllocatorTest, FreeWriteBufferStripe)
{
    VirtualBlks blks = _AllocateBlksTester(TEST_VOL_ID, 1);

    VirtualBlkAddr vsa;
    vsa = blks.startVsa;

    StripeAddr lsid_entry = MapperSingleton::Instance()->GetLSA(vsa.stripeId);

    allocator->InvalidateBlks(blks);
    allocator->FreeWriteBufferStripeId(lsid_entry.stripeId);
}

TEST_F(AllocatorTest, UserDataSegmentValidity)
{
    // Need WBT API for allocating user data area block
    VirtualBlks vsas = _AllocateBlksTester(TEST_VOL_ID, 1);
    LogicalBlkAddr lsa = {.stripeId = vsas.startVsa.stripeId,
        .offset = vsas.startVsa.offset};

    SegmentId seg_id = lsa.stripeId / STRIPES_PER_SEGMENT;
    EXPECT_TRUE(allocator->IsValidUserDataSegmentId(seg_id));

    // Need to convert seg_id to physical seg id
    allocator->FreeUserDataSegmentId(seg_id);
    EXPECT_FALSE(allocator->IsValidUserDataSegmentId(seg_id));
}

TEST_F(AllocatorMetaTest, CheckMetaValidity)
{
    VirtualBlks vsas = _AllocateBlksTester(TEST_VOL_ID, 1);
    LogicalBlkAddr lsa = {.stripeId = vsas.startVsa.stripeId,
        .offset = vsas.startVsa.offset};

    SegmentId seg_id = lsa.stripeId / STRIPES_PER_SEGMENT;
    EXPECT_TRUE(allocator->IsValidUserDataSegmentId(seg_id));

    _StoreMeta();
    _LoadMeta();

    EXPECT_TRUE(allocator->IsValidUserDataSegmentId(seg_id));
}
