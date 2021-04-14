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

#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "Air.h"
#include "spdk/ibof.h"
#include "src/array_mgmt/array_manager.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/device_manager.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/io/frontend_io/flush_command_handler.h"
#include "src/io/frontend_io/read_submission.h"
#include "src/io/frontend_io/write_submission.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/volume/volume_manager.h"
#include "src/volume/volume_service.h"

#ifdef _ADMIN_ENABLED
#include "src/admin/admin_command_handler.h"
#include "src/io_scheduler/io_dispatcher.h"
#endif

namespace pos
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

VolumeService& AioCompletion::volumeService =
    *VolumeServiceSingleton::Instance();

AioCompletion::AioCompletion(FlushIoSmartPtr flushIo, ibof_io& ibofIo, IOCtx& ioContext)
: AioCompletion(flushIo, ibofIo, ioContext,
      EventFrameworkApi::IsSameReactorNow)
{
}

AioCompletion::AioCompletion(FlushIoSmartPtr flushIo, ibof_io& ibofIo,
    IOCtx& ioContext, std::function<bool(uint32_t)> isSameReactorNowFunc)
: Callback(true),
  flushIo(flushIo),
  volumeIo(nullptr),
  ibofIo(ibofIo),
  ioContext(ioContext),
  isSameReactorNowFunc(isSameReactorNowFunc)
{
}

AioCompletion::AioCompletion(VolumeIoSmartPtr volumeIo, ibof_io& ibofIo, IOCtx& ioContext)
: AioCompletion(volumeIo, ibofIo, ioContext,
      EventFrameworkApi::IsSameReactorNow)
{
}

AioCompletion::AioCompletion(VolumeIoSmartPtr volumeIo, ibof_io& ibofIo,
    IOCtx& ioContext, std::function<bool(uint32_t)> isSameReactorNowFunc)
: Callback(true),
  flushIo(nullptr),
  volumeIo(volumeIo),
  ibofIo(ibofIo),
  ioContext(ioContext),
  isSameReactorNowFunc(isSameReactorNowFunc)
{
}

AioCompletion::~AioCompletion(void)
{
}

bool
AioCompletion::_DoSpecificJob(void)
{
    uint32_t originCore;

    if (ibofIo.ioType == IO_TYPE::FLUSH)
    {
        originCore = flushIo->GetOriginCore();
    }
    else
    {
        originCore = volumeIo->GetOriginCore();
    }

    bool keepCurrentReactor = isSameReactorNowFunc(originCore);

    if (likely(keepCurrentReactor))
    {
        _SendUserCompletion();
        return true;
    }
    else
    {
        bool success = SpdkEventScheduler::SendSpdkEvent(originCore,
            shared_from_this());
        return success;
    }
}

void
AioCompletion::_SendUserCompletion(void)
{
    int dir = ibofIo.ioType;
    if (dir != IO_TYPE::FLUSH)
    {
        if (volumeIo->IsPollingNecessary())
        {
            ioContext.needPollingCount--;
        }
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

    if (dir == IO_TYPE::FLUSH)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::AIO_FLUSH_END;
        POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
            eventId, PosEventId::GetString(eventId),
            ibofIo.volume_id);
    }
    else
    {
        IVolumeManager* volumeManager = volumeService.GetVolumeManager(volumeIo->GetArrayName());
        volumeManager->DecreasePendingIOCount(volumeIo->GetVolumeId());
    }
    volumeIo = nullptr;
    flushIo = nullptr;
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

    std::string arrayName(ibofIo.arrayName);
    VolumeIoSmartPtr volumeIo(new VolumeIo(buffer, sectorSize, arrayName));

    switch (ibofIo.ioType)
    {
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
            POS_EVENT_ID eventId = POS_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION;
            POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId));
            throw eventId;
            break;
        }
    }
    volumeIo->SetVolumeId(ibofIo.volume_id);
    uint64_t sectorRba = ChangeByteToSector(ibofIo.offset);
    volumeIo->SetSectorRba(sectorRba);

    CallbackSmartPtr aioCompletion(new AioCompletion(volumeIo, ibofIo,
        ioContext));

    volumeIo->SetCallback(aioCompletion);

    return volumeIo;
}

