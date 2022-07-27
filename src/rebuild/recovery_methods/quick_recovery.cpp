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

#include "quick_recovery.h"
#include "src/io/backend_io/rebuild_io/rebuild_read.h"
#include "src/io/backend_io/rebuild_io/rebuild_read_complete_handler.h"
#include "src/io/backend_io/rebuild_io/rebuild_read_intermediate_complete_handler.h"
#include "src/device/base/ublock_device.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"
#include <functional>

namespace pos
{
QuickRecovery::QuickRecovery(ArrayDevice* srcDev, uint64_t srcSize, uint64_t destSize, uint32_t bufCnt)
: RecoveryBase(srcSize, destSize, bufCnt)
{
    src = srcDev;
    using namespace std::placeholders;
    recoverFunc = bind(&QuickRecovery::_Copy, this, _1, _2, _3);
}

int
QuickRecovery::Recover(UbioSmartPtr ubio)
{
    uint32_t sectorCnt = srcSize / ArrayConfig::SECTOR_SIZE_BYTE;
    void* mem = nullptr;
    mem = srcBuffer->TryGetBuffer();
    if (nullptr == mem)
    {
        POS_TRACE_WARN(EID(RESOURCE_BUFFER_POOL_EMPTY),
            "Failed to get buffer during recover read. {} Pool is empty", srcBuffer->GetOwner());
        mem = Memory<ArrayConfig::SECTOR_SIZE_BYTE>::Alloc(sectorCnt);
    }
    PhysicalBlkAddr srcAddr;
    srcAddr.arrayDev = src;
    srcAddr.lba = ubio->GetPba().lba;

    UbioSmartPtr rebuildUbio(new Ubio(mem, sectorCnt, ubio->GetArrayId()));
    CallbackSmartPtr readCompletion(
        new RebuildReadCompleteHandler(rebuildUbio, recoverFunc, srcSize, srcBuffer));

    rebuildUbio->SetCallback(readCompletion);
    rebuildUbio->SetOriginUbio(ubio);
    readCompletion->SetEventType(ubio->GetEventType());
    rebuildUbio->SetEventType(ubio->GetEventType());

    bool isTail = false;
    UbioSmartPtr split =
        rebuildUbio->Split(ChangeByteToSector(srcSize), isTail);
    split->SetPba(srcAddr);
    CallbackSmartPtr event(
        new RebuildReadIntermediateCompleteHandler(split));
    event->SetCallee(readCompletion);
    event->SetEventType(ubio->GetEventType());
    split->SetEventType(ubio->GetEventType());
    split->SetCallback(event);
    split->SetOriginUbio(rebuildUbio);
    readCompletion->SetWaitingCount(1);

    IODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
    ioDispatcher->Submit(split);
    return EID(SUCCESS);
}

void
QuickRecovery::_Copy(void* dst, void* src, uint32_t size)
{
    memcpy(dst, src, size);
}

} // namespace pos
