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

#include "src/array/ft/buffer_entry.h"
#include "src/array/service/io_locker/i_io_locker.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"
#include "src/include/pos_event_id.h"
#include "src/include/smart_ptr_type.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_submit_interface/io_submit_handler_status.h"

namespace pos
{
class SubmitAsyncWrite
{
public:
    SubmitAsyncWrite(void);
    SubmitAsyncWrite(IIOTranslator* translator, IODispatcher* ioDispatcher);
    ~SubmitAsyncWrite(void);
    IOSubmitHandlerStatus Execute(std::list<BufferEntry>& bufferList,
        LogicalBlkAddr& startLSA, uint64_t blockCount,
        PartitionType partitionToIO, CallbackSmartPtr callback,
        int arrayId, bool needTrim, bool parityOnly = false);

private:
    IIOTranslator* translator;
    IODispatcher* ioDispatcher;

    IOSubmitHandlerStatus _CheckAsyncWriteError(int arrayId);
    UbioSmartPtr _SetupUbio(int arrayId, bool needTrim, BufferEntry& buffer,
        PhysicalBlkAddr addr, CallbackSmartPtr arrayUnlocking, CallbackSmartPtr callback);
};
} // namespace pos
