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

#include "src/io/frontend_io/aio.h"

#include "Air.h"
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "src/io/frontend_io/read_submission.h"
#include "src/io/frontend_io/write_submission.h"
#include "src/io/general_io/affinity_manager.h"
#if defined NVMe_FLUSH_HANDLING
#include "src/io/frontend_io/flush_command_handler.h"
#endif
#include "src/device/device_manager.h"
#include "src/device/event_framework_api.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/include/memory.h"
#include "src/logger/logger.h"
#include "src/scheduler/event.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"
#include "src/volume/volume_manager.h"
#if defined QOS_ENABLED_FE
#include "src/qos/qos_manager.h"
#endif
#include "spdk/ibof.h"
#include "src/device/spdk/spdk.hpp"

namespace ibofos
{
IOCtx::IOCtx(void)
: cnt(0),
  needPollingCount(0)
{
}

thread_local IOCtx AIO::ioContext;

AIO::AIO(void)
{
}

VolumeManager& AioCompletion::volumeManager =
    *VolumeManagerSingleton::Instance();

AioCompletion::AioCompletion(VolumeIoSmartPtr volumeIo, ibof_io& ibofIo,
    IOCtx& ioContext)
: Callback(true),
  volumeIo(volumeIo),
  ibofIo(ibofIo),
  ioContext(ioContext)
{
}

AioCompletion::~AioCompletion(void)
{
}

bool
AioCompletion::_DoSpecificJob(void)
{
    uint32_t originCore = volumeIo->GetOriginCore();

    bool keepCurrentReactor = EventFrameworkApi::IsSameReactorNow(originCore);

    if (likely(keepCurrentReactor))
    {
        _SendUserCompletion();
        return true;
    }
    else
    {
        bool success = EventFrameworkApi::SendSpdkEvent(originCore,
            shared_from_this());
        return success;
    }
}

void
AioCompletion::_SendUserCompletion(void)
{
    if (volumeIo->IsPollingNecessary())
    {
        ioContext.needPollingCount--;
    }
    ioContext.cnt--;

    if (ibofIo.complete_cb)
    {
        int status = IBOF_IO_STATUS_SUCCESS;
        if (unlikely(_GetErrorCount() > 0))
        {
            status = IBOF_IO_STATUS_FAIL;
        }

        ibofIo.complete_cb(&ibofIo, status);
    }

#if defined NVMe_FLUSH_HANDLING
    if (volumeIo->dir == UbioDir::Flush)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AIO_FLUSH_END;
        IBOF_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
            eventId, IbofEventId::GetString(eventId),
            ibofIo.volume_id);
    }
    else
#endif
    {
        volumeManager.DecreasePendingIOCount(volumeIo->GetVolumeId());
    }
    volumeIo = nullptr;
}

VolumeIoSmartPtr
AIO::_CreateVolumeIo(ibof_io& ibofIo)
{
    uint64_t sectorSize = ChangeByteToSector(ibofIo.length);
    void* buffer = nullptr;

    if (ibofIo.iov != nullptr)
    {
        buffer = ibofIo.iov->iov_base;
    }

    VolumeIoSmartPtr volumeIo(new VolumeIo(buffer, sectorSize));

    switch (ibofIo.ioType)
    {
#if defined NVMe_FLUSH_HANDLING
        case IO_TYPE::FLUSH:
        {
            volumeIo->dir = UbioDir::Flush;
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AIO_FLUSH_START;
            IBOF_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
                eventId, IbofEventId::GetString(eventId),
                ibofIo.volume_id);
            break;
        }
#endif
        case IO_TYPE::READ:
        {
            volumeIo->dir = UbioDir::Read;
            break;
        }
        case IO_TYPE::WRITE:
        {
            volumeIo->dir = UbioDir::Write;
            break;
        }
        default:
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION;
            IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
            throw eventId;
            break;
        }
    }
    volumeIo->SetVolumeId(ibofIo.volume_id);
    uint64_t sectorRba = ChangeByteToSector(ibofIo.offset);
    volumeIo->SetRba(sectorRba);

    CallbackSmartPtr aioCompletion(new AioCompletion(volumeIo, ibofIo,
        ioContext));

    volumeIo->SetCallback(aioCompletion);

    return volumeIo;
}

void
AIO::SubmitAsyncIO(ibof_io& ibofIo)
{
    VolumeIoSmartPtr volumeIo = _CreateVolumeIo(ibofIo);

#if defined QOS_ENABLED_FE
    QosManager* qosManager = QosManagerSingleton::Instance();
    qosManager->LogVolBw(ibofIo.volume_id, ibofIo.length);
    qosManager->AioSubmitAsyncIO(volumeIo);
    return;
#endif

    ioContext.cnt++;
    if (volumeIo->IsPollingNecessary())
    {
        ioContext.needPollingCount++;
    }

    VolumeManager* volumeManager = VolumeManagerSingleton::Instance();
    switch (volumeIo->dir)
    {
#if defined NVMe_FLUSH_HANDLING
        case UbioDir::Flush:
        {
            uint32_t core = volumeIo->GetOriginCore();
            EventSmartPtr flushCmdHandler =
                std::make_shared<FlushCmdHandler>(volumeIo);
            EventFrameworkApi::ExecuteOrScheduleEvent(core, flushCmdHandler);
            break;
        }
#endif
        case UbioDir::Write:
        {
            AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_WRITE, ibofIo.length);
            volumeManager->IncreasePendingIOCount(volumeIo->GetVolumeId());
            EventFrameworkApi::CreateAndExecuteOrScheduleEvent<WriteSubmission>(
                volumeIo->GetOriginCore(), volumeIo);
            break;
        }
        case UbioDir::Read:
        {
            AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_READ, ibofIo.length);
            volumeManager->IncreasePendingIOCount(volumeIo->GetVolumeId());
            EventFrameworkApi::CreateAndExecuteOrScheduleEvent<ReadSubmission>(
                volumeIo->GetOriginCore(), volumeIo);
            break;
        }
        default:
        {
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION;
            IBOF_TRACE_ERROR(eventId, IbofEventId::GetString(eventId));
            throw eventId;
            break;
        }
    }
}

void
AIO::CompleteIOs(void)
{
    uint32_t aid = EventFrameworkApi::GetCurrentReactor();
    uint32_t size = ioContext.cnt;
    AIRLOG(Q_AIO, aid, size, size);
    if (ioContext.needPollingCount > 0)
    {
        DeviceManagerSingleton::Instance()->HandleCompletedCommand();
    }
}

} // namespace ibofos
