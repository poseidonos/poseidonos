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

#pragma once

#include "array_mock.h"
#include "journal_manager_tester.h"
#include "mapper_mock.h"
#include "rba_generator_test.h"
#include "stripe_test_info.h"
#include "test_info.h"
#include "written_logs.h"

class LogWriteTester
{
public:
    LogWriteTester(void) = delete;
    LogWriteTester(MockMapper* _mapper, MockArray* _array, JournalManagerTester* _journal, TestInfo* _testInfo);
    virtual ~LogWriteTester(void);

    void Reset(void);
    void UpdateJournal(JournalManagerTester* _journal);

    bool BlockDoneLogAddTester(int volId, BlkAddr rba, VirtualBlks blks);
    BlockMapList OverwriteBlockDoneLogAddTester(StripeWriteInfo stripe,
        BlkAddr rba,
        uint32_t startOffset,
        uint32_t numTest);
    BlockMapList StripeBlockDoneLogAddTester(StripeWriteInfo stripe,
        uint32_t startOffset, int numBlks);

    bool StripeMapUpdatedLogAddTester(StripeId vsid, StripeAddr oldAddr, StripeAddr newAddr);

    StripeLog AddLogForStripe(StripeId vsid, int volId);

    void WaitForAllLogWriteDone(void);

    bool DoesAllJournalWriteDone(void);
    bool CheckLogInTheList(LogHandlerInterface* log);
    void CompareLogs(void);

    MapPageList GetDirtyMap(void);

private:
    BlockMapList _GenerateBlocksInStripe(StripeId vsid, uint32_t startOffset, int numBlks);
    void _AddToDirtyPageList(int mapId, MpageList dirty);

    WrittenLogs testingLogs;
    MapPageList dirtyPages;
    RbaGenerator* rbaGenerator;

    TestInfo* testInfo;
    MockMapper* mapper;
    MockArray* array;
    JournalManagerTester* journal;
};