FlushIoSmartPtr
AIO::_CreateFlushIo(ibof_io& ibofIo)
{
    std::string arrayName(ibofIo.arrayName);
    FlushIoSmartPtr flushIo(new FlushIo(arrayName));
    flushIo->SetVolumeId(ibofIo.volume_id);

    POS_EVENT_ID eventId = POS_EVENT_ID::AIO_FLUSH_START;
    POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
        eventId, PosEventId::GetString(eventId),
        ibofIo.volume_id);

    CallbackSmartPtr aioCompletion(new AioCompletion(flushIo, ibofIo,
        ioContext));
    flushIo->SetCallback(aioCompletion);

    return flushIo;
}

void
AIO::SubmitAsyncIO(ibof_io& ibofIo)
{
    if (ibofIo.ioType == IO_TYPE::FLUSH)
    {
        FlushIoSmartPtr flushIo = _CreateFlushIo(ibofIo);
        ioContext.cnt++;

        SpdkEventScheduler::ExecuteOrScheduleEvent(flushIo->GetOriginCore(), std::make_shared<FlushCmdHandler>(flushIo));
        return;
    }

    VolumeIoSmartPtr volumeIo = _CreateVolumeIo(ibofIo);

    ioContext.cnt++;
    if (volumeIo->IsPollingNecessary())
    {
        ioContext.needPollingCount++;
    }

    IVolumeManager* volumeManager
        = VolumeServiceSingleton::Instance()->GetVolumeManager(volumeIo->GetArrayName());

    uint32_t core = volumeIo->GetOriginCore();
    switch (volumeIo->dir)
    {
        case UbioDir::Write:
        {
            AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_WRITE, ibofIo.length);
            volumeManager->IncreasePendingIOCount(volumeIo->GetVolumeId());
            SpdkEventScheduler::ExecuteOrScheduleEvent(core,
                std::make_shared<WriteSubmission>(volumeIo));
            break;
        }
        case UbioDir::Read:
        {
            AIRLOG(PERF_VOLUME, ibofIo.volume_id, AIR_READ, ibofIo.length);
            volumeManager->IncreasePendingIOCount(volumeIo->GetVolumeId());
            SpdkEventScheduler::ExecuteOrScheduleEvent(core,
                std::make_shared<ReadSubmission>(volumeIo));
            break;
        }
        default:
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION;
            POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId));
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
#ifdef _ADMIN_ENABLED
void
AIO::SubmitAsyncAdmin(ibof_io& io)
{
    if (io.ioType == GET_LOG_PAGE)
    {
        void* bio = io.context;
        struct spdk_bdev_io* bioIbof = (struct spdk_bdev_io*)bio;
        void* callerContext = bioIbof->internal.caller_ctx;

        struct spdk_nvmf_request* req = (struct spdk_nvmf_request*)callerContext;

        struct spdk_nvme_cmd* cmd = &req->cmd->nvme_cmd;
        uint8_t lid = cmd->cdw10 & 0xFF;
        if (lid == SPDK_NVME_LOG_HEALTH_INFORMATION)
        {
            ioContext.needPollingCount++;
        }
    }
    CallbackSmartPtr adminCompletion(new AdminCompletion(&io, ioContext));
    uint32_t originCore = EventFrameworkApi::GetCurrentReactor();
    string arrayName = "POSArray";
    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    IDevInfo* devmgr = DeviceManagerSingleton::Instance();
    IIODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
    IArrayDevMgr* arrayDevMgr = info->GetArrayManager();
    EventSmartPtr event(new AdminCommandHandler(&io, originCore, adminCompletion, info, devmgr, ioDispatcher, arrayDevMgr));
    EventSchedulerSingleton::Instance()->EnqueueEvent(event);
    return;
}

AdminCompletion::AdminCompletion(ibof_io* ibofIo, IOCtx& ioContext)
: Callback(false),
  io(ibofIo),
  ioContext(ioContext)
{
}
AdminCompletion::~AdminCompletion(void)
{
}

bool
AdminCompletion::_DoSpecificJob(void)
{
    if (ioContext.needPollingCount > 0)
    {
        ioContext.needPollingCount--;
    }
    io->complete_cb(io, IBOF_IO_STATUS_SUCCESS);
    return true;
}

#endif
} // namespace pos
