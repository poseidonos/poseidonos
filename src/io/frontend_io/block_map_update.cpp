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

#include "src/io/frontend_io/block_map_update.h"

#include "src/device/event_framework_api.h"
#include "src/include/memory.h"
#include "src/io/frontend_io/block_map_update_completion.h"
#include "src/journal_manager/journal_manager.h"
#include "src/mapper/mapper.h"

namespace ibofos
{
BlockMapUpdate::BlockMapUpdate(VolumeIoSmartPtr inputVolumeIo, CallbackSmartPtr originCallback)
: Event(EventFrameworkApi::IsReactorNow()),
  volumeIo(inputVolumeIo),
  mapper(MapperSingleton::Instance()),
  journalManager(JournalManagerSingleton::Instance()),
  originCallback(originCallback)
{
}

bool
BlockMapUpdate::Execute(void)
{
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetRba());
    MpageList dirty = mapper->GetDirtyVsaMapPages(volumeIo->GetVolumeId(),
        rba, blockCount);

    EventSmartPtr event(new BlockMapUpdateCompletion(volumeIo, originCallback));
    bool logWriteRequestSuccessful = journalManager->AddBlockMapUpdatedLog(
        volumeIo, dirty, event);

    if (logWriteRequestSuccessful)
    {
        volumeIo = nullptr;
    }

    return logWriteRequestSuccessful;
}

} // namespace ibofos
