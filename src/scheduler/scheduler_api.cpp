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

#include "src/scheduler/scheduler_api.h"

#include <vector>

#include "spdk/ibof.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/frontend_io/aio.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"

using namespace ibofos;
using namespace std;

void
UNVMfCompleteHandler(void)
{
    try
    {
        AIO aio;
        aio.CompleteIOs();
    }
    catch (...)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::SCHEDAPI_COMPLETION_POLLING_FAIL;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));
    }
}

int
UNVMfSubmitHandler(struct ibof_io* io)
{
    try
    {
        if (unlikely(nullptr == io))
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::SCHEDAPI_NULL_COMMAND;
            IBOF_TRACE_ERROR(static_cast<int>(eventId),
                IbofEventId::GetString(eventId));
            throw eventId;
        }

#if defined NVMe_FLUSH_HANDLING
        if (io->ioType != IO_TYPE::FLUSH)
#endif
        {
            if (unlikely(1 != io->iovcnt))
            {
                IBOF_EVENT_ID eventId = IBOF_EVENT_ID::SCHEDAPI_WRONG_BUFFER;
                IBOF_TRACE_ERROR(static_cast<int>(eventId),
                    IbofEventId::GetString(eventId));
                throw eventId;
            }
        }

        AIO aio;
        aio.SubmitAsyncIO(*io);
    }
    catch (...)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::SCHEDAPI_SUBMISSION_FAIL;
        IBOF_TRACE_ERROR(static_cast<int>(eventId),
            IbofEventId::GetString(eventId));

        if (nullptr != io->complete_cb)
        {
            io->complete_cb(io, IBOF_IO_STATUS_FAIL);
        }
    }

    return IBOF_IO_STATUS_SUCCESS;
}
