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

#include <vector>

#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/include/address_type.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/journal_manager/log_buffer/map_update_log_write_context.h"
#include "src/mapper/include/mpage_info.h"

namespace pos
{
class LogGroupResetContext;
class JournalConfiguration;

class LogWriteContextFactory
{
public:
    LogWriteContextFactory(void);
    virtual ~LogWriteContextFactory(void);

    virtual void Init(JournalConfiguration* config, LogBufferWriteDoneNotifier* target);

    virtual LogWriteContext* CreateBlockMapLogWriteContext(VolumeIoSmartPtr volumeIo, EventSmartPtr callbackEvent);
    virtual LogWriteContext* CreateStripeMapLogWriteContext(Stripe* stripe,
        StripeAddr oldAddr, EventSmartPtr callbackEvent);
    virtual LogWriteContext* CreateGcBlockMapLogWriteContext(
        GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent);
    virtual std::vector<LogWriteContext*> CreateGcBlockMapLogWriteContexts(
        GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent);
    virtual LogWriteContext* CreateGcStripeFlushedLogWriteContext(
        GcStripeMapUpdateList mapUpdates, EventSmartPtr callbackEvent);
    virtual LogWriteContext* CreateVolumeDeletedLogWriteContext(int volId,
        uint64_t contextVersion, EventSmartPtr callback);
    virtual LogGroupResetContext* CreateLogGroupResetContext(uint64_t offset, int id,
        uint64_t groupSize, EventSmartPtr callbackEvent, char* dataBuffer);

    // For UT
    inline LogBufferWriteDoneNotifier* GetLogBufferWriteDoneNotifier(void)
    {
        return notifier;
    }

private:
    uint64_t _GetMaxNumGcBlockMapUpdateInAContext(void);

    JournalConfiguration* config;
    LogBufferWriteDoneNotifier* notifier;
};

} // namespace pos
