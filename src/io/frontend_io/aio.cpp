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

#include "src/io/frontend_io/aio.h"

#include <air/Air.h>
#include <string.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "spdk/pos.h"
#include "src/admin/admin_command_handler.h"
#include "src/array_mgmt/array_manager.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/device/device_manager.h"
#include "src/debug_lib/debug_info_queue.h"
#include "src/debug_lib/debug_info_queue.hpp"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/io/frontend_io/flush_command_handler.h"
#include "src/io/frontend_io/read_submission.h"
#include "src/io/frontend_io/write_submission.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
#include "src/pos_replicator/posreplicator_manager.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "src/spdk_wrapper/spdk.h"
#include "src/volume/volume_manager.h"
#include "src/volume/volume_service.h"

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

AioCompletion::AioCompletion(FlushIoSmartPtr flushIo, pos_io& posIo, IOCtx& ioContext)
: AioCompletion(flushIo, posIo, ioContext, EventFrameworkApiSingleton::Instance())
{
}

AioCompletion::AioCompletion(FlushIoSmartPtr flushIo, pos_io& posIo,
    IOCtx& ioContext, EventFrameworkApi* eventFrameworkApi)
: Callback(true, CallbackType_AioCompletion),
  flushIo(flushIo),
  volumeIo(nullptr),
  posIo(posIo),
  ioContext(ioContext),
  eventFrameworkApi(eventFrameworkApi)
{
}

AioCompletion::AioCompletion(VolumeIoSmartPtr volumeIo, pos_io& posIo, IOCtx& ioContext)
: AioCompletion(volumeIo, posIo, ioContext, EventFrameworkApiSingleton::Instance())
{
}

AioCompletion::AioCompletion(VolumeIoSmartPtr volumeIo, pos_io& posIo,
    IOCtx& ioContext, EventFrameworkApi* eventFrameworkApi)
: Callback(true, CallbackType_AioCompletion),
  flushIo(nullptr),
  volumeIo(volumeIo),
  posIo(posIo),
  ioContext(ioContext),
  eventFrameworkApi(eventFrameworkApi)
{
}

AioCompletion::~AioCompletion(void)
{
}

bool
AioCompletion::_DoSpecificJob(void)
{
    uint32_t originCore;

    if (posIo.ioType == IO_TYPE::FLUSH)
    {
        originCore = flushIo->GetOriginCore();
    }
    else
    {
        originCore = volumeIo->GetOriginCore();
        if (volumeIo->IsPollingNecessary())
        {
            ioContext.needPollingCount--;
        }
    }

    bool keepCurrentReactor = eventFrameworkApi->IsSameReactorNow(originCore);

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
    int dir = posIo.ioType;
    ioContext.cnt--;
    uint32_t volumeId = posIo.volume_id;
    if (posIo.complete_cb)
    {
        int status = POS_IO_STATUS_SUCCESS;
        if (unlikely(_GetErrorCount() > 0))
        {
            status = POS_IO_STATUS_FAIL;
        }

        posIo.complete_cb(&posIo, status);
    }

    if (dir == IO_TYPE::FLUSH)
    {
        POS_EVENT_ID eventId = EID(AIO_FLUSH_END);
        POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
            eventId, "Flush End in Aio, volume id : {}",
            volumeId);
    }
    else
    {
        IVolumeIoManager* volumeManager = volumeService.GetVolumeManager(volumeIo->GetArrayId());
        if (likely(_GetMostCriticalError() != IOErrorType::VOLUME_UMOUNTED))
        {
            volumeManager->DecreasePendingIOCount(volumeIo->GetVolumeId(), static_cast<VolumeIoType>(dir));
            airlog("UserWritePendingCnt", "user", volumeIo->GetVolumeId(), -1);
            airlog("UserReadPendingCnt", "user", volumeIo->GetVolumeId(), -1);
        }
    }
    volumeIo = nullptr;
    flushIo = nullptr;
}

VolumeIoSmartPtr
AIO::_CreateVolumeIo(pos_io& posIo)
{
    uint64_t sectorSize = ChangeByteToSector(posIo.length);
    void* buffer = nullptr;

    if (posIo.iov != nullptr)
    {
        buffer = posIo.iov->iov_base;
    }

    int arrayId(posIo.array_id);
    VolumeIoSmartPtr volumeIo(new VolumeIo(buffer, sectorSize, arrayId));

    switch (posIo.ioType)
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
            POS_EVENT_ID eventId = EID(BLKHDLR_WRONG_IO_DIRECTION);
            POS_TRACE_ERROR(eventId, "Wrong IO direction (only read/write types are suppoered)");
            throw eventId;
            break;
        }
    }
    volumeIo->SetVolumeId(posIo.volume_id);
    uint64_t sectorRba = ChangeByteToSector(posIo.offset);
    volumeIo->SetSectorRba(sectorRba);
    volumeIo->SetEventType(BackendEvent::BackendEvent_FrontendIO);

    return volumeIo;
}

void
AIO::_IncreaseIoContextCnt(bool needPollingNecessary)
{
    ioContext.cnt++;
    if (needPollingNecessary)
    {
        ioContext.needPollingCount++;
    }
}

VolumeIoSmartPtr
AIO::CreateVolumeIo(pos_io& posIo)
{
    VolumeIoSmartPtr volumeIo = _CreateVolumeIo(posIo);
    CallbackSmartPtr aioCompletion(new AioCompletion(volumeIo, posIo,
        ioContext));
    volumeIo->SetCallback(aioCompletion);

    _IncreaseIoContextCnt(volumeIo->IsPollingNecessary());

    return volumeIo;
}

