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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/io/frontend_io/unvmf_io_handler.h"

#include <vector>

#include "spdk/ibof.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/aio.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/spdk_wrapper/event_framework_api.h"
#if defined QOS_ENABLED_FE
#include "src/qos/qos_manager.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#endif
using namespace pos;
using namespace std;

void
UNVMfCompleteHandler(void)
{
    try
    {
        AIO aio;
        aio.CompleteIOs();
        EventFrameworkApi::CompleteEvents();
    }
    catch (...)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_COMPLETION_POLLING_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));
    }
}

int
UNVMfSubmitHandler(struct ibof_io* io)
{
    try
    {
        if (unlikely(nullptr == io))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_NULL_COMMAND;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                PosEventId::GetString(eventId));
            throw eventId;
        }
#ifdef _ADMIN_ENABLED
        if (io->ioType > IO_TYPE::ADMIN)
        {
            AIO aio;
            aio.SubmitAsyncAdmin(*io);
            return IBOF_IO_STATUS_SUCCESS;
        }
#endif
        if (io->ioType != IO_TYPE::FLUSH)
        {
            if (unlikely(1 != io->iovcnt))
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_WRONG_BUFFER;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    PosEventId::GetString(eventId));
                throw eventId;
            }
        }

#if defined QOS_ENABLED_FE
        QosManagerSingleton::Instance()->LogVolBw(io->volume_id, io->length);
        AioSubmissionAdapter aioSubmission;
        QosManagerSingleton::Instance()->AioSubmitAsyncIO(&aioSubmission, io);
#else
        AIO aio;
        aio.SubmitAsyncIO(*io);
#endif
    }
    catch (...)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_SUBMISSION_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId));

        if (nullptr != io && nullptr != io->complete_cb)
        {
            io->complete_cb(io, IBOF_IO_STATUS_FAIL);
        }
    }

    return IBOF_IO_STATUS_SUCCESS;
}
