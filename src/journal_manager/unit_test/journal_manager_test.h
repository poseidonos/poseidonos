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

#include "allocator_mock.h"
#include "array_mock.h"
#include "gtest/gtest.h"
#include "journal_manager_tester.h"
#include "log_write_test.h"
#include "mapper_mock.h"
#include "src/include/address_type.h"
#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "stripe_test_info.h"
#include "test_info.h"

using namespace ibofos;

class JournalTest : public ::testing::Test
{
public:
    void InitializeJournal(bool isEnabled = true);
    void InitializeJournal(uint32_t logBufferSize, bool isCheckpointEnabled);
    void SimulateSPORWithoutRecovery(void);

    uint32_t GetNumTestsBeforeLogGroupFull(uint32_t unitSize);
    void WaitForFlushingLogGroupIdle(int prevNumber);
    VirtualBlkAddr GetNextBlock(VirtualBlks blks);

    TestInfo* testInfo;

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    JournalManagerTester* journal;
    MockMapper* testMapper;
    MockAllocator* testAllocator;
    MockArray* testArray;

    LogWriteTester* writeTester;

    uint32_t logBufferSize;
    uint32_t logGroupSize;
    int numLogGroups;

private:
    void _GetLogBufferSizeInfo(void);
    void _WaitForAllFlushDone(void);
};
