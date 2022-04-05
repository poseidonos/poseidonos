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

#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/event.h"
#include "src/io_submit_interface/i_io_submit_handler.h"

namespace pos
{
class Stripe;
class IWBStripeAllocator;
class IIOSubmitHandler;
class IIOTranslator;

class FlushSubmission : public Event
{
public:
    FlushSubmission(Stripe* inputStripe, int arrayId, bool isWTEnabled = false);
    FlushSubmission(Stripe* inputStripe, IIOSubmitHandler* ioSubmitHandler, int arrayId,
        IArrayInfo* arrayInfo, IIOTranslator* translator, bool isWTEnabled);
    ~FlushSubmission(void) override;
    bool Execute(void) override;
    uint32_t GetBufferListSize(void);

private:
    Stripe* stripe;
    IIOSubmitHandler* iIOSubmitHandler;
    std::list<BufferEntry> bufferList;
    int arrayId;
    IIOTranslator* translator;
    bool isWTEnabled;
    const PartitionLogicalSize* udSize;
};

} // namespace pos
