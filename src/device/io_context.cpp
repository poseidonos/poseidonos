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

#include "io_context.h"

#include "src/device/ublock_device.h"
#include "src/include/branch_prediction.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"

namespace ibofos
{
IOContext::IOContext(UbioSmartPtr inputUbio, uint32_t inputRetry)
: ubio(inputUbio),
  keyForPendingIOListSet(false),
  keyForPendingErrorListSet(false),
  retryCount(inputRetry)
{
    submitRetryCount = 0;
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
IOContext::ClearRetryCount(void)
{
    retryCount = 0;
}

void
IOContext::IncSubmitRetryCount(void)
{
    submitRetryCount++;
}

void
IOContext::ClearSubmitRetryCount(void)
{
    submitRetryCount = 0;
}

uint32_t
IOContext::GetSubmitRetryCount(void)
{
    return submitRetryCount;
}

bool
IOContext::CheckAndDecreaseRetryCount(void)
{
    bool isPositive = (retryCount > 0);
    if (isPositive)
    {
        retryCount--;
    }
    return (isPositive);
}

std::string
IOContext::GetDeviceName(void)
{
    return ubio->GetUBlock()->GetName();
}

void
IOContext::SetIOKey(std::list<IOContext*>::iterator it)
{
    keyForPendingIOList = it;
    keyForPendingIOListSet = true;
}

std::pair<std::list<IOContext*>::iterator, bool>
IOContext::GetIOKey(void)
{
    if (unlikely(false == keyForPendingIOListSet))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOCTX_IO_KEY_NOT_SET;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
    }

    return std::make_pair(keyForPendingIOList, keyForPendingIOListSet);
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
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::IOCTX_ERROR_KEY_NOT_SET;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
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

void
IOContext::IgnoreRecovery(void)
{
    ubio->IgnoreRecovery();
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

bool
IOContext::CheckErrorDisregard(void)
{
    return ubio->GetUBlock()->GetErrorDisregard();
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
IOContext::CompleteIo(CallbackError errorType)
{
    ubio->Complete(errorType);
    ubio = nullptr;
}

} // namespace ibofos
