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

#include "src/io/backend_io/rebuild_io/rebuild_read.h"

#include <list>

#include "rebuild_read_complete_handler.h"
#include "rebuild_read_intermediate_complete_handler.h"
#include "src/array/service/array_service_layer.h"
#include "src/bio/ubio.h"
#include "src/include/array_config.h"
#include "src/include/recover_method.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/lib/block_alignment.h"
#include "src/logger/logger.h"
#include "src/resource_manager/buffer_pool.h"
namespace pos
{
int
RebuildRead::Recover(UbioSmartPtr ubio, BufferPool* bufferPool)
{
    if (true == ubio->IsRetry())
    {
        int event = 9999; // TODO(iopath) temporal event id
        POS_TRACE_ERROR(event, "RebuildRead::GetRecoverMethod IsRetry");
        return event;
    }

    ubio->SetRetry(true);

    IIORecover* recover = ArrayService::Instance()->Getter()->GetRecover();
    RecoverMethod rm;
    int ret = recover->GetRecoverMethod(ubio->GetArrayId(), ubio, rm);
    if (ret != 0)
    {
        return ret;
    }

    const uint32_t sectorSize = ArrayConfig::SECTOR_SIZE_BYTE;
    const uint32_t sectorsPerBlock = ArrayConfig::SECTORS_PER_BLOCK;
    const uint32_t blockSize = ArrayConfig::BLOCK_SIZE_BYTE;

    PhysicalBlkAddr originPba = ubio->GetPba();
    BlockAlignment blockAlignment(originPba.lba * sectorSize, ubio->GetSize());
    originPba.lba = blockAlignment.GetHeadBlock() * sectorsPerBlock;

    uint32_t readSize = blockAlignment.GetBlockCount() * blockSize;
    uint32_t sectorCnt = rm.srcAddr.size() * readSize / sectorSize;
    void* mem = nullptr;
    if (bufferPool == nullptr) // Degraded Or Timeout case
    {
        mem = Memory<sectorSize>::Alloc(sectorCnt);
    }
    else // Total Rebuild Case
    {
        mem = bufferPool->TryGetBuffer();
    }

    UbioSmartPtr rebuildUbio(new Ubio(mem, sectorCnt, ubio->GetArrayId()));
    rebuildUbio->SetRetry(true);

    CallbackSmartPtr rebuildCompletion(
        new RebuildReadCompleteHandler(rebuildUbio, rm.recoverFunc, readSize, bufferPool));

    rebuildUbio->SetCallback(rebuildCompletion);
    rebuildUbio->SetOriginUbio(ubio);
    rebuildCompletion->SetEventType(ubio->GetEventType());
    rebuildUbio->SetEventType(ubio->GetEventType());
    list<UbioSmartPtr> splitList;
    for (auto pba : rm.srcAddr)
    {
        bool isTail = false;
        UbioSmartPtr split =
            rebuildUbio->Split(ChangeByteToSector(readSize), isTail);
        split->SetPba(pba);
        CallbackSmartPtr event(
            new RebuildReadIntermediateCompleteHandler(split));
        event->SetCallee(rebuildCompletion);
        event->SetEventType(ubio->GetEventType());
        split->SetEventType(ubio->GetEventType());
        split->SetCallback(event);
        split->SetOriginUbio(rebuildUbio);
        splitList.push_back(split);
    }

    rebuildCompletion->SetWaitingCount(splitList.size());

    for (auto split : splitList)
    {
        IODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
        ioDispatcher->Submit(split);
    }

    return EID(SUCCESS);
}
} // namespace pos
