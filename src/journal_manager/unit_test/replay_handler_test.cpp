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

#include "replay_handler_test.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::InSequence;
using ::testing::Return;

void
ReplayTest::SetUp(void)
{
    JournalTest::SetUp();
    EXPECT_CALL(*testMapper, GetVSAInternal).Times(AnyNumber());
    EXPECT_CALL(*testArray, GetSizeInfo(PartitionType::USER_DATA)).Times(AnyNumber());
}

void
ReplayTest::TearDown(void)
{
    JournalTest::TearDown();
}

void
ReplayTest::_ExpectStripeAllocationReplay(StripeId vsid, StripeId wbLsid)
{
    if (vsid % testInfo->numStripesPerSegment == 0)
    {
        EXPECT_CALL(*testAllocator, ReplaySegmentAllocation(vsid)).Times(1);
    }
    EXPECT_CALL(*testMapper, UpdateStripeMap(vsid, wbLsid, IN_WRITE_BUFFER_AREA)).Times(1);
    EXPECT_CALL(*testAllocator, ReplayStripeAllocation(vsid, wbLsid)).Times(1);
}

void
ReplayTest::_ExpectStripeBlockLogReplay(int volId, BlockMapList blksToWrite)
{
    InSequence s;
    for (auto blk : blksToWrite)
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks blks = std::get<1>(blk);

        for (uint32_t offset = 0; offset < blks.numBlks; offset++)
        {
            VirtualBlks blk = _GetBlock(blks, offset);
            EXPECT_CALL(*testMapper, SetVsaMapInternal(volId, rba + offset, blk));
        }
    }
}

VirtualBlks
ReplayTest::_GetBlock(VirtualBlks blks, uint32_t offset)
{
    VirtualBlks blk = {
        .startVsa = {
            .stripeId = blks.startVsa.stripeId,
            .offset = blks.startVsa.offset + offset},
        .numBlks = 1};
    return blk;
}

void
ReplayTest::_ExpectStripeFlushReplay(StripeWriteInfo stripe)
{
    EXPECT_CALL(*testMapper, UpdateStripeMap(stripe.vsid, stripe.userAddr.stripeId, stripe.userAddr.stripeLoc)).Times(1);
    EXPECT_CALL(*testAllocator, ReplayStripeFlushed(stripe.wbAddr.stripeId)).Times(1);
    EXPECT_CALL(*testAllocator, TryToUpdateSegmentValidBlks(stripe.userAddr.stripeId)).Times(1);
}

void
ReplayTest::_ExpectFullStripeReplay(StripeWriteInfo stripe, BlockMapList blksToWrite)
{
    InSequence s;

    _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
    _ExpectStripeBlockLogReplay(stripe.volId, blksToWrite);
    _ExpectStripeFlushReplay(stripe);
}

void
ReplayTest::_ExpectOverwritedBlocksReplay(StripeWriteInfo stripe, BlockMapList writtenVsas)
{
    _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
    _ExpectStripeBlockLogReplay(stripe.volId, writtenVsas);

    for (auto vsa = writtenVsas.begin(); vsa != writtenVsas.end() - 1; vsa++)
    {
        for (uint32_t blockOffset = 0; blockOffset < (*vsa).second.numBlks;
             blockOffset++)
        {
            VirtualBlks blks = _GetBlock((*vsa).second, blockOffset);
            EXPECT_CALL(*testAllocator, InvalidateBlks(blks));
        }
    }
}

void
ReplayTest::_ExpectActiveStripeReplay(VirtualBlkAddr tail, StripeWriteInfo stripe)
{
    EXPECT_CALL(*testAllocator, RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.wbAddr.stripeId)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);
}

///----------------------------------------------------------------------------

TEST_F(ReplayTest, ParseLogBuffer)
{
    TEST_DESCRIPTION("Add logs until it's full with checkpoint disabled, simulate power off, and see if same logs are found");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReadLogBufferAfterPOR");

    InitializeJournal();

    uint32_t logSizeInLoop = sizeof(BlockWriteDoneLog) + sizeof(StripeMapUpdatedLog);
    uint32_t numTests = GetNumTestsBeforeLogGroupFull(logSizeInLoop);

    for (uint32_t testCnt = 0; testCnt < numTests; testCnt++)
    {
        BlkAddr rba = testCnt;
        VirtualBlkAddr vsa = {.stripeId = 0, .offset = testCnt};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = testInfo->numTest - testCnt};
        writeTester->BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);

        StripeAddr newAddr = {.stripeLoc = IN_USER_AREA,
            .stripeId = testInfo->numTest / 10};

        writeTester->StripeMapUpdatedLogAddTester(testCnt, unmapAddr, newAddr);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    writeTester->CompareLogs();
}

