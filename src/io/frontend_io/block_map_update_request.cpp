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

#include "src/io/frontend_io/block_map_update_request.h"

#include "mk/ibof_config.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/meta_service/meta_service.h"
#include "src/io/frontend_io/block_map_update_completion.h"

#include <memory>

namespace pos
{
BlockMapUpdateRequest::BlockMapUpdateRequest(VolumeIoSmartPtr volumeIo, CallbackSmartPtr originCallback)
: BlockMapUpdateRequest(volumeIo, originCallback, std::make_shared<BlockMapUpdateCompletion>(volumeIo, originCallback),
    MetaServiceSingleton::Instance()->GetMetaUpdater(volumeIo->GetArrayId()),
    EventSchedulerSingleton::Instance(), EventFrameworkApiSingleton::Instance()->IsReactorNow())
{
}

BlockMapUpdateRequest::BlockMapUpdateRequest(VolumeIoSmartPtr volumeIo, CallbackSmartPtr originCallback,
    CallbackSmartPtr blockMapUpdateCompletionEvent, IMetaUpdater* metaUpdater,
    EventScheduler* eventScheduler, bool isReactorNow)
: Callback(isReactorNow, CallbackType_BlockMapUpdateRequest),
  volumeIo(volumeIo),
  originCallback(originCallback),
  blockMapUpdateCompletionEvent(blockMapUpdateCompletionEvent),
  metaUpdater(metaUpdater),
  eventScheduler(eventScheduler)
{
}

BlockMapUpdateRequest::~BlockMapUpdateRequest(void)
{
}

bool
BlockMapUpdateRequest::_DoSpecificJob(void)
{
    bool result = true;
    try
    {
        if (unlikely(_GetErrorCount() > 0))
        {
            POS_EVENT_ID eventId = EID(WRCMP_IO_ERROR);
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Write is failed at WriteCompleting state");
            throw eventId;
        }
        result = _UpdateMeta();
    }
    catch (...)
    {
        CallbackSmartPtr callee;
        if (originCallback == nullptr)
        {
            callee = volumeIo->GetOriginUbio()->GetCallback();
        }
        else
        {
            callee = originCallback;
        }
        this->SetCallee(callee);
        return true;
    }

    if (result == true)
    {
        volumeIo = nullptr;
    }
    return result;
}

bool
BlockMapUpdateRequest::_UpdateMeta(void)
{
    if (likely(blockMapUpdateCompletionEvent != nullptr))
    {
        int result = metaUpdater->UpdateBlockMap(volumeIo, blockMapUpdateCompletionEvent);
        if (unlikely(result != 0))
        {
            POS_TRACE_ERROR_CONDITIONALLY(&changeLogger, result, result, "Write wraup failed at map update ");
            return false;
        }
    }
    else
    {
        POS_EVENT_ID eventId = EID(WRWRAPUP_EVENT_ALLOC_FAILED);
        POS_TRACE_ERROR(static_cast<int>(eventId), "Flush Event allocation failed at WriteWrapup state");
        eventScheduler->EnqueueEvent(blockMapUpdateCompletionEvent);
    }
    return true;
}

} // namespace pos
