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

#include "src/io/frontend_io/block_map_update_completion.h"

#include "src/allocator_service/allocator_service.h"
#include "src/allocator/i_block_allocator.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/write_completion.h"
#include "src/bio/volume_io.h"
#include "src/mapper_service/mapper_service.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/logger/logger.h"

namespace pos
{
BlockMapUpdateCompletion::BlockMapUpdateCompletion(
    VolumeIoSmartPtr input, CallbackSmartPtr originCallback, 
    function<bool(void)> IsReactorNow,
    IVSAMap *iVSAMap, EventScheduler *eventScheduler, 
    WriteCompletion *writeCompletion, IBlockAllocator* iBlockAllocator)
: Event(IsReactorNow()),
  volumeIo(input),
  originCallback(originCallback),
  iVSAMap(iVSAMap),
  eventScheduler(eventScheduler),
  writeCompletion(writeCompletion),
  iBlockAllocator(iBlockAllocator)

{
}

BlockMapUpdateCompletion::BlockMapUpdateCompletion(VolumeIoSmartPtr inputVolumeIo,
    CallbackSmartPtr originCallback)
: BlockMapUpdateCompletion(
    inputVolumeIo, originCallback, EventFrameworkApi::IsReactorNow,
    MapperServiceSingleton::Instance()->GetIVSAMap(inputVolumeIo->GetArrayName()),
    EventSchedulerSingleton::Instance(),
    new WriteCompletion(inputVolumeIo),
    AllocatorServiceSingleton::Instance()->GetIBlockAllocator(inputVolumeIo->GetArrayName()))
{
}


BlockMapUpdateCompletion::~BlockMapUpdateCompletion(void)
{
}

bool
BlockMapUpdateCompletion::Execute(void)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlks targetVsaRange = {.startVsa = volumeIo->GetVsa(), .numBlks = blockCount};
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    if (volumeIo->IsGc())
    {
        iVSAMap->SetVSAsInternal(volumeIo->GetVolumeId(), rba, targetVsaRange);
    }
    else
    {
        iVSAMap->SetVSAs(volumeIo->GetVolumeId(), rba, targetVsaRange);
    }
    iBlockAllocator->ValidateBlks(targetVsaRange);

    CallbackSmartPtr event(writeCompletion);
    CallbackSmartPtr callee;

    if (originCallback == nullptr)
    {
        callee = volumeIo->GetOriginUbio()->GetCallback();
    }
    else
    {
        callee = originCallback;
    }

    event->SetCallee(callee);
    bool wrapupSuccessful = event->Execute();

    if (unlikely(false == wrapupSuccessful))
    {
        writeCompletion->SetCallee(callee);
        eventScheduler->EnqueueEvent(event);
    }
    volumeIo = nullptr;

    return true;
}

} // namespace pos