VolumeIoSmartPtr
AIO::CreatePosReplicatorVolumeIo(pos_io& posIo, uint64_t lsn)
{
    VolumeIoSmartPtr volumeIo = _CreateVolumeIo(posIo);

    if (lsn != REPLICATOR_INVALID_LSN)
    {
        CallbackSmartPtr aioCompletion(new AioCompletion(volumeIo, posIo,
            ioContext));
        volumeIo->SetCallback(aioCompletion);
        _IncreaseIoContextCnt(volumeIo->IsPollingNecessary());
    }
    else
    {
        volumeIo->SetCallback(nullptr);
    }

    return volumeIo;
}

FlushIoSmartPtr
AIO::_CreateFlushIo(pos_io& posIo)
{
    int arrayId(posIo.array_id);
    FlushIoSmartPtr flushIo(new FlushIo(arrayId));
    flushIo->SetVolumeId(posIo.volume_id);

    POS_EVENT_ID eventId = EID(AIO_FLUSH_START);
    POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
        eventId, "Flush Start in Aio, volume id : {}",
        posIo.volume_id);

    CallbackSmartPtr aioCompletion(new AioCompletion(flushIo, posIo,
        ioContext));
    flushIo->SetCallback(aioCompletion);

    ioContext.cnt++;

    return flushIo;
}

void
AIO::SubmitFlush(pos_io& posIo)
{
    FlushIoSmartPtr flushIo = _CreateFlushIo(posIo);

    SpdkEventScheduler::ExecuteOrScheduleEvent(flushIo->GetOriginCore(), std::make_shared<FlushCmdHandler>(flushIo));
    return;
}

void
AIO::SubmitAsyncIO(VolumeIoSmartPtr volumeIo)
{
    uint32_t core = volumeIo->GetOriginCore();
    uint32_t arr_vol_id = volumeIo->GetVolumeId() + (volumeIo->GetArrayId() << 8);
    switch (volumeIo->dir)
    {
        case UbioDir::Write:
        {
            airlog("PERF_ARR_VOL", "write", arr_vol_id, volumeIo->GetSize());
            SpdkEventScheduler::ExecuteOrScheduleEvent(core,
                std::make_shared<WriteSubmission>(volumeIo));
        }
        break;

        case UbioDir::Read:
        {
            airlog("PERF_ARR_VOL", "read", arr_vol_id, volumeIo->GetSize());
            SpdkEventScheduler::ExecuteOrScheduleEvent(core,
                std::make_shared<ReadSubmission>(volumeIo));
        }
        break;

        default:
        {
            POS_EVENT_ID eventId = EID(BLKHDLR_WRONG_IO_DIRECTION);
            POS_TRACE_ERROR(eventId, "Wrong IO direction (only read/write types are suppoered)");
            throw eventId;
            break;
        }
    }
}

void
AIO::CompleteIOs(void)
{
    uint32_t reactor_id = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    int cnt = ioContext.cnt;
    airlog("CNT_AIO_CompleteIOs", "base", reactor_id, cnt);
    IODispatcher::CompleteForThreadLocalDeviceList();
}

void
AIO::SubmitAsyncAdmin(pos_io& io, IArrayInfo* arrayInfo)
{
    if (io.ioType == GET_LOG_PAGE)
    {
        void* bio = io.context;
        struct spdk_bdev_io* bioPos = (struct spdk_bdev_io*)bio;
        void* callerContext = bioPos->internal.caller_ctx;

        struct spdk_nvmf_request* req = (struct spdk_nvmf_request*)callerContext;

        struct spdk_nvme_cmd* cmd = &req->cmd->nvme_cmd;
        uint8_t lid = cmd->cdw10 & 0xFF;
        if (lid == SPDK_NVME_LOG_HEALTH_INFORMATION)
        {
            ioContext.needPollingCount++;
        }
    }
    if (arrayInfo == nullptr)
    {
        std::string arrayName(io.arrayName);
        arrayInfo = ArrayMgr()->GetInfo(arrayName)->arrayInfo;
    }
    uint32_t originCore = EventFrameworkApiSingleton::Instance()->GetCurrentReactor();
    CallbackSmartPtr adminCompletion(new AdminCompletion(&io, ioContext, originCore));
    IDevInfo* devmgr = DeviceManagerSingleton::Instance();
    IIODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
    IArrayDevMgr* arrayDevMgr = arrayInfo->GetArrayManager();
    EventSmartPtr event(new AdminCommandHandler(&io, originCore, adminCompletion, arrayInfo, devmgr, ioDispatcher, arrayDevMgr));
    EventSchedulerSingleton::Instance()->EnqueueEvent(event);
    return;
}

AdminCompletion::AdminCompletion(pos_io* posIo, IOCtx& ioContext, uint32_t originCore)
: Callback(false, CallbackType_AdminCompletion),
  io(posIo),
  ioContext(ioContext),
  originCore(originCore)
{
}
AdminCompletion::~AdminCompletion(void)
{
}

bool
AdminCompletion::_DoSpecificJob(void)
{
    bool keepCurrentReactor = EventFrameworkApiSingleton::Instance()->IsSameReactorNow(originCore);
    assert(keepCurrentReactor == true);
    if (ioContext.needPollingCount > 0)
    {
        ioContext.needPollingCount--;
    }
    io->complete_cb(io, POS_IO_STATUS_SUCCESS);

    airlog("CompleteUserAdminIo", "user", GetEventType(), 1);

    return true;
}

} // namespace pos
