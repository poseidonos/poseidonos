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

#include "active_user_stripe_replayer_test.h"

#include <algorithm>
#include <random>
#include <vector>

#include "stripe_test_info.h"

using ::testing::AnyNumber;

void
ActiveUserStripeReplayerTest::SetUp(void)
{
    testInfo = new TestInfo();
    allocator = new MockAllocator();
    array = new MockArray(testInfo);

    userStripeReplayer = new ActiveUserStripeReplayer(allocator, array);

    EXPECT_CALL(*array, GetSizeInfo(PartitionType::USER_DATA)).Times(AnyNumber());
}

void
ActiveUserStripeReplayerTest::TearDown(void)
{
    delete testInfo;
    delete allocator;
    delete array;

    delete userStripeReplayer;
}

void
ActiveUserStripeReplayerTest::UpdateUserStripeSequentially(uint32_t beginLSID, uint32_t endLSID)
{
    for (uint32_t i = beginLSID; i <= endLSID; i++)
    {
        userStripeReplayer->Update(i);
    }
}

void
ActiveUserStripeReplayerTest::UpdateUserStripeRandomly(uint32_t beginLSID, uint32_t endLSID)
{
    std::vector<StripeId> userLsids;
    for (StripeId i = beginLSID; i <= endLSID; i++)
    {
        userLsids.push_back(i);
    }
    auto rng = std::default_random_engine{};
    std::shuffle(userLsids.begin(), userLsids.end(), rng);

    for (auto it = userLsids.begin(); it != userLsids.end(); it++)
    {
        userStripeReplayer->Update(*it);
    }
}

void
ActiveUserStripeReplayerTest::ReplayUserStripe(StripeId expectLSID)
{
    EXPECT_CALL(*allocator, ReplaySsdLsid(expectLSID));
    userStripeReplayer->Replay();
    userStripeReplayer->_Reset();
}

TEST_F(ActiveUserStripeReplayerTest, ReplaySequentialLsidInOneSegment)
{
    TEST_DESCRIPTION("Replay sequentially logged user lsid in the same segment");

    UpdateUserStripeSequentially(0, testInfo->numStripesPerSegment / 2);
    ReplayUserStripe(testInfo->numStripesPerSegment / 2);

    UpdateUserStripeSequentially(testInfo->numStripesPerSegment / 2,
        testInfo->numStripesPerSegment - 1);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);

    UpdateUserStripeSequentially(0, testInfo->numStripesPerSegment - 1);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);
}

TEST_F(ActiveUserStripeReplayerTest, ReplayRandomLsidInOneSegment)
{
    TEST_DESCRIPTION("Replay randomly logged user lsid in the same segment");

    UpdateUserStripeRandomly(0, testInfo->numStripesPerSegment / 2);
    ReplayUserStripe(testInfo->numStripesPerSegment / 2);

    UpdateUserStripeRandomly(testInfo->numStripesPerSegment / 2,
        testInfo->numStripesPerSegment - 1);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);

    UpdateUserStripeRandomly(0, testInfo->numStripesPerSegment - 1);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);
}

TEST_F(ActiveUserStripeReplayerTest, ReplaySequentialLsidInSeveralSegment)
{
    TEST_DESCRIPTION("eplay sequentially logged user lsid in multiple segments");

    uint32_t numSegment = 3;
    uint32_t lastLSID = testInfo->numStripesPerSegment * numSegment +
        testInfo->numStripesPerSegment / 2 - 1;
    UpdateUserStripeSequentially(0, lastLSID);
    ReplayUserStripe(lastLSID);

    UpdateUserStripeSequentially(testInfo->numStripesPerSegment / 2, lastLSID);
    ReplayUserStripe(lastLSID);

    lastLSID = testInfo->numStripesPerSegment * (numSegment + 1) - 1;
    UpdateUserStripeSequentially(0, lastLSID);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);

    UpdateUserStripeSequentially(testInfo->numStripesPerSegment / 2, lastLSID);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);
}

TEST_F(ActiveUserStripeReplayerTest, ReplayRandomLsidInSeveralSegment)
{
    TEST_DESCRIPTION("Replay randomly logged user lsid in multiple segments");

    uint32_t numSegment = 3;
    uint32_t lastLSID = testInfo->numStripesPerSegment * numSegment +
        testInfo->numStripesPerSegment / 2 - 1;
    UpdateUserStripeRandomly(0, lastLSID);
    ReplayUserStripe(lastLSID);

    UpdateUserStripeRandomly(testInfo->numStripesPerSegment / 2, lastLSID);
    ReplayUserStripe(lastLSID);

    lastLSID = testInfo->numStripesPerSegment * (numSegment + 1) - 1;
    UpdateUserStripeRandomly(0, lastLSID);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);

    UpdateUserStripeRandomly(testInfo->numStripesPerSegment / 2, lastLSID);
    ReplayUserStripe(testInfo->numStripesPerSegment - 1);
}

// TODO(huijeong.kim): These scenarios to checks whether ActiveUserStripeReplayer
// operates normally even if the same segment is reused after GC
// Additional replayer refactoring is required for the case to succeed.
// Therefore, these tests are currently failed
TEST_F(ActiveUserStripeReplayerTest, ReplaySequentialLsidAfterGC)
{
    TEST_DESCRIPTION("Replay sequentially logged user lsid in multiple segments, and a specific segment reused cause of GC");

    uint32_t numSegment = 3;
    uint32_t beforeGc = 10;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegment;
    UpdateUserStripeSequentially(beforeGc, numTests - beforeGc);
    UpdateUserStripeSequentially(0, beforeGc);
    EXPECT_CALL(*allocator, ReplaySsdLsid(beforeGc));
    userStripeReplayer->Replay();

    UpdateUserStripeSequentially(beforeGc,
        testInfo->numStripesPerSegment - beforeGc);
    EXPECT_CALL(*allocator, ReplaySsdLsid(testInfo->numStripesPerSegment - 1));
    userStripeReplayer->Replay();
}

TEST_F(ActiveUserStripeReplayerTest, ReplayRandomLsidAfterGC)
{
    TEST_DESCRIPTION("Replay randomly logged user lsid in multiple segments, and a specific segment reused cause of GC");

    uint32_t numSegment = 3;
    uint32_t beforeGc = 10;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegment +
        testInfo->numStripesPerSegment / 2;
    UpdateUserStripeRandomly(beforeGc, numTests);
    UpdateUserStripeRandomly(0, beforeGc);
    EXPECT_CALL(*allocator, ReplaySsdLsid(beforeGc));
    userStripeReplayer->Replay();

    UpdateUserStripeRandomly(beforeGc, testInfo->numStripesPerSegment - beforeGc);
    EXPECT_CALL(*allocator, ReplaySsdLsid(testInfo->numStripesPerSegment - 1));
    userStripeReplayer->Replay();
}
