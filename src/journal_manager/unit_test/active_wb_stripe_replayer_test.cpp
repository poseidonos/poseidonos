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

#include "active_wb_stripe_replayer_test.h"

#include <algorithm>
#include <random>
#include <vector>

#include "test_info.h"

void
ActiveWBStripeReplayerTest::SetUp(void)
{
    testInfo = new TestInfo();
    allocator = new MockAllocator();
    wbStripeReplayer = new ActiveWBStripeReplayer(allocator, pendingStripes);
}

void
ActiveWBStripeReplayerTest::TearDown(void)
{
    delete testInfo;
    for (auto stripe : pendingStripes)
    {
        delete stripe;
    }

    delete allocator;
    delete wbStripeReplayer;
}

StripeInfo
ActiveWBStripeReplayerTest::_GetActiveStripe(int volumeId, int wbufIndex)
{
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeInfo activeStripe(vsid);

    activeStripe.UpdateVolumeId(volumeId);
    activeStripe.UpdateWbIndex(wbufIndex);
    activeStripe.UpdateUserLsid(vsid);
    activeStripe.UpdateWbLsid(std::rand() % testInfo->numWbStripes);
    activeStripe.UpdateLastOffset(std::rand() % (testInfo->numBlksPerStripe - 1));

    return activeStripe;
}

StripeInfo
ActiveWBStripeReplayerTest::_GetFlushedActiveStripe(int volumeId, int wbufIndex)
{
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeInfo activeStripe(vsid);

    activeStripe.UpdateVolumeId(volumeId);
    activeStripe.UpdateWbIndex(wbufIndex);
    activeStripe.UpdateUserLsid(vsid);
    activeStripe.UpdateWbLsid(std::rand() % testInfo->numWbStripes);
    activeStripe.ResetOffset();

    return activeStripe;
}

VirtualBlkAddr
ActiveWBStripeReplayerTest::_GetTail(StripeInfo stripe)
{
    VirtualBlkAddr tail = {
        .stripeId = stripe.GetVsid(),
        .offset = stripe.GetLastOffset() + 1};

    return tail;
}

TEST_F(ActiveWBStripeReplayerTest, SingleActiveStripe)
{
    TEST_DESCRIPTION("Replay single active stripe");

    StripeInfo activeStripe = _GetActiveStripe(testInfo->defaultTestVol,
        testInfo->defaultTestVol);

    wbStripeReplayer->Update(activeStripe);

    VirtualBlkAddr tail = _GetTail(activeStripe);
    EXPECT_CALL(*allocator, RestoreActiveStripeTail(activeStripe.GetWbIndex(), tail, activeStripe.GetWbLsid())).Times(1);

    wbStripeReplayer->Replay();
}

TEST_F(ActiveWBStripeReplayerTest, SingleFullActiveStripe)
{
    TEST_DESCRIPTION("Replay single full active stripe");

    StripeInfo fullActiveStripe = _GetFlushedActiveStripe(testInfo->defaultTestVol,
        testInfo->defaultTestVol);

    wbStripeReplayer->Update(fullActiveStripe);

    EXPECT_CALL(*allocator, ResetActiveStripeTail(fullActiveStripe.GetWbIndex())).Times(1);

    wbStripeReplayer->Replay();
}

TEST_F(ActiveWBStripeReplayerTest, SingleVolumeMultipleStripe)
{
    TEST_DESCRIPTION("Replay active stripes in same wbuf index, and find latest");

    for (int stripe = 0; stripe < 5; stripe++)
    {
        StripeInfo fullStripe = _GetFlushedActiveStripe(testInfo->defaultTestVol,
            testInfo->defaultTestVol);

        wbStripeReplayer->Update(fullStripe);
    }

    StripeInfo lastActiveStripe = _GetActiveStripe(testInfo->defaultTestVol,
        testInfo->defaultTestVol);
    wbStripeReplayer->Update(lastActiveStripe);

    VirtualBlkAddr tail = _GetTail(lastActiveStripe);
    EXPECT_CALL(*allocator, RestoreActiveStripeTail(lastActiveStripe.GetWbIndex(), tail, lastActiveStripe.GetWbLsid())).Times(1);

    wbStripeReplayer->Replay();
}

