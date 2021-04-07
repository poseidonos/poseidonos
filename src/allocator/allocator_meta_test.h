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

#include "allocator_address_info.h"
#include "allocator_meta_archive.h"

#include "gtest/gtest.h"

using namespace std;
using namespace ibofos;

class AllocatorMetaTest;

class FlushDoneEvent : public Event
{
public:
    FlushDoneEvent(AllocatorMetaTest* tester);
    virtual ~FlushDoneEvent(void);
    virtual bool Execute(void);

private:
    AllocatorMetaTest* tester;
};

class AllocatorMetaTest : public ::testing::Test
{
public:
    void FlushDoneCallback(void);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    void _SetupAddrInfo(void);

    void _SimulateNPOR(void);
    void _SimulateSPOR(void);

    void _SetupMockEventScheduler(void);
    void _ResetMockEventScheduler(void);

    void _TriggerFlush(char* buffer);

    ibofos::AllocatorAddressInfo* info;
    AllocatorMetaArchive* meta;
    const int TEST_SIZE = 100;
    const int TEST_BIT = 3;

    std::atomic<bool> flushCompleted;
};
