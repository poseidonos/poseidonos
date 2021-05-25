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

#include "allocator_meta_test.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/lib/bitmap.h"

FlushDoneEvent::FlushDoneEvent(AllocatorMetaTest* tester)
: tester(tester)
{
}

FlushDoneEvent::~FlushDoneEvent(void)
{
}

bool
FlushDoneEvent::Execute(void)
{
    tester->FlushDoneCallback();
    return true;
}

void
AllocatorMetaTest::SetUp(void)
{
    info = new AllocatorAddressInfo();
    _SetupAddrInfo();

    meta = new AllocatorContext(*info);

    _SetupMockEventScheduler();
}

void
AllocatorMetaTest::TearDown(void)
{
    delete info;
    delete meta;

    _ResetMockEventScheduler();
}

void
AllocatorMetaTest::_SetupAddrInfo(void)
{
    info->Init();
    info->SetblksPerStripe(128);
    info->SetblksPerSegment(128 * 1024);
    info->SetnumWbStripes(10);
    info->SetnumUserAreaSegments(1);
    info->SetnumUserAreaStripes(1024);
}

void
AllocatorMetaTest::_SetupMockEventScheduler(void)
{
    cpu_set_t zeroed;
    CPU_ZERO(&zeroed);

    EventSchedulerSingleton::Instance()->Initialize(1, zeroed, zeroed);
}

void
AllocatorMetaTest::_ResetMockEventScheduler(void)
{
}

void
AllocatorMetaTest::_SimulateNPOR(void)
{
    meta->StoreSync();
    delete meta;

    meta = new AllocatorContext(*info);
    // meta->LoadSync();
}

void
AllocatorMetaTest::_SimulateSPOR(void)
{
    delete meta;

    meta = new AllocatorContext(*info);
    // meta->LoadSync();
}

void
AllocatorMetaTest::FlushDoneCallback(void)
{
    flushCompleted = true;
}

void
AllocatorMetaTest::_TriggerFlush(char* data)
{
    flushCompleted = false;

    EventSmartPtr event(new FlushDoneEvent(this));
    EXPECT_TRUE(meta->Flush(data, event) == 0);
    // dataToWrite will be freed once flush is completed
}

TEST_F(AllocatorMetaTest, LoadStoreTest)
{
    meta->StoreSync();
    _SimulateSPOR();
    meta->LoadSync();
}

TEST_F(AllocatorMetaTest, WbLsidBitmap)
{
    (meta->wbLsidBitmap)->SetBit(TEST_BIT);
    _SimulateNPOR();
    EXPECT_TRUE((meta->wbLsidBitmap)->IsSetBit(TEST_BIT));
}

TEST_F(AllocatorMetaTest, WbTail)
{
    for (uint32_t volumeId = 0; volumeId < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++volumeId)
    {
        VirtualBlkAddr vsa = {.stripeId = volumeId, .offset = 0};
        meta->SetActiveStripeTail(volumeId, vsa);
    }

    _SimulateNPOR();

    for (int volumeId = 0; volumeId < ACTIVE_STRIPE_TAIL_ARRAYLEN; volumeId++)
    {
        VirtualBlkAddr vsa = meta->GetActiveStripeTail(volumeId);
        EXPECT_TRUE((vsa.stripeId == volumeId) && (vsa.offset == 0));
    }
}

TEST_F(AllocatorMetaTest, FlushMeta)
{
    (meta->wbLsidBitmap)->SetBit(TEST_BIT);
    for (uint32_t volumeId = 0; volumeId < ACTIVE_STRIPE_TAIL_ARRAYLEN; ++volumeId)
    {
        VirtualBlkAddr vsa = {.stripeId = volumeId, .offset = 0};
        meta->SetActiveStripeTail(volumeId, vsa);
    }

    char* origData = new char[meta->GetMetaHeaderTotalSize()];
    char* dataToWrite = meta->GetCopiedMetaBuffer();
    memcpy(origData, dataToWrite, meta->GetMetaHeaderTotalSize());

    _TriggerFlush(dataToWrite);
    while (flushCompleted != true)
    {
    }

    _SimulateSPOR();

    char* readData = meta->GetCopiedMetaBuffer();
    EXPECT_TRUE(memcmp(origData, readData, meta->GetMetaHeaderTotalSize()) == 0);

    delete[] origData;
    delete[] readData;
}
