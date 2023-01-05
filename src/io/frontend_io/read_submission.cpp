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

#include "src/io/frontend_io/read_submission.h"

#include <air/Air.h>
#include <unistd.h>

#include "spdk/event.h"
#include "src/admin/smart_log_mgr.h"
#include "src/array/array.h"
#include "src/array/device/array_device.h"
#include "src/bio/volume_io.h"
#include "src/device/base/ublock_device.h"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"
#include "src/event_scheduler/callback.h"
#include "src/include/pos_event_id.hpp"

namespace pos
{
ReadSubmission::ReadSubmission(VolumeIoSmartPtr volumeIo, BlockAlignment* blockAlignment_, Merger* merger_, Translator* translator_)
: Event(true),
  blockAlignment(blockAlignment_),
  merger(merger_),
  translator(translator_),
  volumeIo(volumeIo)
{
    if (nullptr == blockAlignment)
    {
        blockAlignment = new BlockAlignment{ChangeSectorToByte(volumeIo->GetSectorRba()), volumeIo->GetSize()};
    }
    if (nullptr == merger)
    {
        merger = new Merger{volumeIo, &readCompletionFactory};
    }
    if (nullptr == translator)
    {
        translator = new Translator{volumeIo->GetVolumeId(), blockAlignment->GetHeadBlock(), blockAlignment->GetBlockCount(), volumeIo->GetArrayId(), true};
    }
    airlog("RequestedUserRead", "user", GetEventType(), 1);
}

ReadSubmission::~ReadSubmission()
{
    if (nullptr != blockAlignment)
    {
        delete blockAlignment;
        blockAlignment = nullptr;
    }
    if (nullptr != merger)
    {
        delete merger;
        merger = nullptr;
    }
    if (nullptr != translator)
    {
        delete translator;
        translator = nullptr;
    }
}

bool
ReadSubmission::Execute(void)
{
    uint32_t volId = volumeIo->GetVolumeId();
    uint32_t arrayId = volumeIo->GetArrayId();
    SmartLogMgrSingleton::Instance()->IncreaseReadCmds(volId, arrayId);
    SmartLogMgrSingleton::Instance()->IncreaseReadBytes(blockAlignment->GetBlockCount(), volId, arrayId);
    bool isInSingleBlock = (blockAlignment->GetBlockCount() == 1);
    if (isInSingleBlock)
    {
        _PrepareSingleBlock();
        _SendVolumeIo(volumeIo);
    }
    else
    {
        _PrepareMergedIo();
        _ProcessMergedIo();
    }

    volumeIo = nullptr;
    return true;
}

void
ReadSubmission::_PrepareSingleBlock(void)
{
    PhysicalBlkAddr pba = translator->GetPba();
    pba.lba = blockAlignment->AlignHeadLba(0, pba.lba);
    volumeIo->SetPba(pba);
    StripeAddr lsidEntry;
    bool referenced;
    std::tie(lsidEntry, referenced) = translator->GetLsidRefResult(0);
    if (referenced)
    {
        CallbackSmartPtr callee(volumeIo->GetCallback());
        CallbackSmartPtr readCompletion(new ReadCompletion(volumeIo));
        readCompletion->SetCallee(callee);
        volumeIo->SetCallback(readCompletion);
        volumeIo->SetLsidEntry(lsidEntry);
        callee->SetWaitingCount(1);
    }
}

void
ReadSubmission::_PrepareMergedIo(void)
{
    uint32_t blockCount = blockAlignment->GetBlockCount();

    for (uint32_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        _MergeBlock(blockIndex);
    }

    merger->Cut();
}

void
ReadSubmission::_MergeBlock(uint32_t blockIndex)
{
    PhysicalBlkAddr pba = translator->GetPba(blockIndex);
    VirtualBlkAddr vsa = translator->GetVsa(blockIndex);
    StripeAddr lsidEntry = translator->GetLsidEntry(blockIndex);
    uint32_t dataSize = blockAlignment->GetDataSize(blockIndex);
    pba.lba = blockAlignment->AlignHeadLba(blockIndex, pba.lba);
    merger->Add(pba, vsa, lsidEntry, dataSize);
}

void
ReadSubmission::_ProcessMergedIo(void)
{
    uint32_t volumeIoCount = merger->GetSplitCount();
    CallbackSmartPtr callback = volumeIo->GetCallback();
    callback->SetWaitingCount(volumeIoCount);

    for (uint32_t volumeIoIndex = 0; volumeIoIndex < volumeIoCount;
         volumeIoIndex++)
    {
        _ProcessVolumeIo(volumeIoIndex);
    }
}

void
ReadSubmission::_ProcessVolumeIo(uint32_t volumeIoIndex)
{
    VolumeIoSmartPtr volumeIo = merger->GetSplit(volumeIoIndex);
    _SendVolumeIo(volumeIo);
}

} // namespace pos
