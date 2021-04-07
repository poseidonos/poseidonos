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

#include "src/device/event_framework_api.h"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/write_completion.h"
#include "src/io/general_io/volume_io.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"

namespace ibofos
{
BlockMapUpdateCompletion::BlockMapUpdateCompletion(
    VolumeIoSmartPtr input, CallbackSmartPtr originCallback)
: Event(EventFrameworkApi::IsReactorNow()),
  volumeIo(input),
  mapper(MapperSingleton::Instance()),
  originCallback(originCallback)
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
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetRba());
    if (volumeIo->IsGc())
    {
        mapper->SetVsaMapInternal(volumeIo->GetVolumeId(), rba, targetVsaRange);
    }
    else
    {
        mapper->SetVsaMap(volumeIo->GetVolumeId(), rba, targetVsaRange);
    }

    WriteCompletion event(volumeIo);
    CallbackSmartPtr callee;

    if (originCallback == nullptr)
    {
        callee = volumeIo->GetOriginUbio()->GetCallback();
    }
    else
    {
        callee = originCallback;
    }

    event.SetCallee(callee);
    bool wrapupSuccessful = event.Execute();

    if (unlikely(false == wrapupSuccessful))
    {
        WriteCompletion* writeCompletion = new WriteCompletion(volumeIo);
        if (likely(writeCompletion != nullptr))
        {
            writeCompletion->SetCallee(callee);
            EventSmartPtr event(writeCompletion);
            EventArgument::GetEventScheduler()->EnqueueEvent(event);
        }
        else
        {
            IBOF_TRACE_ERROR(static_cast<int>(IBOF_EVENT_ID::MAP_UPDATE_HANDLER_EVENT_ALLOCATE_FAIL),
                "Failed to allocate write wrapup event");
        }
    }
    volumeIo = nullptr;

    return true;
}

} // namespace ibofos
