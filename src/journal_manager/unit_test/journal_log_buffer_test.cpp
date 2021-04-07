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

#include "journal_log_buffer_test.h"

#include <iostream>

#include "../log/log_buffer_parser.h"
#include "../log_buffer/journal_log_buffer.h"
#include "../log_buffer/log_write_context_factory.h"
#include "../log_write/buffer_write_done_notifier.h"
#include "test_info.h"

void
JournalLogBufferTest::SetUp(void)
{
    numLogsWritten = 0;

    logBuffer = new JournalLogBuffer();
    logBuffer->Delete();
    logBuffer->Setup();
    logBuffer->SyncResetAll();
}

void
JournalLogBufferTest::TearDown(void)
{
    delete logBuffer;
}

LogWriteContext*
JournalLogBufferTest::_CreateBlockLogWriteContext(void)
{
    LogWriteContextFactory factory;
    factory.Init(new LogBufferWriteDoneNotifier());

    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 0));
    volumeIo->SetRba(0);
    volumeIo->SetVolumeId(testInfo->defaultTestVol);

    MpageList dirty;

    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateBlockMapLogWriteContext(volumeIo, dirty, callback);
    context->callback = std::bind(&JournalLogBufferTest::WriteDone, this,
        std::placeholders::_1);
    return context;
}

int
JournalLogBufferTest::_ParseLogBuffer(int groupId, LogList& groupLogs)
{
    void* buffer = malloc(logBuffer->GetLogGroupSize());

    EXPECT_TRUE(logBuffer->ReadLogBuffer(0, buffer) == 0);

    LogBufferParser parser;
    int result = parser.GetLogs(buffer, logBuffer->GetLogGroupSize(), groupLogs);

    free(buffer);
    return result;
}

void
JournalLogBufferTest::WriteDone(AsyncMetaFileIoCtx* ctx)
{
    numLogsWritten++;
}

void
JournalLogBufferTest::_WaitForLogWriteDone(int numLogsWaitingFor)
{
    while (numLogsWritten != numLogsWaitingFor)
    {
    }
}

///----------------------------------------------------------------------------

TEST_F(JournalLogBufferTest, ParseLogBuffer)
{
    JournalTest::SetUp();

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

        StripeAddr oldAddr = unmapAddr;
        StripeAddr newAddr = {.stripeLoc = IN_USER_AREA,
            .stripeId = testInfo->numTest / 10};

        writeTester->StripeMapUpdatedLogAddTester(testCnt, oldAddr, newAddr);
    }

    writeTester->WaitForAllLogWriteDone();
    SimulateSPORWithoutRecovery();

    writeTester->CompareLogs();

    JournalTest::TearDown();
}

TEST_F(JournalLogBufferTest, WriteInvalidLogType)
{
    TEST_DESCRIPTION("Write log with invalid type to the buffer, and test parse fail");
    IBOF_TRACE_DEBUG(9999, "JournalLogBufferTest::WriteInvalidLogType");

    LogWriteContext* context = _CreateBlockLogWriteContext();

    BlockWriteDoneLogHandler* blockLog = dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());
    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* typePtr = reinterpret_cast<int*>(&(log->type));
        *typePtr = -1;

        EXPECT_TRUE(logBuffer->WriteLog(context, 0, 0) == 0);

        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) != 0);
        EXPECT_TRUE(groupLogs.size() == 0);
    }
    else
    {
        std::cout << "Failed to get blocklog" << std::endl;
    }

    delete context;
}

TEST_F(JournalLogBufferTest, WriteWithoutMark)
{
    TEST_DESCRIPTION("Write log with invalid mark to the buffer, and test parse fail");
    IBOF_TRACE_DEBUG(9999, "JournalLogBufferTest::WriteWithoutMark");

    LogWriteContext* context = _CreateBlockLogWriteContext();

    BlockWriteDoneLogHandler* blockLog = dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());
    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* markPtr = reinterpret_cast<int*>(&(log->mark));
        *markPtr = 0;

        EXPECT_TRUE(logBuffer->WriteLog(context, 0, 0) == 0);

        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) == 0);
        EXPECT_TRUE(groupLogs.size() == 0);
    }
    else
    {
        std::cout << "Failed to get blocklog" << std::endl;
    }

    delete context;
}
