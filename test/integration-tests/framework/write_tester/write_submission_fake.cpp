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

#include "test/integration-tests/framework/write_tester/write_submission_fake.h"
#include "src/io/frontend_io/block_map_update_request.h"
#include <iostream>

namespace pos
{
FakeWriteSubmission::FakeWriteSubmission(VolumeIoSmartPtr volumeIo, RBAStateManager* inputRbaStateManager,
    IBlockAllocator* inputIBlockAllocator, FlowControl* inputFlowControl, IVolumeInfoManager* inputVolumeManager,
    bool isReactorNow, TranslatorSmartPtr translator_)
: WriteSubmission(volumeIo, inputRbaStateManager, inputIBlockAllocator, inputFlowControl, inputVolumeManager,
    isReactorNow),
  translator(translator_)
{

}

FakeWriteSubmission::~FakeWriteSubmission(void)
{

}

bool
FakeWriteSubmission::Execute(void)
{
    WriteSubmission::Execute();
}

void
FakeWriteSubmission::_SetupVolumeIo(VolumeIoSmartPtr newVolumeIo, VirtualBlksInfo& virtualBlksInfo,
    CallbackSmartPtr callback)
{
    VirtualBlks vsaRange = virtualBlksInfo.first;
    StripeId userLsid = virtualBlksInfo.second;
    VirtualBlkAddr startVsa = vsaRange.startVsa;

    StripeAddr lsidEntry;
    list<PhysicalEntry> physicalEntries;

    void* mem = newVolumeIo->GetBuffer();
    physicalEntries = translator->GetPhysicalEntries(mem, vsaRange.numBlks);
    lsidEntry = translator->GetLsidEntry(0);

    // TODO Support multiple buffers (for RAID1) - create additional VolumeIo
    assert(physicalEntries.size() == 1);

    newVolumeIo->SetVsa(startVsa);
    PhysicalBlkAddr pba = physicalEntries.front().addr;
    if (volumeIo != newVolumeIo)
    {
        newVolumeIo->SetOriginUbio(volumeIo);
    }
    newVolumeIo->SetVsa(startVsa);
    newVolumeIo->SetPba(pba);
    newVolumeIo->SetUserLsid(userLsid);

    CallbackSmartPtr blockMapUpdateRequest(
        new BlockMapUpdateRequest(newVolumeIo, callback));

    newVolumeIo->SetCallback(blockMapUpdateRequest);
    newVolumeIo->SetLsidEntry(lsidEntry);

    pba.lba += ChangeBlockToSector(vsaRange.numBlks);
}
} // namespace pos