TEST_F(ActiveWBStripeReplayerTest, MultiVolumeMultipleStripe)
{
    TEST_DESCRIPTION("Replay active stripes in same wbuf index, and find latest");

    std::vector<StripeInfo> stripes;
    for (int volumeId = 1; volumeId < 4; volumeId++)
    {
        for (int stripe = 0; stripe < 5; stripe++)
        {
            StripeInfo fullStripe = _GetFlushedActiveStripe(volumeId, volumeId);
            stripes.push_back(fullStripe);
        }

        StripeInfo lastActiveStripe = _GetActiveStripe(volumeId, volumeId);
        stripes.push_back(lastActiveStripe);

        VirtualBlkAddr tail = _GetTail(lastActiveStripe);
        EXPECT_CALL(*allocator, RestoreActiveStripeTail(lastActiveStripe.GetWbIndex(), tail, lastActiveStripe.GetWbLsid())).Times(1);
    }

    auto rng = std::default_random_engine{};
    std::shuffle(stripes.begin(), stripes.end(), rng);

    for (auto it = stripes.begin(); it != stripes.end(); ++it)
    {
        wbStripeReplayer->Update(*it);
    }

    wbStripeReplayer->Replay();
}

TEST_F(ActiveWBStripeReplayerTest, SingleVolumeMultipleActiveStripe)
{
    TEST_DESCRIPTION("Replay active stripes with several orphan stripes");

    std::vector<StripeInfo> orphanStripes;
    for (int stripe = 0; stripe < 5; stripe++)
    {
        StripeInfo orphanStripe = _GetActiveStripe(testInfo->defaultTestVol,
            testInfo->defaultTestVol);

        wbStripeReplayer->Update(orphanStripe);
        orphanStripes.push_back(orphanStripe);

        StripeInfo fullStripe = _GetFlushedActiveStripe(testInfo->defaultTestVol,
            testInfo->defaultTestVol);
        wbStripeReplayer->Update(fullStripe);
    }

    StripeInfo lastStripe = _GetActiveStripe(testInfo->defaultTestVol,
        testInfo->defaultTestVol);
    wbStripeReplayer->Update(lastStripe);

    EXPECT_CALL(*allocator, RestoreActiveStripeTail(lastStripe.GetWbIndex(), _GetTail(lastStripe), lastStripe.GetWbLsid())).Times(1);

    wbStripeReplayer->Replay();

    EXPECT_TRUE(orphanStripes.size() == pendingStripes.size());

    std::sort(orphanStripes.begin(), orphanStripes.end(), [](StripeInfo a, StripeInfo b) {
        return (a.GetWbLsid() < b.GetWbLsid());
    });

    std::sort(pendingStripes.begin(), pendingStripes.end(), [](PendingStripe* a, PendingStripe* b) {
        return (a->wbLsid < b->wbLsid);
    });

    for (uint32_t index = 0; index < orphanStripes.size(); index++)
    {
        EXPECT_TRUE(orphanStripes[index].GetWbLsid() == pendingStripes[index]->wbLsid);
        EXPECT_TRUE(_GetTail(orphanStripes[index]) == pendingStripes[index]->tailVsa);
    }
}

TEST_F(ActiveWBStripeReplayerTest, MultiVolumeMultipleActiveStripe)
{
    TEST_DESCRIPTION("Replay active stripes of multiple volumes with several orphan stripes");

    std::vector<StripeInfo> orphanStripes;
    for (int volumeId = 0; volumeId < 5; volumeId++)
    {
        for (int stripe = 0; stripe < 5; stripe++)
        {
            StripeInfo orphanStripe = _GetActiveStripe(volumeId, volumeId);
            wbStripeReplayer->Update(orphanStripe);
            orphanStripes.push_back(orphanStripe);

            StripeInfo fullStripe = _GetFlushedActiveStripe(volumeId, volumeId);
            wbStripeReplayer->Update(fullStripe);
        }

        StripeInfo lastStripe = _GetActiveStripe(volumeId, volumeId);
        wbStripeReplayer->Update(lastStripe);

        EXPECT_CALL(*allocator, RestoreActiveStripeTail(lastStripe.GetWbIndex(), _GetTail(lastStripe), lastStripe.GetWbLsid())).Times(1);
    }

    wbStripeReplayer->Replay();

    EXPECT_TRUE(orphanStripes.size() == pendingStripes.size());

    std::sort(orphanStripes.begin(), orphanStripes.end(), [](StripeInfo a, StripeInfo b) {
        return (a.GetWbLsid() < b.GetWbLsid());
    });

    std::sort(pendingStripes.begin(), pendingStripes.end(), [](PendingStripe* a, PendingStripe* b) {
        return (a->wbLsid < b->wbLsid);
    });

    for (uint32_t index = 0; index < orphanStripes.size(); index++)
    {
        EXPECT_TRUE(orphanStripes[index].GetWbLsid() == pendingStripes[index]->wbLsid);
        EXPECT_TRUE(_GetTail(orphanStripes[index]) == pendingStripes[index]->tailVsa);
    }
}
