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

#include "src/io/frontend_io/block_map_update_completion.h"

#include "src/event_scheduler/event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/frontend_io/write_completion.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

#include <memory>

namespace pos
{
BlockMapUpdateCompletion::BlockMapUpdateCompletion(
    VolumeIoSmartPtr input, CallbackSmartPtr originCallback,
    bool isReactorNow, EventScheduler* eventScheduler, CallbackSmartPtr writeCompletionEvent)
: Callback(isReactorNow, CallbackType_BlockMapUpdateRequestCompletion),
  volumeIo(input),
  originCallback(originCallback),
  eventScheduler(eventScheduler),
  writeCompletionEvent(writeCompletionEvent)
{
}

BlockMapUpdateCompletion::BlockMapUpdateCompletion(VolumeIoSmartPtr inputVolumeIo,
    CallbackSmartPtr originCallback)
: BlockMapUpdateCompletion(
    inputVolumeIo, originCallback, EventFrameworkApiSingleton::Instance()->IsReactorNow(),
    EventSchedulerSingleton::Instance(),
    std::make_shared<WriteCompletion>(inputVolumeIo))
{
}

BlockMapUpdateCompletion::~BlockMapUpdateCompletion(void)
{
}

bool
BlockMapUpdateCompletion::_DoSpecificJob(void)
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

    writeCompletionEvent->SetCallee(callee);
    bool wrapupSuccessful = writeCompletionEvent->Execute();

    if (unlikely(false == wrapupSuccessful))
    {
        writeCompletionEvent->SetCallee(callee);
        eventScheduler->EnqueueEvent(writeCompletionEvent);
    }
    volumeIo = nullptr;

    return true;
}

} // namespace pos
