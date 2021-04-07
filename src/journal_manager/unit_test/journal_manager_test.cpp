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

#include "journal_manager_test.h"

#include <thread>

#include "src/allocator/active_stripe_index_info.h"
#include "src/io/general_io/volume_io.h"

using ::testing::AnyNumber;
using ::testing::AtLeast;
using ::testing::StrictMock;

void
JournalTest::SetUp(void)
{
    testInfo = new TestInfo();
    testMapper = new StrictMock<MockMapper>(testInfo);
    testAllocator = new StrictMock<MockAllocator>();
    testArray = new StrictMock<MockArray>(testInfo);
    journal = new JournalManagerTester(testMapper, testAllocator, testArray);

    writeTester = new LogWriteTester(testMapper, testArray, journal, testInfo);
}

void
JournalTest::InitializeJournal(bool isEnabled)
{
    journal->Delete();

    journal->SetEnable(isEnabled);
    journal->Init();

    journal->SetTriggerCheckpoint(false);

    _GetLogBufferSizeInfo();
    writeTester->Reset();
}

void
JournalTest::InitializeJournal(uint32_t logBufferSize, bool isCheckpointEnabled)
{
    journal->ReinitializeLogBuffer(logBufferSize);
    InitializeJournal();
    journal->SetTriggerCheckpoint(isCheckpointEnabled);
}

void
JournalTest::_GetLogBufferSizeInfo(void)
{
    logBufferSize = journal->GetLogBufferSize();
    logGroupSize = journal->GetLogGroupSize();
    numLogGroups = logBufferSize / logGroupSize;
}

void
JournalTest::TearDown(void)
{
    if (journal->IsCheckpointEnabled() == true)
    {
        _WaitForAllFlushDone();
    }

    writeTester->WaitForAllLogWriteDone();
    writeTester->Reset();

    delete testInfo;
    delete journal;
    delete testMapper;
    delete testAllocator;
    delete testArray;
    delete writeTester;
}

void
JournalTest::SimulateSPORWithoutRecovery(void)
{
    delete journal;

    journal = new JournalManagerTester(testMapper, testAllocator, testArray);
    writeTester->UpdateJournal(journal);

    journal->SetEnable(true);
    journal->Init();
}

void
JournalTest::_WaitForAllFlushDone(void)
{
    while (journal->GetNumFullLogGroups() != 0)
    {
    }
}

void
JournalTest::WaitForFlushingLogGroupIdle(int prevNumber)
{
    int numFullLogGroup = 0;
    while (!((numFullLogGroup = journal->GetNumFullLogGroups()) < prevNumber))
    {
    }
}

VirtualBlkAddr
JournalTest::GetNextBlock(VirtualBlks blks)
{
    VirtualBlkAddr nextVsa = blks.startVsa;
    nextVsa.offset += blks.numBlks;
    return nextVsa;
}
// Note that the number is for (block log + stripe log) written together
uint32_t
JournalTest::GetNumTestsBeforeLogGroupFull(uint32_t unitSize)
{
    uint32_t numTestsBeforeLogGroupFull = logGroupSize / unitSize;

    if (numTestsBeforeLogGroupFull > testInfo->numTest)
    {
        numTestsBeforeLogGroupFull = testInfo->numTest;
    }
    return numTestsBeforeLogGroupFull;
}

///----------------------------------------------------------------------------

TEST_F(JournalTest, JournalDisableTest)
{
    TEST_DESCRIPTION("Test journal manager behaviour with journal disabled");
    IBOF_TRACE_DEBUG(9999, "JournalTest::JournalDisableTest");

    InitializeJournal(false);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = testInfo->numTest};
    VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest};
    writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, 0, blks);

    LogList readLogs;
    int result = journal->GetLogs(readLogs);

    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(readLogs.size() == 0);
}

TEST_F(JournalTest, SingleBlockLogWrite)
{
    TEST_DESCRIPTION("Test single block log to the journal buffer");
    IBOF_TRACE_DEBUG(9999, "JournalTest::SingleBlockLogWrite");

    InitializeJournal();

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = testInfo->numTest};
    VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest};

    writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, 0, blks);
    writeTester->WaitForAllLogWriteDone();

    EXPECT_TRUE(journal->GetNumLogsAdded() == 1);

    LogList readLogs;
    int result = journal->GetLogs(readLogs);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(readLogs.size() == 1);
    EXPECT_TRUE(writeTester->CheckLogInTheList(readLogs.front()) == true);

    delete readLogs.front();
}

