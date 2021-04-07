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

#include "journal_volume_test.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::StrictMock;

void
JournalVolumeTest::SetUp(void)
{
    ReplayTest::SetUp();
}

void
JournalVolumeTest::TearDown(void)
{
    ReplayTest::TearDown();
}

vector<StripeLog>
JournalVolumeTest::_VolumeDeleteTester(int numVolumes, unordered_set<int> volumesToDelete)
{
    std::vector<StripeLog> writtenStripes(numVolumes);
    StripeId currentVsid = std::rand() % testInfo->numUserStripes;

    for (int vol = 0; vol < numVolumes; vol++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(currentVsid++, vol);
        writtenStripes[vol] = logInfo;
    }

    writeTester->WaitForAllLogWriteDone();

    for (auto volId : volumesToDelete)
    {
        EXPECT_TRUE(journal->VolumeDeleted(volId) == true);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    LogList logs;
    int result = journal->GetLogs(logs);
    EXPECT_TRUE(result == 0);

    int deleteVolumeLogFound = 0;
    while (logs.size() != 0)
    {
        LogHandlerInterface* log = logs.front();

        if (log->GetType() == LogType::VOLUME_DELETED)
        {
            deleteVolumeLogFound++;

            VolumeDeletedLog* logData = reinterpret_cast<VolumeDeletedLog*>(log->GetData());
            EXPECT_TRUE(volumesToDelete.find(logData->volId) != volumesToDelete.end());
        }

        delete log;
        logs.pop_front();
    }

    EXPECT_TRUE(deleteVolumeLogFound == (int)volumesToDelete.size());

    writeTester->Reset();

    return writtenStripes;
}

void
JournalVolumeTest::_VolumeDeleteReplayTester(vector<StripeLog> writtenStripes,
    int numVolumes, unordered_set<int> volumesToDelete)
{
    for (int volId = 0; volId < numVolumes; volId++)
    {
        auto stripe = writtenStripes[volId].stripe;
        auto blks = writtenStripes[volId].blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillRepeatedly(Return(unmapAddr));
        _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);

        if (volumesToDelete.find(volId) == volumesToDelete.end())
        {
            InSequence s;
            _ExpectStripeBlockLogReplay(volId, blks);
        }
        _ExpectStripeFlushReplay(stripe);

        EXPECT_CALL(*testAllocator, ResetActiveStripeTail(volId)).Times(1);
    }

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(JournalVolumeTest, JournalDisableTest)
{
    TEST_DESCRIPTION("Test journal manager volume event behaviour with journal disabled");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::JournalDisableTest");

    InitializeJournal(false);

    EXPECT_TRUE(journal->VolumeDeleted(testInfo->defaultTestVol) == true);

    LogList readLogs;
    int result = journal->GetLogs(readLogs);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(readLogs.size() == 0);
}

TEST_F(JournalVolumeTest, AddVolumeDeletedLog)
{
    TEST_DESCRIPTION("Test journal write to several volumes, and delete one of them");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::AddVolumeDeletedLog");

    InitializeJournal();

    int numVolumes = testInfo->maxNumVolume;

    unordered_set<int> volumesToDelete;
    volumesToDelete.insert(numVolumes / 2);

    vector<StripeLog> writtenStripes = _VolumeDeleteTester(numVolumes, volumesToDelete);
    EXPECT_TRUE((int)writtenStripes.size() == numVolumes);
    _VolumeDeleteReplayTester(writtenStripes, numVolumes, volumesToDelete);
}

TEST_F(JournalVolumeTest, AddVolumeDeletedLogSeveralTimes)
{
    TEST_DESCRIPTION("Test journal write to several volumes, and delete severals of them");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::AddVolumeDeletedLogSeveralTimes");

    InitializeJournal();

    int numVolumes = testInfo->maxNumVolume;
    unordered_set<int> volumesToDelete;
    volumesToDelete.insert(1);
    volumesToDelete.insert(5);

    vector<StripeLog> writtenStripes = _VolumeDeleteTester(numVolumes, volumesToDelete);
    EXPECT_TRUE((int)writtenStripes.size() == numVolumes);
    _VolumeDeleteReplayTester(writtenStripes, numVolumes, volumesToDelete);
}

TEST_F(JournalVolumeTest, ReplayAfterVolumeDeleted)
{
    TEST_DESCRIPTION("Test journal write to several volumes, and delete one of them, Deleted volume has full stripe only");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::ReplayAfterVolumeDeleted");

    InitializeJournal();

    int numVolumes = testInfo->maxNumVolume;
    unordered_set<int> volumesToDelete;

    volumesToDelete.insert(3);
    volumesToDelete.insert(0);

    vector<StripeLog> writtenStripes = _VolumeDeleteTester(numVolumes, volumesToDelete);
    EXPECT_TRUE((int)writtenStripes.size() == numVolumes);
    _VolumeDeleteReplayTester(writtenStripes, numVolumes, volumesToDelete);
}

// TODO(huijeong.kim): simplify the tc
TEST_F(JournalVolumeTest, VolumeDeleteAndCreate)
{
    TEST_DESCRIPTION("Test journal write to several volumes, and delete one of them, create the volume with same id, write another pattern to the volumes, and verify after simulating SPOR");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::VolumeDeleteAndCreate");

    InitializeJournal();

    int numTest = 2;
    std::list<StripeLog> writtenStripes;

    int volumeToDelete = 1;
    StripeId deletedVsid = UNMAP_STRIPE;

    int currentVsid = std::rand() % testInfo->numUserStripes;
    for (int vol = 0; vol < numTest; vol++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(currentVsid++, vol);

        if (volumeToDelete == vol)
        {
            deletedVsid = logInfo.stripe.vsid;
        }
        writtenStripes.push_back(logInfo);
    }

    EXPECT_TRUE(journal->VolumeDeleted(volumeToDelete) == true);

    for (int vol = 0; vol < numTest; vol++)
    {
        StripeLog logInfo = writeTester->AddLogForStripe(currentVsid++, vol);
        writtenStripes.push_back(logInfo);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    for (auto stripeLogs : writtenStripes)
    {
        auto stripe = stripeLogs.stripe;
        auto blks = stripeLogs.blks;

        EXPECT_CALL(*testMapper, GetLSA(stripe.vsid)).WillRepeatedly(Return(unmapAddr));

        if (stripe.vsid == deletedVsid)
        {
            _ExpectStripeAllocationReplay(stripe.vsid, stripe.wbAddr.stripeId);
            _ExpectStripeFlushReplay(stripe);
        }
        else
        {
            _ExpectFullStripeReplay(stripe, blks);
        }
    }

    for (int vol = 0; vol < numTest; vol++)
    {
        EXPECT_CALL(*testAllocator, ResetActiveStripeTail(vol)).Times(1);
    }

    EXPECT_CALL(*testAllocator, ReplaySsdLsid).Times(1);

    EXPECT_TRUE(journal->DoRecovery() == 0);
}

TEST_F(JournalVolumeTest, DuplicatedVolumeDeleteEvent)
{
    TEST_DESCRIPTION("Simulate the case when volume is deleted several times");
    IBOF_TRACE_DEBUG(9999, "JournalVolumeTest::DuplicatedVolumeDeleteEvent");

    InitializeJournal();

    int numVolumes = 5;
    unordered_set<int> volumesToDelete;
    volumesToDelete.insert(2);
    volumesToDelete.insert(2);
    volumesToDelete.insert(2);

    vector<StripeLog> writtenStripes = _VolumeDeleteTester(numVolumes, volumesToDelete);
    EXPECT_TRUE((int)writtenStripes.size() == numVolumes);
    _VolumeDeleteReplayTester(writtenStripes, numVolumes, volumesToDelete);
}
