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

#include <cstdint>
#include <list>
#include <string>

#include "src/include/partition_type.h"
#include "src/include/address_type.h"
#include "src/include/smart_ptr_type.h"
#include "src/array/ft/buffer_entry.h"
#include "src/io_submit_interface/io_submit_handler_status.h"

namespace pos
{
enum class IODirection
{
    WRITE,
    READ,
    TRIM,
};

class IIOSubmitHandler
{
public:
    IIOSubmitHandler(void);
    virtual ~IIOSubmitHandler(void);

    static void
    RegisterInstance(IIOSubmitHandler* ioSubmitHandlerModule);

    static IIOSubmitHandler* GetInstance(void);

    virtual IOSubmitHandlerStatus
    SyncIO(IODirection direction,
        std::list<BufferEntry>& bufferList,
        LogicalBlkAddr& startLSA, uint64_t blockCount,
        PartitionType partitionToIO, int arrayId) = 0;

    virtual IOSubmitHandlerStatus
    SubmitAsyncIO(IODirection direction,
        std::list<BufferEntry>& bufferList,
        LogicalBlkAddr& startLSA, uint64_t blockCount,
        PartitionType partitionToIO,
        CallbackSmartPtr callback, int arrayId, bool pairtyOnly = false) = 0;

    virtual IOSubmitHandlerStatus
    SubmitAsyncByteIO(IODirection direction,
        void* buffer,
        LogicalByteAddr& startLSA,
        PartitionType partitionToIO,
        CallbackSmartPtr callback, int arrayId) = 0;

private:
    static IIOSubmitHandler* instance;
}; // class IOSubmitHandler

} // namespace pos