TEST_F(JournalTest, SingleStripeLogWrite)
{
    TEST_DESCRIPTION("Test single stripe log to the journal buffer");
    IBOF_TRACE_DEBUG(9999, "JournalTest::SingleStripeLogWrite");

    InitializeJournal();

    StripeAddr oldAddr = unmapAddr;
    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = testInfo->numTest / 10};

    writeTester->StripeMapUpdatedLogAddTester(1, oldAddr, newAddr);
    writeTester->WaitForAllLogWriteDone();
    writeTester->CompareLogs();
}

TEST_F(JournalTest, LogWriteBulk)
{
    TEST_DESCRIPTION("Test bulk log writes to the journal buffer, but not triggering checkpoint");
    IBOF_TRACE_DEBUG(9999, "JournalTest::LogWriteBulk");

    InitializeJournal();

    uint32_t logSizeInLoop = sizeof(BlockWriteDoneLog) + sizeof(StripeMapUpdatedLog);
    uint32_t numTest = GetNumTestsBeforeLogGroupFull(logSizeInLoop);

    for (uint32_t testCnt = 0; testCnt < numTest; testCnt++)
    {
        BlkAddr rba = testCnt % testInfo->numTest;
        VirtualBlkAddr vsa = {
            .stripeId = 0,
            .offset = (unsigned int)testCnt};
        VirtualBlks blks = {
            .startVsa = vsa,
            .numBlks = testInfo->numTest - testCnt};
        writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);

        StripeAddr oldAddr = unmapAddr;
        StripeAddr newAddr = {
            .stripeLoc = IN_USER_AREA,
            .stripeId = testCnt / 10};
        writeTester->StripeMapUpdatedLogAddTester(testCnt, oldAddr, newAddr);
    }

    writeTester->WaitForAllLogWriteDone();
    writeTester->CompareLogs();
}

TEST_F(JournalTest, PendLogWrite)
{
    TEST_DESCRIPTION("Test bulk log writes to the journal buffer, make log writes pended in the waiting list ");
    IBOF_TRACE_DEBUG(9999, "JournalTest::PendLogWrite");

    InitializeJournal((uint32_t)16 * 1024, false);

    uint32_t logSizeInLoop = sizeof(BlockWriteDoneLog) + sizeof(StripeMapUpdatedLog);
    int numTest = (GetNumTestsBeforeLogGroupFull(logSizeInLoop) + 1) * numLogGroups;
    for (int testCnt = 0; testCnt < numTest; testCnt++)
    {
        BlkAddr rba = testCnt % testInfo->numTest;
        VirtualBlkAddr vsa = {
            .stripeId = 0,
            .offset = (unsigned int)testCnt};
        VirtualBlks blks = {
            .startVsa = vsa,
            .numBlks = testInfo->numTest - testCnt};
        writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);

        StripeAddr newAddr = {
            .stripeLoc = IN_USER_AREA,
            .stripeId = testInfo->numTest / 10};
        writeTester->StripeMapUpdatedLogAddTester(testCnt, unmapAddr, newAddr);
    }

    EXPECT_TRUE(writeTester->DoesAllJournalWriteDone() == false);

    EXPECT_CALL(*testMapper, StartDirtyPageFlush).Times(AtLeast(1));
    journal->StartCheckpoint();

    writeTester->WaitForAllLogWriteDone();
}

TEST_F(JournalTest, MultiThreadAddLog)
{
    TEST_DESCRIPTION("Writes log with multi thread, without checkpoint trigger");
    IBOF_TRACE_DEBUG(9999, "JournalTest::MultiThreadAddLog");

    InitializeJournal();

    EXPECT_CALL(*testMapper, StartDirtyPageFlush).Times(AnyNumber());

    std::vector<std::thread> threadList;
    uint32_t logSizeInLoop = sizeof(BlockWriteDoneLog);
    uint32_t numTests = GetNumTestsBeforeLogGroupFull(logSizeInLoop);
    for (uint32_t testCnt = 0; testCnt < numTests; testCnt++)
    {
        BlkAddr rba = testCnt;
        VirtualBlkAddr vsa = {.stripeId = 0, .offset = testCnt};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest - testCnt};

        int testVol = testInfo->defaultTestVol;
        threadList.push_back(std::thread(&LogWriteTester::BlockDoneLogAddTester,
            writeTester, testVol, rba, blks));
    }

    for (auto& th : threadList)
    {
        th.join();
    }
    threadList.clear();

    writeTester->WaitForAllLogWriteDone();
    writeTester->CompareLogs();
}
