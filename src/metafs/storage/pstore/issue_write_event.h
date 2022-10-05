/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#ifndef _INCLUDE_ISSUE_WRITE_EVENT_INCLUDE_H
#define _INCLUDE_ISSUE_WRITE_EVENT_INCLUDE_H

#include <functional>

#include "src/event_scheduler/event.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/metafs/log/metafs_log.h"
#include "src/metafs/storage/pstore/mss_aio_cb_cxt.h"
#include "src/metafs/storage/pstore/mss_aio_data.h"

namespace pos
{
using MssRequestFunction = std::function<POS_EVENT_ID(const IODirection, MssAioCbCxt*, CallbackSmartPtr, const bool)>;

class IssueWriteEvent : public Event
{
public:
    IssueWriteEvent(MssRequestFunction handler, MssAioCbCxt* cb, CallbackSmartPtr callback)
    : handler(handler),
      cb(cb),
      callback(callback),
      lastResult(EID(SUCCESS))
    {
        SetEventType(BackendEvent_MetaIO);
    }
    virtual ~IssueWriteEvent(void)
    {
    }
    virtual bool Execute(void) override
    {
        POS_EVENT_ID result = handler(IODirection::WRITE, cb, callback, false);

        if (cb && result != EID(SUCCESS))
        {
            if (result != lastResult)
            {
                MssAioData* aioData = reinterpret_cast<MssAioData*>(cb->GetAsycCbCxt());
                if (EID(MFS_IO_FAILED_DUE_TO_STOP_STATE) == result)
                {
                    aioData->SetError((int)result);
                    aioData->SetErrorStopState(true);
                }

                MFS_TRACE_DEBUG(result,
                    "arrayId: {}, metaLpn: {}, storage: {}, mpioId: {}, tagId: {}",
                    aioData->GetArrayId(), aioData->GetMetaLpn(), (int)aioData->GetStorageType(),
                    aioData->GetMpioId(), aioData->GetTagId());

                lastResult = result;
            }
        }

        return _ShouldRetry(result);
    }

private:
    bool _ShouldRetry(const POS_EVENT_ID eventId) const
    {
        if (eventId == EID(SUCCESS) ||
            eventId == EID(MFS_IO_FAILED_DUE_TO_STOP_STATE))
        {
            return true;
        }

        return false;
    }

    ChangeLogger<int> changeLogger;
    MssRequestFunction handler;
    MssAioCbCxt* cb;
    CallbackSmartPtr callback;
    POS_EVENT_ID lastResult;
};
} // namespace pos

#endif // _INCLUDE_ISSUE_WRITE_EVENT_INCLUDE_H
