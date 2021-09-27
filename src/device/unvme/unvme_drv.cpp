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

#include "unvme_drv.h"

#include <utility>

#include "Air.h"
#include "spdk/thread.h"
#include "src/bio/ubio.h"
#include "src/device/unvme/unvme_device_context.h"
#include "src/device/unvme/unvme_io_context.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/event_scheduler/callback.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/caller/spdk_nvme_caller.h"
#include "src/spdk_wrapper/nvme.hpp"

namespace pos
{
static void
UpdateRetryAndRecovery(const struct spdk_nvme_cpl* completion, UnvmeIOContext* ioCtx)
{
    bool retrySkipSctGeneric =
        (completion->status.sct == SPDK_NVME_SCT_GENERIC &&
            completion->status.sc != SPDK_NVME_SC_ABORTED_BY_REQUEST);

    bool retrySkipCmdError =
        (completion->status.sct == SPDK_NVME_SCT_COMMAND_SPECIFIC);

    bool retrySkipMediaError =
        (completion->status.sct == SPDK_NVME_SCT_MEDIA_ERROR &&
            (completion->status.sc == SPDK_NVME_SC_COMPARE_FAILURE ||
                completion->status.sc == SPDK_NVME_SC_DEALLOCATED_OR_UNWRITTEN_BLOCK));

    bool retrySkipPathError =
        (completion->status.sct == SPDK_NVME_SCT_PATH &&
            (completion->status.sc != SPDK_NVME_SC_ABORTED_BY_HOST));

    if (retrySkipSctGeneric || retrySkipCmdError ||
        retrySkipMediaError || retrySkipPathError)
    {
        ioCtx->ClearErrorRetryCount();
    }
}

static void
_CollectAirReadLog(uint64_t size, uint64_t ssdId)
{
    airlog("PERF_SSD", "AIR_READ", ssdId, size);
}

static void
_CollectAirWriteLog(uint64_t size, uint64_t ssdId)
{
    airlog("PERF_SSD", "AIR_WRITE", ssdId, size);
}

static void
AsyncIOComplete(void* ctx, const struct spdk_nvme_cpl* completion)
{
    UnvmeIOContext* ioCtx = static_cast<UnvmeIOContext*>(ctx);
    UnvmeDeviceContext* devCtx = ioCtx->GetDeviceContext();

    if (likely(!ioCtx->IsAsyncIOCompleted()))
    {
        devCtx->DecreasePendingIO();
        if (unlikely(ioCtx->IsAdminCommand()))
        {
            devCtx->DecAdminCommandCount();
        }

        if (!ioCtx->IsFrontEnd())
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_DEBUG_COMPLETE_IO;
            POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_GENERAL,
                eventId, PosEventId::GetString(eventId),
                ioCtx->GetStartSectorOffset(),
                ioCtx->GetSectorCount(),
                static_cast<int>(ioCtx->GetOpcode()),
                completion->status.sc, completion->status.sct, ioCtx->GetDeviceName());
        }

        ioCtx->SetAsyncIOCompleted();

        if (ioCtx->HasOutOfMemoryError())
        {
            // if continueous submit retry for same io context accumulates its submit retry count.
            ioCtx->IncOutOfMemoryRetryCount();
            ioCtx->SetOutOfMemoryError(false);
        }
        else
        {
            ioCtx->ClearOutOfMemoryRetryCount();
        }

        if (unlikely(spdk_nvme_cpl_is_error(completion)))
        {
            UpdateRetryAndRecovery(completion, ioCtx);
            devCtx->AddPendingError(*ioCtx);
        }
        else
        {
            auto dir = ioCtx->GetOpcode();
            uint64_t size = ioCtx->GetByteCount();
            uint64_t ssdId =
                reinterpret_cast<uint64_t>(ioCtx->GetDeviceContext());
            if (UbioDir::Read == dir)
            {
                _CollectAirReadLog(size, ssdId);
            }
            else if (UbioDir::Write == dir)
            {
                _CollectAirWriteLog(size, ssdId);
            }

            devCtx->ioCompletionCount++;
            ioCtx->CompleteIo(IOErrorType::SUCCESS);
            delete ioCtx;
        }
    }
}

static void
SpdkDetachEventHandler(std::string sn)
{
    UnvmeDrvSingleton::Instance()->DeviceDetached(sn);
}

static void
SpdkAttachEventHandler(struct spdk_nvme_ns* ns, int num_devs,
    const spdk_nvme_transport_id* trid)
{
    UnvmeDrvSingleton::Instance()->DeviceAttached(ns, num_devs, trid);
}

