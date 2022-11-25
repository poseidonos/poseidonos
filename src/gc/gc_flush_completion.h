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

#include <atomic>
#include <cstdint>
#include <string>
#include <list>

#include "src/event_scheduler/callback.h"
#include "src/gc/gc_stripe_manager.h"
#include "src/include/smart_ptr_type.h"
#include "src/include/address_type.h"
#include "src/mapper/mapper.h"
#include "src/mapper_service/mapper_service.h"

using namespace std;

namespace pos
{
class GcStripeManager;
class RBAStateManager;
class IArrayInfo;

struct VictimRba
{
    uint64_t rba;
    uint32_t offset;
    list<RbaAndSize>::iterator iter;
};

class GcFlushCompletion : public Callback
{
public:
    explicit GcFlushCompletion(StripeSmartPtr stripe, string arrayName, GcStripeManager* gcStripeManager, GcWriteBuffer* dataBuffer);
    GcFlushCompletion(StripeSmartPtr stripe, string arrayName, GcStripeManager* gcStripeManager, GcWriteBuffer* dataBuffer,
                    EventSmartPtr inputEvent,
                    RBAStateManager* inputRbaStateManager,
                    IArrayInfo* inputIArrayInfo,
                    IVSAMap* iVSAMap);
    ~GcFlushCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    void _Init(void);
    bool _IsValidRba(BlkAddr rba, uint32_t offset);
    void _RemoveInvalidRba(void);
    bool _AcquireOwnership(void);

    StripeSmartPtr stripe;
    string arrayName;
    GcStripeManager* gcStripeManager;
    GcWriteBuffer* dataBuffer;

    EventSmartPtr inputEvent;
    RBAStateManager* rbaStateManager;
    IArrayInfo* iArrayInfo;
    IVSAMap* iVSAMap;
    list<VictimRba> victimBlockList;
    list<RbaAndSize> sectorRbaList;
    uint32_t volId = 0;
    uint32_t retryCnt = 0;
    bool isInit = false;
    StripeId lsid = 0;
    uint32_t totalBlksPerUserStripe = 0;
};
} // namespace pos
