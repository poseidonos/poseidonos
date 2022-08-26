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

#include "io_context.h"

#include "src/bio/ubio.h"
#include "src/device/base/ublock_device.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
IOContext::IOContext(UbioSmartPtr inputUbio, uint32_t inputRetry)
: ubio(inputUbio),
  keyForPendingIOListSet(false),
  keyForPendingErrorListSet(false),
  remainingErrorRetryCount(inputRetry)
{
    outOfMemoryRetryCount = 0;
    completeCalled = false;
}

IOContext::~IOContext(void)
{
}

void
IOContext::ClearAsyncIOCompleted(void)
{
    completeCalled = false;
}

void
IOContext::SetAsyncIOCompleted(void)
{
    completeCalled = true;
}
bool
IOContext::IsAsyncIOCompleted(void)
{
    return completeCalled;
}

void
IOContext::ClearErrorRetryCount(void)
{
    remainingErrorRetryCount = 0;
}

void
IOContext::IncOutOfMemoryRetryCount(void)
{
    outOfMemoryRetryCount++;
}

void
IOContext::ClearOutOfMemoryRetryCount(void)
{
    outOfMemoryRetryCount = 0;
}

uint32_t
IOContext::GetOutOfMemoryRetryCount(void)
{
    return outOfMemoryRetryCount;
}

bool
IOContext::CheckAndDecreaseErrorRetryCount(void)
{
    bool isPositive = (remainingErrorRetryCount > 0);
    if (isPositive)
    {
        remainingErrorRetryCount--;
    }
    return (isPositive);
}

std::string
IOContext::GetDeviceName(void)
{
    return ubio->GetUBlock()->GetName();
}

void
IOContext::SetErrorKey(std::list<IOContext*>::iterator it)
{
    keyForPendingErrorList = it;
    keyForPendingErrorListSet = true;
}

std::pair<std::list<IOContext*>::iterator, bool>
IOContext::GetErrorKey(void)
{
    if (unlikely(false == keyForPendingErrorListSet))
    {
        POS_EVENT_ID eventId = EID(IOCTX_ERROR_KEY_NOT_SET);
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Key for pending ERROR was not set, before!");
    }

    return std::make_pair(keyForPendingErrorList, keyForPendingErrorListSet);
}

UbioDir
IOContext::GetOpcode(void)
{
    return ubio->dir;
}

void*
IOContext::GetBuffer(void)
{
    return ubio->GetBuffer();
}

uint64_t
IOContext::GetStartByteOffset(void)
{
    return ChangeSectorToByte(ubio->GetLba());
}

uint64_t
IOContext::GetByteCount(void)
{
    return ubio->GetSize();
}

uint64_t
IOContext::GetStartSectorOffset(void)
{
    return ubio->GetLba();
}

uint64_t
IOContext::GetSectorCount(void)
{
    return ChangeByteToSector(ubio->GetSize());
}

void
IOContext::AddPendingErrorCount(uint32_t errorCountToAdd)
{
    return ubio->GetUBlock()->AddPendingErrorCount(errorCountToAdd);
}

void
IOContext::SubtractPendingErrorCount(uint32_t errorCountToSubtract)
{
    return ubio->GetUBlock()->SubtractPendingErrorCount(errorCountToSubtract);
}

void
IOContext::CompleteIo(IOErrorType errorType)
{
    IoCompleter ioCompleter(ubio);
    ioCompleter.CompleteUbio(errorType, true);
    ubio = nullptr;
}

BackendEvent
IOContext::GetEventType(void)
{
    return ubio->GetEventType();
}

} // namespace pos
