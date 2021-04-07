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

#include "replay_segment_test.h"

#include "test_info.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::InSequence;
using ::testing::Return;

void
ReplaySegmentTest::SetUp(void)
{
    JournalTest::SetUp();
    testInfo->numStripesPerSegment = testInfo->numStripesPerSegment / 8;

    EXPECT_CALL(*testMapper, GetVSAInternal).Times(AnyNumber());
    EXPECT_CALL(*testArray, GetSizeInfo(PartitionType::USER_DATA)).Times(AnyNumber());
}

void
ReplaySegmentTest::TearDown(void)
{
    JournalTest::TearDown();
}

TEST_F(ReplaySegmentTest, ReplaySegmentsWithPatial)
{
    IBOF_TRACE_DEBUG(9999, "ReplaySegmentTest::ReplaySegmentsWithPatial");

    InitializeJournal();

    uint32_t lengthOfPatialIndex = testInfo->numStripesPerSegment / 2;
    uint32_t numSegments = 5;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments - lengthOfPatialIndex;

    std::list<StripeLog> writtenStripes;
    for (uint32_t index = lengthOfPatialIndex; index < numTests; index++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(index, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLog : writtenStripes)
    {
        auto stripe = stripeLog.stripe;
        auto blks = stripeLog.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid))
            .WillOnce(Return(unmapAddr))
            .WillOnce(Return(unmapAddr));

        _ExpectFullStripeReplay(stripe, blks);
    }

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplaySegmentTest, ReplaySegment)
{
    IBOF_TRACE_DEBUG(9999, "ReplaySegmentTest::ReplaySegment");

    InitializeJournal();

    uint32_t numSegments = 1;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments;

    std::list<StripeLog> writtenStripes;
    for (uint32_t index = 0; index < numTests; index++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(index, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLog : writtenStripes)
    {
        auto stripe = stripeLog.stripe;
        auto blks = stripeLog.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid))
            .WillOnce(Return(unmapAddr))
            .WillOnce(Return(unmapAddr));

        _ExpectFullStripeReplay(stripe, blks);
    }

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplaySegmentTest, ReplayFullSegment)
{
    IBOF_TRACE_DEBUG(9999, "ReplaySegmentTest::ReplayFullSegment");

    InitializeJournal();

    uint32_t numSegments = 3;
    uint32_t numTests = testInfo->numStripesPerSegment * numSegments;

    std::list<StripeLog> writtenStripes;
    for (uint32_t index = 0; index < numTests; index++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(index, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLog : writtenStripes)
    {
        auto stripe = stripeLog.stripe;
        auto blks = stripeLog.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid))
            .WillOnce(Return(unmapAddr))
            .WillOnce(Return(unmapAddr));

        _ExpectFullStripeReplay(stripe, blks);
    }

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplaySegmentTest, ReplayReusedSegment)
{
    IBOF_TRACE_DEBUG(9999, "ReplaySegmentTest::ReplayReusedSegment");

    InitializeJournal();

    uint32_t numTestsBeforeSegmentFull = testInfo->numStripesPerSegment;
    uint32_t numTestbeforeGC = testInfo->numStripesPerSegment / 2;

    std::list<StripeLog> writtenStripes;
    for (uint32_t i = numTestbeforeGC + 1; i < numTestsBeforeSegmentFull; i++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(i, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);
    }

    for (uint32_t i = 0; i <= numTestbeforeGC; i++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(i, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLog : writtenStripes)
    {
        auto stripe = stripeLog.stripe;
        auto blks = stripeLog.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid))
            .WillOnce(Return(unmapAddr))
            .WillOnce(Return(unmapAddr));

        _ExpectFullStripeReplay(stripe, blks);
    }

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}