UnvmeDrv::UnvmeDrv(UnvmeCmd* unvmeCmd, SpdkNvmeCaller* spdkNvmeCaller)
: nvmeSsd(new Nvme("spdk_daemon")),
  unvmeCmd(unvmeCmd),
  spdkNvmeCaller(spdkNvmeCaller)
{
    name = "UnvmeDrv";
    nvmeSsd->SetCallback(SpdkAttachEventHandler, SpdkDetachEventHandler);
    if (this->unvmeCmd == nullptr)
    {
        this->unvmeCmd = new UnvmeCmd();
    }
    if (this->spdkNvmeCaller == nullptr)
    {
        this->spdkNvmeCaller = new SpdkNvmeCaller();
    }
}

UnvmeDrv::~UnvmeDrv(void)
{
    if (nullptr != nvmeSsd)
    {
        nvmeSsd->Stop();
    }
    if (nullptr != unvmeCmd)
    {
        delete unvmeCmd;
    }
    if (nullptr != spdkNvmeCaller)
    {
        delete spdkNvmeCaller;
    }
}

DeviceMonitor*
UnvmeDrv::GetDaemon(void)
{
    return nvmeSsd;
}

int
UnvmeDrv::DeviceDetached(std::string sn)
{
    if (nullptr == detach_event)
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SSD_DETACH_NOTIFICATION_FAILED;
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId), sn);
        return (int)eventId;
    }
    detach_event(sn);
    return 0;
}

int
UnvmeDrv::DeviceAttached(struct spdk_nvme_ns* ns, int nsid,
    const spdk_nvme_transport_id* trid)
{
    int ret = 0;
    std::string deviceName = DEVICE_NAME_PREFIX + std::to_string(nsid);

    if (nullptr != attach_event)
    {
        uint64_t diskSize = spdkNvmeCaller->SpdkNvmeNsGetSize(ns);
        UblockSharedPtr dev = make_shared<UnvmeSsd>(deviceName, diskSize, this,
            ns, trid->traddr);
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SSD_DEBUG_CREATED;
        POS_TRACE_DEBUG(eventId, PosEventId::GetString(eventId), deviceName);
        attach_event(dev);
    }
    else
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SSD_ATTACH_NOTIFICATION_FAILED;
        POS_TRACE_ERROR(eventId, PosEventId::GetString(eventId), deviceName);
        ret = (int)eventId;
    }

    nvmeSsd->Resume();
    return ret;
}

int
UnvmeDrv::ScanDevs(vector<UblockSharedPtr>* devs)
{
    return unvmeMgmt.ScanDevs(devs, nvmeSsd, this);
}

bool
UnvmeDrv::Open(DeviceContext* deviceContext)
{
    return unvmeMgmt.Open(deviceContext);
}

bool
UnvmeDrv::Close(DeviceContext* deviceContext)
{
    return unvmeMgmt.Close(deviceContext);
}

int
UnvmeDrv::_SubmitAsyncIOInternal(UnvmeDeviceContext* deviceContext,
    UnvmeIOContext* ioCtx)
{
    int retValue = 0, retValueComplete = 0;
    int completions = 0;

    ioCtx->ClearAsyncIOCompleted();
    deviceContext->IncreasePendingIO();

    retValue = unvmeCmd->RequestIO(deviceContext, AsyncIOComplete, ioCtx);
    if (unlikely(-ENOMEM == retValue)) // Usually ENOMEM means the submissuion queue is full
    {
        if (unlikely(UNVME_DRV_OUT_OF_MEMORY_RETRY_LIMIT ==
                ioCtx->GetOutOfMemoryRetryCount()))
        {
            // submission timed out.
            POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SUBMISSION_RETRY_EXCEED;
            uint64_t offset = 0, sectorCount = 0;

            offset = ioCtx->GetStartSectorOffset();
            sectorCount = ioCtx->GetSectorCount();

            POS_TRACE_WARN(static_cast<int>(eventId),
                PosEventId::GetString(eventId),
                offset,
                sectorCount,
                ioCtx->GetDeviceName(),
                spdkNvmeCaller->SpdkNvmeNsGetId(deviceContext->ns));
        }

        retValueComplete = _CompleteIOs(deviceContext, ioCtx);
        if (0 < retValueComplete)
        {
            completions += retValueComplete;
        }
        else
        {
            spdk_nvme_ctrlr* ctrl =
                spdkNvmeCaller->SpdkNvmeNsGetCtrlr(deviceContext->ns);
            if (spdkNvmeCaller->SpdkNvmeCtrlrIsFailed(ctrl))
            {
                retValue = -ENXIO;
            }
        }
    }

    if (unlikely(0 > retValue))
    {
        struct spdk_nvme_cpl completion;
        completion.status.sct = SPDK_NVME_SCT_GENERIC;
        completion.status.sc = SPDK_NVME_SC_INTERNAL_DEVICE_ERROR;
        if (likely(retValue == -ENOMEM))
        {
            ioCtx->SetOutOfMemoryError(true);
        }
        AsyncIOComplete(ioCtx, &completion);
    }

    return completions;
}