TEST_F(ReplayTest, ReplayOverwrite)
{
    TEST_DESCRIPTION("Add logs for same rba to several volumes, simulate SPOR, and see if it's recovered with latest value");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayOverwrite");

    InitializeJournal();

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    uint32_t numBlksToOverwrite = std::rand() % testInfo->numBlksPerStripe;

    StripeWriteInfo stripe(vsid, testInfo->defaultTestVol);
    BlockMapList writtenVsas;

    writtenVsas = writeTester->OverwriteBlockDoneLogAddTester(stripe, rba, 0, numBlksToOverwrite);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));
    _ExpectOverwritedBlocksReplay(stripe, writtenVsas);

    VirtualBlkAddr tail = GetNextBlock(writtenVsas.back().second);
    _ExpectActiveStripeReplay(tail, stripe);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayFullStripe)
{
    TEST_DESCRIPTION("Add logs for one stripe write,simulate SPOR, and see if all stripe is recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayFullStripe");

    InitializeJournal();

    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeLog logInfo = writeTester->AddLogForStripe(vsid, testInfo->defaultTestVol);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(logInfo.stripe.vsid))
        .WillOnce(Return(unmapAddr))
        .WillOnce(Return(unmapAddr));

    _ExpectFullStripeReplay(logInfo.stripe, logInfo.blks);

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayFullStripeSeveralTimes)
{
    TEST_DESCRIPTION("Add logs for stripe write until buffer is full, simulate SPOR, and see if all stripe is recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayFullStripeSeveralTimes");

    InitializeJournal();

    std::list<StripeLog> writtenStripes;

    uint32_t writtenLogSize = 0;
    uint32_t currentVsid = 0;
    while (writtenLogSize < logGroupSize / 1024)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(currentVsid++, testInfo->defaultTestVol);
        writtenStripes.push_back(logInfo);

        writtenLogSize += (logInfo.blks.size() * sizeof(BlockWriteDoneLog));
        writtenLogSize += sizeof(StripeMapUpdatedLog);
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

TEST_F(ReplayTest, ReplayeSeveralUnflushedStripe)
{
    TEST_DESCRIPTION("Add several unflushed stripe on same volume simulate SPOR, and see if all stripe is recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayeSeveralUnflushedStripe");

    InitializeJournal();

    StripeWriteInfo partialStripe(0, testInfo->defaultTestVol);
    int startOffset = std::rand() % testInfo->numBlksPerStripe;
    BlockMapList blksToWrite = writeTester->StripeBlockDoneLogAddTester(partialStripe,
        startOffset, testInfo->numBlksPerStripe - startOffset);
    StripeLog partialUnflushedStripe(partialStripe, blksToWrite);

    StripeWriteInfo fullStripe(1, testInfo->defaultTestVol);
    blksToWrite = writeTester->StripeBlockDoneLogAddTester(fullStripe, 0,
        testInfo->numBlksPerStripe);
    StripeLog fullUnflushedStripe(fullStripe, blksToWrite);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;
        auto stripe = partialUnflushedStripe.stripe;
        auto block = partialUnflushedStripe.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));

        _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
        _ExpectStripeBlockLogReplay(stripe.volId, block);

        VirtualBlkAddr tailVsa = GetNextBlock(partialUnflushedStripe.blks.back().second);
        EXPECT_CALL(*testAllocator, FlushStripe(testInfo->defaultTestVol, stripe.wbAddr.stripeId, tailVsa));
    }

    {
        InSequence s;
        auto stripe = fullUnflushedStripe.stripe;
        auto block = fullUnflushedStripe.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));

        _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
        _ExpectStripeBlockLogReplay(stripe.volId, block);

        VirtualBlkAddr tail = GetNextBlock(fullUnflushedStripe.blks.back().second);
        EXPECT_CALL(*testAllocator, RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.wbAddr.stripeId)).Times(1);
    }

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayBlockWritesFromStart)
{
    TEST_DESCRIPTION("Add logs for block writes from offset 0, simulate SPOR, and see if all logs are properly recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayBlockWritesFromStart");

    InitializeJournal();

    StripeWriteInfo stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    BlockMapList blksToWrite = writeTester->StripeBlockDoneLogAddTester(stripe, 0, numBlks);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));

        _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
        _ExpectStripeBlockLogReplay(stripe.volId, blksToWrite);
    }

    VirtualBlkAddr tail = GetNextBlock(blksToWrite.back().second);
    EXPECT_CALL(*testAllocator, RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.wbAddr.stripeId)).Times(1);

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayBlockWrites)
{
    TEST_DESCRIPTION("Add logs for block writes from offset not zero, simulate SPOR, and see if all logs are properly recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayBlockWrites");

    InitializeJournal();

    StripeWriteInfo stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    BlockMapList blksToWrite = writeTester->StripeBlockDoneLogAddTester(stripe,
        testInfo->numBlksPerStripe - numBlks, numBlks);
    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));

    _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
    _ExpectStripeBlockLogReplay(stripe.volId, blksToWrite);

    VirtualBlkAddr tail = GetNextBlock(blksToWrite.back().second);

    EXPECT_CALL(*testAllocator, RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.wbAddr.stripeId)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayBlockWritesFromStartToEnd)
{
    TEST_DESCRIPTION("Add logs for block writes from offset 0 to end of the stripe, simulate SPOR, and see if all logs are properly recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayBlockWritesFromStartToEnd");

    InitializeJournal();

    StripeWriteInfo stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    BlockMapList blksToWrite = writeTester->StripeBlockDoneLogAddTester(stripe, 0, testInfo->numBlksPerStripe);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));

        _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
        _ExpectStripeBlockLogReplay(stripe.volId, blksToWrite);
    }

    VirtualBlkAddr tail = {
        .stripeId = stripe.vsid,
        .offset = testInfo->numBlksPerStripe};
    EXPECT_CALL(*testAllocator, RestoreActiveStripeTail(testInfo->defaultTestVol, tail, stripe.wbAddr.stripeId)).Times(1);

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayBlockWritesAndFlush)
{
    TEST_DESCRIPTION("Add logs for block writes and flush,simulate SPOR, and see if all stripe is recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayBlockWritesAndFlush");

    InitializeJournal();

    StripeWriteInfo stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    int numBlks = std::rand() % (testInfo->numBlksPerStripe - 1) + 1;
    uint32_t startOffset = testInfo->numBlksPerStripe - numBlks;
    BlockMapList blksToWrite = writeTester->StripeBlockDoneLogAddTester(stripe, startOffset, numBlks);
    writeTester->StripeMapUpdatedLogAddTester(stripe.vsid, stripe.wbAddr, stripe.userAddr);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    {
        InSequence s;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillRepeatedly(Return(stripe.wbAddr));
        _ExpectStripeBlockLogReplay(stripe.volId, blksToWrite);
        _ExpectStripeFlushReplay(stripe);
    }

    EXPECT_CALL(*testAllocator, ResetActiveStripeTail(testInfo->defaultTestVol)).Times(1);
    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayFlush)
{
    TEST_DESCRIPTION("Add logs for flush,simulate SPOR, and see if the stripe flush event is recovered");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayFlushButMapNotUpdated");

    InitializeJournal();

    StripeWriteInfo stripe(std::rand() % testInfo->numUserStripes, testInfo->defaultTestVol);
    writeTester->StripeMapUpdatedLogAddTester(stripe.vsid, stripe.wbAddr, stripe.userAddr);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(stripe.wbAddr));
    _ExpectStripeFlushReplay(stripe);

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayFullLogBuffer)
{
    TEST_DESCRIPTION("Add logs for same rba until log buffer is full without checkpoint, simulate SPOR, and check if log groups are replayed in sequential order");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplaySeveralLogGroup");

    InitializeJournal((uint32_t)16 * 1024, false);

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    StripeWriteInfo stripe(vsid, testInfo->defaultTestVol);
    BlockMapList writtenVsas;

    uint32_t numTests = logGroupSize / sizeof(BlockWriteDoneLog) * numLogGroups;
    writtenVsas = writeTester->OverwriteBlockDoneLogAddTester(stripe, rba, 0, numTests);

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));
    _ExpectOverwritedBlocksReplay(stripe, writtenVsas);

    VirtualBlkAddr tail = GetNextBlock(writtenVsas.back().second);
    _ExpectActiveStripeReplay(tail, stripe);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(ReplayTest, ReplayCirculatedLogBuffer)
{
    TEST_DESCRIPTION("Add logs for same rba until log buffer is overflowed with checkpoint, simulate SPOR, and check whether the overwritten log groups are replayed in sequential order");
    IBOF_TRACE_DEBUG(9999, "ReplayTest::ReplayOverwrittenLogGroups");

    InitializeJournal((uint32_t)16 * 1024, false);

    BlkAddr rba = std::rand() % testInfo->defaultTestVolSizeInBlock;
    StripeId vsid = std::rand() % testInfo->numUserStripes;
    BlockMapList writtenVsas;
    StripeWriteInfo stripe(vsid, testInfo->defaultTestVol);

    uint32_t numCheckpointedLogGroups = 1;
    uint32_t numPendingLogGroups = numLogGroups;
    uint32_t numTests = logGroupSize / sizeof(BlockWriteDoneLog) * numCheckpointedLogGroups;
    writtenVsas = writeTester->OverwriteBlockDoneLogAddTester(stripe, rba, 0, numTests + 1);
    auto lastVsa = writtenVsas.back();

    writeTester->WaitForAllLogWriteDone();

    MapPageList dirtyPages = writeTester->GetDirtyMap();
    EXPECT_TRUE(dirtyPages.size() == 1);
    EXPECT_TRUE(dirtyPages.begin()->first == testInfo->defaultTestVol);
    EXPECT_CALL(*testMapper, StartDirtyPageFlush(dirtyPages.begin()->first, dirtyPages.begin()->second, ::_)).Times(1);

    journal->StartCheckpoint();
    WaitForFlushingLogGroupIdle(numCheckpointedLogGroups);

    numTests *= numPendingLogGroups;
    writtenVsas = writeTester->OverwriteBlockDoneLogAddTester(
        stripe, rba, lastVsa.second.startVsa.offset + 1, numTests - 1);
    writtenVsas.insert(writtenVsas.begin(), lastVsa);
    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillOnce(Return(unmapAddr));
    _ExpectOverwritedBlocksReplay(stripe, writtenVsas);

    VirtualBlkAddr tail = GetNextBlock(writtenVsas.back().second);
    _ExpectActiveStripeReplay(tail, stripe);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}
