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

#pragma once

#include <list>
#include <string>
#include <utility>

#include "src/bio/ubio.h"
#include "src/debug_lib/dump_shared_ptr.h"
#include "src/debug_lib/dump_shared_ptr.hpp"
#include "src/include/io_error_type.h"

namespace pos
{
enum class UbioDir;
class Ubio;

class IOContext : public DumpSharedPtr<IOContext*, static_cast<int>(DumpSharedPtrType::IO_CONTEXT)>
{
public:
    IOContext()
    {
    } // For MockClass
    IOContext(UbioSmartPtr inputUbio, uint32_t inputRetry);
    virtual ~IOContext(void);

    virtual void SetErrorKey(std::list<IOContext*>::iterator it);
    virtual std::pair<std::list<IOContext*>::iterator, bool> GetErrorKey(void);

    virtual std::string GetDeviceName(void);
    virtual uint64_t GetEncodedPCIeAddr(void);

    virtual UbioDir GetOpcode(void);

    virtual void* GetBuffer(void);

    virtual uint64_t GetStartByteOffset(void);
    virtual uint64_t GetByteCount(void);

    virtual uint64_t GetStartSectorOffset(void);
    virtual uint64_t GetSectorCount(void);

    virtual void AddPendingErrorCount(uint32_t errorCountToAdd = 1);
    virtual void SubtractPendingErrorCount(uint32_t errorCountToSubtract = 1);

    virtual void CompleteIo(IOErrorType error);
    virtual void SetAsyncIOCompleted(void);
    virtual void ClearAsyncIOCompleted(void);
    virtual bool IsAsyncIOCompleted(void);
    virtual bool CheckAndDecreaseErrorRetryCount();
    virtual void ClearErrorRetryCount(void);

    virtual void IncOutOfMemoryRetryCount(void);
    virtual void ClearOutOfMemoryRetryCount(void);
    virtual uint32_t GetOutOfMemoryRetryCount(void);

    virtual BackendEvent GetEventType(void);

private:
    UbioSmartPtr ubio;
    std::list<IOContext*>::iterator keyForPendingIOList;
    std::list<IOContext*>::iterator keyForPendingErrorList;
    bool keyForPendingIOListSet;
    bool keyForPendingErrorListSet;
    bool completeCalled;
    uint32_t outOfMemoryRetryCount;
    uint32_t remainingErrorRetryCount;
};
} // namespace pos
