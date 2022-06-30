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

#include <map>

#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_segment_ctx.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/journal_manager/i_journal_manager.h"
#include "src/journal_manager/i_journal_writer.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/meta_service/i_meta_updater.h"

namespace pos
{
class EventScheduler;
class MetaEventFactory;
class GcStripeManager;
class IArrayInfo;
class IContextManager;

class MetaUpdater : public IMetaUpdater
{
public:
    MetaUpdater(IStripeMap* stripeMap,
        IJournalManager* journal, IJournalWriter* journalWriter,
        EventScheduler* eventScheduler,
        MetaEventFactory* eventFactory, IArrayInfo* arrayInfo);
    virtual ~MetaUpdater(void);

    virtual int UpdateBlockMap(VolumeIoSmartPtr volumeIo, CallbackSmartPtr callback) override;
    virtual int UpdateStripeMap(Stripe* stripe, CallbackSmartPtr callback) override;
    virtual int UpdateGcMap(StripeSmartPtr stripe, GcStripeMapUpdateList mapUpdateInfoList, std::map<SegmentId, uint32_t> invalidSegCnt, CallbackSmartPtr callback) override;

private:
    IStripeMap* stripeMap;
    IJournalManager* journal;
    IJournalWriter* journalWriter;
    EventScheduler* eventScheduler;
    MetaEventFactory* metaEventFactory;
    IArrayInfo* arrayInfo;
};
} // namespace pos
