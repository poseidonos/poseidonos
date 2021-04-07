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

#include "log_write_test.h"

#include "test_journal_write_completion.h"

LogWriteTester::LogWriteTester(MockMapper* _mapper, MockArray* _array, JournalManagerTester* _journal, TestInfo* _testInfo)
{
    Reset();

    mapper = _mapper;
    array = _array;
    journal = _journal;
    testInfo = _testInfo;
    rbaGenerator = new RbaGenerator(testInfo);
}

LogWriteTester::~LogWriteTester(void)
{
    Reset();
    delete rbaGenerator;
}

void
LogWriteTester::Reset(void)
{
    testingLogs.Reset();
    dirtyPages.clear();
}

void
LogWriteTester::UpdateJournal(JournalManagerTester* _journal)
{
    journal = _journal;
}

bool
LogWriteTester::BlockDoneLogAddTester(int volId, BlkAddr rba, VirtualBlks blks)
{
    MpageList dirty = mapper->GetVsaMapDirtyPages(volId, rba, blks.numBlks);
    uint32_t numBlksInSector = blks.numBlks * SECTORS_PER_BLOCK;
    StripeAddr stripeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = GetWbLsid(blks.startVsa.stripeId)};

    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, numBlksInSector));
    volumeIo->SetRba(ChangeBlockToSector(rba));
    volumeIo->SetVolumeId(volId);
    volumeIo->SetVsa(blks.startVsa);
    volumeIo->SetLsidEntry(stripeAddr);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    bool ret = journal->AddBlockMapUpdatedLog(volumeIo, dirty, event);
    if (ret == true)
    {
        testingLogs.AddToWriteList(volumeIo);
        _AddToDirtyPageList(volId, dirty);
    }
    else
    {
        cout << "Log write failed" << endl;
    }

    return ret;
}

bool
LogWriteTester::StripeMapUpdatedLogAddTester(StripeId vsid, StripeAddr oldAddr, StripeAddr newAddr)
{
    assert(newAddr.stripeLoc == IN_USER_AREA);

    MpageList dirty = mapper->GetStripeMapDirtyPages(vsid);

    Stripe* stripe = new Stripe();
    stripe->SetVsid(vsid);
    stripe->SetUserLsid(newAddr.stripeId);

    EventSmartPtr event(new TestJournalWriteCompletion(&testingLogs));
    bool ret = journal->AddStripeMapUpdatedLog(stripe, oldAddr, dirty, event);

    if (ret == true)
    {
        testingLogs.AddToWriteList(stripe, oldAddr);
        _AddToDirtyPageList(STRIPE_MAP_ID, dirty);
    }
    else
    {
        cout << "Log write failed " << endl;
        delete stripe;
    }

    return ret;
}

void
LogWriteTester::_AddToDirtyPageList(int mapId, MpageList dirty)
{
    if (dirtyPages.find(mapId) == dirtyPages.end())
    {
        dirtyPages.emplace(mapId, dirty);
    }
    else
    {
        dirtyPages[mapId].insert(dirty.begin(), dirty.end());
    }
}

StripeLog
LogWriteTester::AddLogForStripe(StripeId vsid, int volId)
{
    StripeWriteInfo stripe(vsid, volId);
    BlockMapList blksWritten = StripeBlockDoneLogAddTester(stripe, 0, testInfo->numBlksPerStripe);
    StripeMapUpdatedLogAddTester(stripe.vsid, stripe.wbAddr, stripe.userAddr);
    return StripeLog(stripe, blksWritten);
}

void
LogWriteTester::WaitForAllLogWriteDone(void)
{
    testingLogs.WaitForAllLogWriteDone();
}

BlockMapList
LogWriteTester::_GenerateBlocksInStripe(StripeId vsid, uint32_t startOffset, int numBlks)
{
    BlockMapList listToReturn;

    int blksRemaining = numBlks;
    VirtualBlkAddr vsa = {.stripeId = vsid, .offset = startOffset};

    while (blksRemaining != 0)
    {
        vsa.offset = numBlks - blksRemaining + startOffset;

        uint32_t blksToWrite;
        if (blksRemaining == 1)
        {
            blksToWrite = 1;
        }
        else
        {
            blksToWrite = blksRemaining / 2;
        }

        VirtualBlks blks = {.startVsa = vsa, .numBlks = blksToWrite};

        BlkAddr rba = rbaGenerator->Generate(blksToWrite);
        listToReturn.push_back(std::make_pair(rba, blks));
        blksRemaining -= blksToWrite;
    }

    return listToReturn;
}

BlockMapList
LogWriteTester::OverwriteBlockDoneLogAddTester(StripeWriteInfo stripe,
    BlkAddr rba,
    uint32_t startOffset,
    uint32_t numTest)
{
    BlockMapList writtenVsas;
    for (uint32_t testCnt = 0; testCnt < numTest; testCnt++)
    {
        VirtualBlkAddr vsa = {.stripeId = stripe.vsid, .offset = startOffset + testCnt};
        VirtualBlks blks = {.startVsa = vsa, .numBlks = 1};
        BlockDoneLogAddTester(testInfo->defaultTestVol, rba, blks);
        writtenVsas.push_back(make_pair(rba, blks));
    }
    return writtenVsas;
}

BlockMapList
LogWriteTester::StripeBlockDoneLogAddTester(StripeWriteInfo stripe, uint32_t startOffset, int numBlks)
{
    auto blksToWrite = _GenerateBlocksInStripe(stripe.vsid, startOffset, numBlks);

    for (auto blk : blksToWrite)
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks virtualBlks = std::get<1>(blk);
        BlockDoneLogAddTester(stripe.volId, rba, virtualBlks);
    }

    return blksToWrite;
}

bool
LogWriteTester::DoesAllJournalWriteDone(void)
{
    return testingLogs.DoesAllJournalWriteDone();
}
bool
LogWriteTester::CheckLogInTheList(LogHandlerInterface* log)
{
    return testingLogs.CheckLogInTheList(log);
}

void
LogWriteTester::CompareLogs(void)
{
    LogList readLogs;
    int result = journal->GetLogs(readLogs);
    EXPECT_TRUE(result == 0);

    EXPECT_TRUE(testingLogs.GetNumLogsInTesting() == readLogs.size());

    while (readLogs.size() != 0)
    {
        LogHandlerInterface* log = readLogs.front();
        EXPECT_TRUE(testingLogs.CheckLogInTheList(log) == true);
        readLogs.pop_front();
        delete log;
    }

    EXPECT_TRUE(readLogs.size() == 0);
}

MapPageList
LogWriteTester::GetDirtyMap(void)
{
    return dirtyPages;
}
