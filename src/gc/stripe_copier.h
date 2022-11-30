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
#include <map>

#include "src/gc/copier_meta.h"
#include "src/gc/victim_stripe.h"

namespace pos
{
class EventScheduler;
class IIOSubmitHandler;

class StripeCopier : public Event
{
public:
    explicit StripeCopier(StripeId victimStripeId, CopierMeta* meta, uint32_t copyIndex);
    StripeCopier(StripeId victimStripeId, CopierMeta* meta, uint32_t copyIndex,
                EventSmartPtr inputCopyEvent, EventSmartPtr inputStripeCopier,
                EventScheduler* inputEventScheduler);
    virtual ~StripeCopier(void);
    virtual bool Execute(void);

private:
    class CopyEvent : public Event
    {
    public:
        explicit CopyEvent(void* buffer,
            LogicalBlkAddr lsa,
            uint32_t listIndex,
            CopierMeta* meta,
            uint32_t stripeId,
            uint32_t copyIndex);

        CopyEvent(void* buffer,
            LogicalBlkAddr lsa,
            uint32_t listIndex,
            CopierMeta* meta,
            uint32_t stripeId,
            uint32_t copyIndex,
            CallbackSmartPtr inputCallback,
            IIOSubmitHandler* inputIIOSubmitHandler);
        virtual ~CopyEvent(void);
        virtual bool Execute(void);

    private:
        void* buffer;
        LogicalBlkAddr lsa;
        uint32_t listIndex;
        CopierMeta* meta;
        uint32_t stripeId;
        uint32_t copyIndex;

        CallbackSmartPtr inputCallback;
        IIOSubmitHandler* iIOSubmitHandler;
    };

    StripeId victimStripeId;
    CopierMeta* meta;

    bool loadedValidBlock;
    uint32_t listIndex;
    uint32_t stripeOffset;
    uint32_t copyIndex;

    EventSmartPtr inputCopyEvent;
    EventSmartPtr inputStripeCopier;
    EventScheduler* eventScheduler;
    uint32_t bufAllocRetryCnt = 0;
};

} // namespace pos
