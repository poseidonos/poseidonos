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

#include "src/io/general_io/merged_io.h"

#include "src/include/memory.h"
#include "src/io/general_io/buffer_entry.h"
#include "src/io/general_io/internal_read_completion.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"
#include "src/state/state_manager.h"

namespace ibofos
{
MergedIO::MergedIO(CallbackSmartPtr callback)
: bufferEntry(new BufferEntry(nullptr, 0)),
  startPba({.dev = nullptr, .lba = static_cast<uint64_t>(-1)}),
  nextContiguousLba(UINT64_MAX),
  callback(callback),
  ioDispatcher(*EventArgument::GetIODispatcher())
{
}

MergedIO::~MergedIO(void)
{
    delete bufferEntry;
}

void
MergedIO::Reset(void)
{
    bufferEntry->Reset();
    startPba = {.dev = nullptr, .lba = static_cast<uint64_t>(-1)};
    nextContiguousLba = static_cast<uint64_t>(-1);
}

void
MergedIO::AddContiguousBlock(void)
{
    uint32_t currentBlockCount = bufferEntry->GetBlkCnt();
    uint32_t addedBlockCount = currentBlockCount + 1;
    bufferEntry->SetBlkCnt(addedBlockCount);
    nextContiguousLba += SECTORS_PER_BLOCK;
}

void
MergedIO::SetNewStart(void* newBuffer, PhysicalBlkAddr& newPba)
{
    bufferEntry->SetBuffer(newBuffer);
    bufferEntry->SetBlkCnt(1);
    startPba = newPba;
    nextContiguousLba = newPba.lba + SECTORS_PER_BLOCK;
}

bool
MergedIO::IsContiguous(PhysicalBlkAddr& targetPba)
{
    bool isSameDevice = (startPba.dev == targetPba.dev);
    bool isContiguousLba = (nextContiguousLba == targetPba.lba);
    bool isContiguous = (isSameDevice && isContiguousLba);

    return isContiguous;
}

IOSubmitHandlerStatus
MergedIO::Process(void)
{
    IOSubmitHandlerStatus status = IOSubmitHandlerStatus::SUCCESS;

    if (0 < bufferEntry->GetBlkCnt())
    {
        uint32_t blockCount = bufferEntry->GetBlkCnt();
        UbioSmartPtr ubio(new Ubio(bufferEntry->GetBufferEntry(),
            blockCount * Ubio::UNITS_PER_BLOCK));
        ubio->SetPba(startPba);

        CallbackSmartPtr event(new InternalReadCompletion(blockCount));
#if defined QOS_ENABLED_BE
        event->SetEventType(callback->GetEventType());
#endif
        event->SetCallee(callback);
        ubio->SetCallback(event);

#if defined QOS_ENABLED_BE
        ubio->SetEventType(callback->GetEventType());
#endif
        IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
        if (ioDispatcher->Submit(ubio) < 0)
        {
            IOSubmitHandlerStatus status =
                _CheckAsyncReadError(IBOF_EVENT_ID::REF_COUNT_RAISE_FAIL);
            return status;
        }
    }

    return status;
}

IOSubmitHandlerStatus
MergedIO::_CheckAsyncReadError(IBOF_EVENT_ID eventId)
{
    StateManager* stateMgr = StateManagerSingleton::Instance();
    if (stateMgr->GetState() == State::STOP)
    {
        IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
        return IOSubmitHandlerStatus::FAIL_IN_SYSTEM_STOP;
    }
    return IOSubmitHandlerStatus::SUCCESS;
}

uint32_t
MergedIO::GetBlockCount(void)
{
    return bufferEntry->GetBlkCnt();
}
} // namespace ibofos
