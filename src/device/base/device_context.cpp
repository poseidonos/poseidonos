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

#include "src/device/base/device_context.h"

#include "src/include/branch_prediction.h"

using namespace pos;

const uint64_t
    DeviceContext::NANOS_TO_WAIT_UNTIL_ERROR_COMPLETION = 0ULL;

DeviceContext::DeviceContext(SystemTimeoutChecker* timeoutChecker)
: pendingIoCount(0),
  nextErrorCompletionStartIt(pendingErrorList.end()),
  timeoutChecker(timeoutChecker)
{
}

DeviceContext::~DeviceContext(void)
{
    if (timeoutChecker != nullptr)
    {
        delete timeoutChecker;
    }
}

void
DeviceContext::IncreasePendingIO(void)
{
    pendingIoCount++;
}

void
DeviceContext::DecreasePendingIO(void)
{
    pendingIoCount--;
}

uint32_t
DeviceContext::GetPendingIOCount(void)
{
    return pendingIoCount;
}

void
DeviceContext::AddPendingError(IOContext& errorToAdd)
{
    std::list<IOContext*>::iterator it =
        _AddPendingIOContext(pendingErrorList, errorToAdd);
    errorToAdd.SetErrorKey(it);
    errorToAdd.AddPendingErrorCount();

    if (timeoutChecker->CheckTimeout())
    {
        _ResetTargetExpiration(errorToAdd);
    }
}

void
DeviceContext::_ResetTargetExpiration(IOContext& firstErrorForNextTimeout)
{
    std::pair<std::list<IOContext*>::iterator, bool> result =
        firstErrorForNextTimeout.GetErrorKey();
    std::list<IOContext*>::iterator currentIt = result.first;
    bool errorAvailable = result.second;
    if (likely(errorAvailable))
    {
        nextErrorCompletionStartIt = currentIt;
        timeoutChecker->SetTimeout(NANOS_TO_WAIT_UNTIL_ERROR_COMPLETION);
    }
}

void
DeviceContext::RemovePendingError(IOContext& errorToRemove)
{
    std::pair<std::list<IOContext*>::iterator, bool> result =
        errorToRemove.GetErrorKey();
    std::list<IOContext*>::iterator currentIt = result.first;
    bool errorAvailable = result.second;
    if (likely(errorAvailable))
    {
        _RemovePendingIOContext(pendingErrorList, currentIt);
        errorToRemove.SubtractPendingErrorCount();
    }
}

uint32_t
DeviceContext::GetPendingErrorCount(void)
{
    return _GetPendingIOContextCount(pendingErrorList);
}

void
DeviceContext::_ReadyAllRemainingErrors(void)
{
    nextErrorCompletionStartIt = pendingErrorList.end();
}

IOContext*
DeviceContext::GetPendingError(void)
{
    IOContext* ioCtx = _GetPendingIOContext(pendingErrorList);

    if (nullptr != ioCtx)
    {
        if (timeoutChecker->CheckTimeout())
        {
            _ReadyAllRemainingErrors();
        }

        if (false == _CheckErrorReady(*ioCtx))
        {
            ioCtx = nullptr;
        }
    }

    return ioCtx;
}

bool
DeviceContext::_CheckErrorReady(IOContext& ioCtx)
{
    std::pair<std::list<IOContext*>::iterator, bool> result =
        ioCtx.GetErrorKey();
    std::list<IOContext*>::iterator currentIt = result.first;
    bool errorAvailable = result.second;
    if (likely(errorAvailable))
    {
        errorAvailable = (nextErrorCompletionStartIt != currentIt);
    }

    return errorAvailable;
}

std::list<IOContext*>::iterator
DeviceContext::_AddPendingIOContext(std::list<IOContext*>& ioCtxList,
    IOContext& ioCtxToAdd)
{
    std::list<IOContext*>::iterator it =
        ioCtxList.insert(ioCtxList.cend(), &ioCtxToAdd);
    return it;
}

void
DeviceContext::_RemovePendingIOContext(std::list<IOContext*>& ioCtxList,
    std::list<IOContext*>::iterator itToRemove)
{
    ioCtxList.erase(itToRemove);
}

uint32_t
DeviceContext::_GetPendingIOContextCount(std::list<IOContext*>& ioCtxList)
{
    return ioCtxList.size();
}

IOContext*
DeviceContext::_GetPendingIOContext(std::list<IOContext*>& ioCtxList)
{
    IOContext* pendingError = nullptr;

    if (0 < ioCtxList.size())
    {
        pendingError = ioCtxList.front();
    }

    return pendingError;
}
