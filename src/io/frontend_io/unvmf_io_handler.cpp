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
#include "src/volume/volume_manager.h"
#include "src/event_scheduler/io_completer.h"

#include "src/io/frontend_io/unvmf_io_handler.h"

#include <vector>

#include "spdk/pos.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/branch_prediction.h"
#include "src/io/frontend_io/aio.h"
#include "src/bio/ubio.h"
#include "src/logger/logger.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/qos/qos_manager.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
using namespace pos;
using namespace std;

void
UNVMfCompleteHandler(void)
{
    try
    {
        AIO aio;
        aio.CompleteIOs();
        EventFrameworkApiSingleton::Instance()->CompleteEvents();
    }
    catch (...)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_COMPLETION_POLLING_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Fail to poll ibof completion");
    }
}

int
UNVMfSubmitHandler(struct pos_io* io)
{
    try
    {
        if (unlikely(nullptr == io))
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_NULL_COMMAND;
            POS_TRACE_ERROR(static_cast<int>(eventId),
                "Command from bdev is empty");
            throw eventId;
        }
        if (io->ioType > IO_TYPE::ADMIN)
        {
            AIO aio;
            aio.SubmitAsyncAdmin(*io);
            return POS_IO_STATUS_SUCCESS;
        }
        switch (io->ioType)
        {
            case IO_TYPE::READ:
            case IO_TYPE::WRITE:
            {
                if (unlikely(1 != io->iovcnt))
                {
                    POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_WRONG_BUFFER;
                    POS_TRACE_ERROR(static_cast<int>(eventId),
                            "Single IO command should have a continuous buffer");
                    throw eventId;
                }
                break;
            }
            case IO_TYPE::FLUSH:
            {
                AIO aio;
                aio.SubmitFlush(*io);
                return POS_IO_STATUS_SUCCESS;
            }
            break;
            default:
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION;
                POS_TRACE_ERROR(eventId, "Wrong IO direction (only read/write types are suppoered)");
                throw eventId;
                break;
            }
        }

        QosManager* qosManager = QosManagerSingleton::Instance();

        IVolumeManager* volumeManager
            = VolumeServiceSingleton::Instance()->GetVolumeManager(io->array_id);

        AIO aio;
        VolumeIoSmartPtr volumeIo = aio.CreateVolumeIo(*io);
        if (unlikely(EID(SUCCESS) != volumeManager->IncreasePendingIOCountIfNotZero(io->volume_id)))
        {
            IoCompleter ioCompleter(volumeIo);
            ioCompleter.CompleteUbioWithoutRecovery(IOErrorType::VOLUME_UMOUNTED, true);
            return POS_IO_STATUS_SUCCESS;
        }

        if (true == qosManager->IsFeQosEnabled())
        {
            AioSubmissionAdapter aioSubmission;
            qosManager->HandlePosIoSubmission(&aioSubmission, volumeIo);
        }
        else
        {
            aio.SubmitAsyncIO(volumeIo);
        }
    }
    catch (...)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::SCHEDAPI_SUBMISSION_FAIL;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            "Fail to submit ibof IO");

        if (nullptr != io && nullptr != io->complete_cb)
        {
            io->complete_cb(io, POS_IO_STATUS_FAIL);
        }
    }

    return POS_IO_STATUS_SUCCESS;
}
