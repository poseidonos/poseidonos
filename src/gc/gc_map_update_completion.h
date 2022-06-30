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
#include <map>
#include <string>
#include <vector>

#include "src/include/address_type.h"
#include "src/event_scheduler/callback.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/include/mpage_info.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "src/include/smart_ptr_type.h"

namespace pos
{
class Stripe;
class IStripeMap;
class EventScheduler;
class GcStripeManager;
class IArrayInfo;
class RBAStateManager;
class IVolumeIoManager;

class GcMapUpdateCompletion : public Callback
{
public:
    explicit GcMapUpdateCompletion(StripeSmartPtr stripe, std::string arrayName, IStripeMap* iStripeMap,
                               EventScheduler* eventScheduler, GcStripeManager* gcStripeManager);
    GcMapUpdateCompletion(StripeSmartPtr stripe, std::string arrayName, IStripeMap* iStripeMap,
                        EventScheduler* eventScheduler, GcStripeManager* gcStripeManager,
                        IArrayInfo* inputIArrayInfo,
                        RBAStateManager* inputRbaStateManager,
                        IVolumeIoManager* inputVolumeManager);
    ~GcMapUpdateCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;

    StripeSmartPtr stripe;
    std::string arrayName;
    IStripeMap* iStripeMap;
    EventScheduler* eventScheduler;
    GcStripeManager* gcStripeManager;

    uint32_t totalBlksPerUserStripe;

    IArrayInfo* iArrayInfo;
    RBAStateManager* rbaStateManager;
    IVolumeIoManager* volumeManager;
};

} // namespace pos

