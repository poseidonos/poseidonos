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

#include "checkpoint_test.h"

using ::testing::_;
using ::testing::AtLeast;

void
CheckpointTest::SetUp(void)
{
    JournalTest::SetUp();
}

void
CheckpointTest::TearDown(void)
{
    JournalTest::TearDown();
}

TEST_F(CheckpointTest, TriggerCheckpoint)
{
    TEST_DESCRIPTION("Writes log until all log groups are full, trigger checkpoint manually, and wait for the checkpoint to be completed");
    IBOF_TRACE_DEBUG(9999, "CheckpointTest::TriggerCheckpoint");

    InitializeJournal((uint32_t)16 * 1024, false);

    uint32_t testCnt = 0;
    int numFullLogGroups;

    while (1)
    {
        BlkAddr rba = testCnt % testInfo->numTest;
        VirtualBlkAddr vsa = {.stripeId = 0, .offset = testCnt};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest - testCnt};
        writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);

        if ((numFullLogGroups = journal->GetNumFullLogGroups()) != 0)
        {
            break;
        }

        StripeAddr oldAddr = unmapAddr;
        StripeAddr newAddr = {.stripeLoc = IN_USER_AREA,
            .stripeId = testInfo->numTest / 10};
        writeTester->StripeMapUpdatedLogAddTester(testCnt, oldAddr, newAddr);

        if ((numFullLogGroups = journal->GetNumFullLogGroups()) != 0)
        {
            break;
        }

        testCnt = (testCnt + 1) % testInfo->numTest;
    }

    MapPageList dirtyPages = writeTester->GetDirtyMap();
    EXPECT_TRUE(journal->GetNumDirtyMap(0) == static_cast<int>(dirtyPages.size()));

    for (auto it = dirtyPages.begin(); it != dirtyPages.end(); it++)
    {
        EXPECT_CALL(*testMapper, StartDirtyPageFlush(it->first, it->second, ::_)).Times(1);
    }

    journal->StartCheckpoint();

    WaitForFlushingLogGroupIdle(numFullLogGroups);
    writeTester->WaitForAllLogWriteDone();
}

TEST_F(CheckpointTest, ContinuousAddLogUntilFull)
{
    TEST_DESCRIPTION("Writes log continuously, and see if checkpoint is triggered and completed");
    IBOF_TRACE_DEBUG(9999, "CheckpointTest::ContinuousAddLogUntilFull");

    InitializeJournal((uint32_t)16 * 1024, true);

    EXPECT_CALL(*testMapper, StartDirtyPageFlush).Times(AtLeast(1));

    int testCnt = 0;
    int numFullLogGroups;
    while (1)
    {
        BlkAddr rba = testCnt % testInfo->numTest;
        VirtualBlkAddr vsa = {.stripeId = 0, .offset = (unsigned int)testCnt};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest - testCnt};
        writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);

        if ((numFullLogGroups = journal->GetNumFullLogGroups()) != 0)
        {
            break;
        }

        testCnt = (testCnt + 1) % testInfo->numTest;
    }

    WaitForFlushingLogGroupIdle(numFullLogGroups);
    writeTester->WaitForAllLogWriteDone();
}