int
UnvmeDrv::SubmitAsyncIO(DeviceContext* deviceContext, UbioSmartPtr bio)
{
    UnvmeDeviceContext* devCtx =
        static_cast<UnvmeDeviceContext*>(deviceContext);

    uint32_t retryCount = 0;
    CallbackSmartPtr callback = bio->GetCallback();
    bool frontEnd = false;
    if (callback != nullptr)
    {
        frontEnd = callback->IsFrontEnd();
    }

    if (frontEnd)
    {
        retryCount = nvmeSsd->GetRetryCount(RetryType::RETRY_TYPE_FRONTEND);
    }
    else
    {
        retryCount = nvmeSsd->GetRetryCount(RetryType::RETRY_TYPE_BACKEND);
    }

    UnvmeIOContext* ioCtx =
        new UnvmeIOContext(devCtx, bio, retryCount, frontEnd);

    return _SubmitAsyncIOInternal(devCtx, ioCtx);
}

int
UnvmeDrv::CompleteIOs(DeviceContext* deviceContext)
{
    return _CompleteIOs(deviceContext, nullptr);
}

int
UnvmeDrv::_CompleteIOs(DeviceContext* deviceContext,
    UnvmeIOContext* ioCtxToSkip)
{
    UnvmeDeviceContext* devCtx =
        static_cast<UnvmeDeviceContext*>(deviceContext);
    uint32_t completionCount = 0;
    devCtx->ioCompletionCount = 0;
    int returnCode = 0;

    if (unlikely(!devCtx->IsAdminCommandPendingZero()))
    {
        spdk_nvme_ctrlr* ctrlr =
            spdkNvmeCaller->SpdkNvmeNsGetCtrlr(devCtx->ns);
        returnCode =
            spdkNvmeCaller->SpdkNvmeCtrlrProcessAdminCompletions(ctrlr);
        if (0 < returnCode)
        {
            completionCount = devCtx->ioCompletionCount;
            return completionCount;
        }
    }

    // if admin command is failed to completion (including not-yet-processed)
    // Also try to complete io.

    returnCode =
        spdkNvmeCaller->SpdkNvmeQpairProcessCompletions(devCtx->ioQPair, 0);

    if (likely(0 <= returnCode))
    {
        completionCount = devCtx->ioCompletionCount;
    }

    return completionCount;
}

int
UnvmeDrv::CompleteErrors(DeviceContext* deviceContext)
{
    int completionCount = 0;

    IOContext* ioCtx = deviceContext->GetPendingError();

    if (nullptr != ioCtx)
    {
        deviceContext->RemovePendingError(*ioCtx);
        if (ioCtx->GetOutOfMemoryRetryCount() != 0)
        {
            UnvmeDeviceContext* unvmeDevCtx =
                static_cast<UnvmeDeviceContext*>(deviceContext);
            UnvmeIOContext* unvmeIoCtx = static_cast<UnvmeIOContext*>(ioCtx);
            completionCount = _SubmitAsyncIOInternal(unvmeDevCtx, unvmeIoCtx);

            return completionCount;
        }
        else if (ioCtx->CheckAndDecreaseErrorRetryCount() == true)
        {
            POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_DEBUG_RETRY_IO;
            POS_TRACE_INFO(eventId, PosEventId::GetString(eventId),
                ioCtx->GetStartSectorOffset(),
                ioCtx->GetSectorCount(),
                static_cast<int>(ioCtx->GetOpcode()),
                ioCtx->GetDeviceName());

            UnvmeDeviceContext* unvmeDevCtx =
                static_cast<UnvmeDeviceContext*>(deviceContext);
            UnvmeIOContext* unvmeIoCtx = static_cast<UnvmeIOContext*>(ioCtx);
            completionCount = _SubmitAsyncIOInternal(unvmeDevCtx, unvmeIoCtx);

            return completionCount;
        }
        else
        {
            ioCtx->CompleteIo(IOErrorType::GENERIC_ERROR);
        }

        delete ioCtx;
        completionCount++;
    }

    return completionCount;
}

} // namespace pos
