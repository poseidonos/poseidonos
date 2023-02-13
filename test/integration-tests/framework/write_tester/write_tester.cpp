/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
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

#include "src/io/frontend_io/write_submission.h"
#include "test/integration-tests/framework/write_tester/write_tester.h"
#include "test/integration-tests/framework/write_tester/write_submission_fake.h"

namespace pos
{
WriteTester::WriteTester()
: curStripeId(0),
  curBlkOffset(0),
  curLba(0)
{
    mockAllocatorService = new NiceMock<MockAllocatorService>();
    mockRBAStateManager = new NiceMock<MockRBAStateManager>("", 0);

    ON_CALL(*mockRBAStateManager, BulkAcquireOwnership(_, _, _)).WillByDefault(Return(true)); 
}

WriteTester::~WriteTester(void)
{
    delete mockAllocatorService;
    delete mockRBAStateManager;
}

NiceMock<MockIBlockAllocator>*
WriteTester::InitBlockAllocator(VirtualBlks vsaRange)
{
    NiceMock<MockIBlockAllocator>* mockIBlockAllocator =
        reinterpret_cast<NiceMock<MockIBlockAllocator>*>(AllocatorServiceSingleton::Instance()->GetIBlockAllocator("POSArray"));
    ON_CALL(*mockIBlockAllocator, AllocateWriteBufferBlks(_, _)).WillByDefault(Return(std::make_pair(vsaRange,
        UNMAP_STRIPE)));

    return mockIBlockAllocator;
}

NiceMock<MockTranslator>*
WriteTester::InitTranslator(VirtualBlkAddr vsa, uint32_t arrayId, uint32_t numBlks)
{
    NiceMock<MockTranslator>* mockTranslator = 
        new NiceMock<MockTranslator>(vsa, arrayId, UNMAP_STRIPE);
    
    PhysicalBlkAddr physicalBlkAddr {curLba, nullptr};
    StripeAddr stripeAddr;
    stripeAddr.stripeId = curStripeId;
    auto lsidRefResult = std::make_tuple(stripeAddr, true);

    ON_CALL(*mockTranslator, GetPba()).WillByDefault(Return(physicalBlkAddr));
    ON_CALL(*mockTranslator, GetLsidRefResult(_)).WillByDefault(Return(lsidRefResult));

    PhysicalEntry physicalEntry;
    physicalEntry.addr = physicalBlkAddr;
    physicalEntry.blkCnt = numBlks;

    list<PhysicalEntry> physicalEntries;
    physicalEntries.push_back(physicalEntry);
    ON_CALL(*mockTranslator, GetPhysicalEntries(_, _)).WillByDefault(Return(physicalEntries));

    return mockTranslator;
}

VirtualBlks
WriteTester::InitVsaRange(uint32_t numBlks)
{
    VirtualBlkAddr vsa = {.stripeId = curStripeId, .offset = curBlkOffset};
    // TODO: use only 1 blks (temporal code for test)
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 1};
    return vsaRange;
}

void
WriteTester::WriteIo(FakeVolumeIo* newVolumeIo, uint32_t numBlks)
{
    VirtualBlks vsaRange = InitVsaRange(numBlks);   
    NiceMock<MockIBlockAllocator>* mockIBlockAllocator = InitBlockAllocator(vsaRange);
    NiceMock<MockTranslator>* mockTranslator = InitTranslator(vsaRange.startVsa, newVolumeIo->GetArrayId(), numBlks);
    TranslatorSmartPtr translator(mockTranslator);
    VolumeIoSmartPtr volumeIo(newVolumeIo);

    FakeWriteSubmission writeSubmission(volumeIo, mockRBAStateManager, mockIBlockAllocator,
        nullptr, nullptr, false, translator);

    bool actual = writeSubmission.Execute();
    bool expected = true;

    ASSERT_EQ(expected, actual);

    // debug info.
    UpdateAddress();
}

void
WriteTester::UpdateAddress(void)
{
    curBlkOffset = (++curBlkOffset) % BLK_PER_STRIPE;
    ++curLba;

    if (0 == curBlkOffset)
    {
        curStripeId = (++curStripeId) % STRIPE_PER_SEGMENT;
        curLba = 0;
    }
}

} // namespace pos
