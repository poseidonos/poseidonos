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

#pragma once

#include <list>
#include <string>
#include <utility>

#include "src/dump/dump_shared_ptr.h"
#include "src/dump/dump_shared_ptr.hpp"
#include "src/include/io_error_type.h"
#include "src/bio/ubio.h"

namespace pos
{
enum class UbioDir;
class Ubio;

class IOContext : public DumpSharedPtr<IOContext*, static_cast<int>(DumpSharedPtrType::IO_CONTEXT)>
{
public:
    IOContext(void) = delete;
    explicit IOContext(UbioSmartPtr inputUbio, uint32_t inputRetry);
    virtual ~IOContext(void);

    void SetIOKey(std::list<IOContext*>::iterator it);
    std::pair<std::list<IOContext*>::iterator, bool> GetIOKey(void);

    void SetErrorKey(std::list<IOContext*>::iterator it);
    std::pair<std::list<IOContext*>::iterator, bool> GetErrorKey(void);

    std::string GetDeviceName(void);

    UbioDir GetOpcode(void);

    void* GetBuffer(void);

    uint64_t GetStartByteOffset(void);
    uint64_t GetByteCount(void);

    uint64_t GetStartSectorOffset(void);
    uint64_t GetSectorCount(void);

    void AddPendingErrorCount(uint32_t errorCountToAdd = 1);
    void SubtractPendingErrorCount(uint32_t errorCountToSubtract = 1);

    bool CheckErrorDisregard(void);

    void CompleteIo(IOErrorType error);
    void SetAsyncIOCompleted(void);
    void ClearAsyncIOCompleted(void);
    bool IsAsyncIOCompleted(void);
    bool CheckAndDecreaseRetryCount();
    void ClearRetryCount(void);

    void IncSubmitRetryCount(void);
    void ClearSubmitRetryCount(void);
    uint32_t GetSubmitRetryCount(void);
    void SetAdminCommand(void);

    Ubio* GetUbio(void);

private:
    UbioSmartPtr ubio;
    std::list<IOContext*>::iterator keyForPendingIOList;
    std::list<IOContext*>::iterator keyForPendingErrorList;
    bool keyForPendingIOListSet;
    bool keyForPendingErrorListSet;
    bool completeCalled;
    uint32_t submitRetryCount;
    uint32_t retryCount;
};
} // namespace pos
