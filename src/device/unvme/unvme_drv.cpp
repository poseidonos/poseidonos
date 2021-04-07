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

#include <algorithm>
#include <ctime>
#include <list>
#include <utility>
#include <vector>

#include "spdk/thread.h"
#include "src/device/spdk/nvme.hpp"
#include "src/include/ibof_error_code.hpp"
#include "src/include/memory.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "unvme_device_context.h"
#include "unvme_io_context.h"
#include "unvme_ssd.h"

#define DEVICE_NAME_PREFIX "unvme-ns-"

namespace ibofos
{
AbortContext::AbortContext(struct spdk_nvme_ctrlr* inputCtrlr,
    struct spdk_nvme_qpair* inputQpair, uint16_t inputCid)
: ctrlr(inputCtrlr),
  qpair(inputQpair),
  cid(inputCid)
{
}

static void
UpdateRetryAndRecovery(const struct spdk_nvme_cpl* completion, UnvmeIOContext* ioCtx)
{
    bool retrySkipSctGeneric = (completion->status.sct == SPDK_NVME_SCT_GENERIC &&
        completion->status.sc != SPDK_NVME_SC_ABORTED_BY_REQUEST);

    bool retrySkipCmdError = (completion->status.sct == SPDK_NVME_SCT_COMMAND_SPECIFIC);

    bool retrySkipMediaError = (completion->status.sct == SPDK_NVME_SCT_MEDIA_ERROR &&
        (completion->status.sc == SPDK_NVME_SC_COMPARE_FAILURE ||
            completion->status.sc == SPDK_NVME_SC_DEALLOCATED_OR_UNWRITTEN_BLOCK));

    bool retrySkipPathError = (completion->status.sct == SPDK_NVME_SCT_PATH && (completion->status.sc != SPDK_NVME_SC_ABORTED_BY_HOST));

    if (retrySkipSctGeneric || retrySkipCmdError || retrySkipMediaError || retrySkipPathError)
    {
        ioCtx->ClearRetryCount();
    }

    bool recoverySkipSctGeneric = (completion->status.sct == SPDK_NVME_SCT_GENERIC &&
        (completion->status.sc == SPDK_NVME_SC_LBA_OUT_OF_RANGE));
    if (recoverySkipSctGeneric)
    {
        ioCtx->IgnoreRecovery();
    }
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
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_DEBUG_COMPLETE_IO;
            IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_GENERAL,
                eventId, IbofEventId::GetString(eventId),
                ioCtx->GetStartSectorOffset(),
                ioCtx->GetSectorCount(),
                static_cast<int>(ioCtx->GetOpcode()),
                completion->status.sc, completion->status.sct, ioCtx->GetDeviceName());
        }

        ioCtx->SetAsyncIOCompleted();

        if (completion->status.sct == SPDK_NVME_SCT_VENDOR_SPECIFIC)
        {
            // if continueous submit retry for same io context accumulates its submit retry count.
            ioCtx->IncSubmitRetryCount();
        }
        else
        {
            ioCtx->ClearSubmitRetryCount();
        }

        if (unlikely(spdk_nvme_cpl_is_error(completion)))
        {
            UpdateRetryAndRecovery(completion, ioCtx);
            devCtx->AddPendingError(*ioCtx);
        }
        else
        {
            devCtx->ioCompletionCount++;
            ioCtx->CompleteIo(CallbackError::SUCCESS);
            delete ioCtx;
        }

    }
}

static bool
CompareNamespaceEntry(struct NsEntry* first, struct NsEntry* second)
{
    int result = memcmp(first->trAddr, second->trAddr, MAX_TR_ADDR_LENGTH);
    bool firstLess = (0 >= result);

    return firstLess;
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

const uint32_t
    UnvmeDrv::SUBMISSION_RETRY_LIMIT = 10000000;

UnvmeDrv::UnvmeDrv(void)
: nvmeSsd(new Nvme("spdk_daemon")),
  spdkInitDone(false)
{
    name = "UnvmeDrv";
    nvmeSsd->SetCallback(SpdkAttachEventHandler, SpdkDetachEventHandler);
}

UnvmeDrv::~UnvmeDrv(void)
{
    if (nullptr != nvmeSsd)
    {
        nvmeSsd->Stop();
    }
}

DeviceMonitor*
UnvmeDrv::GetDaemon(void)
{
    return nvmeSsd;
}

void
UnvmeDrv::DeviceDetached(std::string sn)
{
    if (nullptr != detach_event)
    {
        detach_event(sn);
    }
    else
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UNVME_SSD_DETACH_NOTIFICATION_FAILED,
            IbofEventId::GetString(IBOF_EVENT_ID::UNVME_SSD_DETACH_NOTIFICATION_FAILED),
            sn);
    }
}

void
UnvmeDrv::DeviceAttached(struct spdk_nvme_ns* ns, int nsid,
    const spdk_nvme_transport_id* trid)
{
    std::string deviceName = DEVICE_NAME_PREFIX + std::to_string(nsid);

    if (nullptr != attach_event)
    {
        uint64_t diskSize = spdk_nvme_ns_get_size(ns);

        UBlockDevice* dev = new UnvmeSsd(deviceName, diskSize, this,
            ns, trid->traddr);

        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_SSD_DEBUG_CREATED;
        IBOF_TRACE_DEBUG(eventId, IbofEventId::GetString(eventId), deviceName);
        attach_event(dev);
    }
    else
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UNVME_SSD_ATTACH_NOTIFICATION_FAILED,
            IbofEventId::GetString(IBOF_EVENT_ID::UNVME_SSD_ATTACH_NOTIFICATION_FAILED),
            deviceName);
    }
    nvmeSsd->Resume();
}

int
UnvmeDrv::ScanDevs(vector<UBlockDevice*>* devs)
{
    uint32_t addedDeviceCount = 0;

    if (spdkInitDone == false)
    {
        std::list<struct NsEntry*>* nsList = nvmeSsd->InitController();
        // spdkInitDone flag gurantees, nsList will not be accessed by other device operation.(like DeviceMonitor)
        // So, we will not guard lock for nsList
        if (nullptr != nsList)
        {
            uint32_t nsIndex = 0;
            std::vector<struct NsEntry*> nsEntryVector;
            for (auto& iter : *nsList)
            {
                nsEntryVector.push_back(iter);
            }

            std::sort(nsEntryVector.begin(), nsEntryVector.end(),
                CompareNamespaceEntry);

            for (struct NsEntry* nsEntry : nsEntryVector)
            {
                int ret = _CheckConstraints(nsEntry);
                if (ret != 0)
                {
                    continue;
                }
                std::string name = DEVICE_NAME_PREFIX;
                name = name + std::to_string(nsIndex);

                IBOF_TRACE_INFO((int)IBOF_EVENT_ID::UNVME_SSD_SCANNED,
                    IbofEventId::GetString(IBOF_EVENT_ID::UNVME_SSD_SCANNED),
                    name, nsEntry->trAddr);

                vector<UBlockDevice*>::iterator it;
                for (it = devs->begin(); it != devs->end(); it++)
                {
                    if ((*it)->GetName() == name)
                    {
                        break;
                    }
                }

                if (it == devs->end())
                {
                    uint64_t diskSize = spdk_nvme_ns_get_size(nsEntry->u.nvme.ns);
                    UBlockDevice* dev = new UnvmeSsd(name, diskSize, this,
                        nsEntry->u.nvme.ns, nsEntry->trAddr);

                    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_SSD_DEBUG_CREATED;
                    IBOF_TRACE_DEBUG(eventId, IbofEventId::GetString(eventId), name);
                    devs->push_back(dev);
                    addedDeviceCount++;
                }

                nsIndex++;
            }

            spdkInitDone = true;
        }
        else
        {
            IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UNVME_SSD_SCAN_FAILED,
                IbofEventId::GetString(IBOF_EVENT_ID::UNVME_SSD_SCAN_FAILED));
        }
    }

    return addedDeviceCount;
}

bool
UnvmeDrv::Open(DeviceContext* deviceContext)
{
    bool openSuccessful = false;
    if (nullptr != deviceContext)
    {
        UnvmeDeviceContext* devCtx =
            static_cast<UnvmeDeviceContext*>(deviceContext);
        if (nullptr != devCtx->ns)
        {
            if (nullptr == devCtx->ioQPair)
            {
                struct spdk_nvme_ctrlr* ctrlr =
                    spdk_nvme_ns_get_ctrlr(devCtx->ns);
                struct spdk_nvme_qpair* qpair =
                    spdk_nvme_ctrlr_alloc_io_qpair(ctrlr, NULL, 0);

                if (nullptr == qpair)
                {
                    IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::UNVME_SSD_OPEN_FAILED,
                        IbofEventId::GetString(IBOF_EVENT_ID::UNVME_SSD_OPEN_FAILED),
                        spdk_nvme_ns_get_id(devCtx->ns));
                }
                else
                {
                    devCtx->ioQPair = qpair;
                    openSuccessful = true;
                }
            }
        }
    }

    return openSuccessful;
}

bool
UnvmeDrv::Close(DeviceContext* deviceContext)
{
    bool closeSuccessful = true;
    if (nullptr != deviceContext)
    {
        UnvmeDeviceContext* devCtx =
            static_cast<UnvmeDeviceContext*>(deviceContext);
        if (nullptr != devCtx->ioQPair)
        {
            int retError = spdk_nvme_ctrlr_free_io_qpair(devCtx->ioQPair);
            if (0 != retError)
            {
                IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_SSD_CLOSE_FAILED;
                IBOF_TRACE_ERROR(static_cast<int>(eventId),
                    IbofEventId::GetString(eventId),
                    spdk_nvme_ns_get_id(devCtx->ns));

                closeSuccessful = false;
            }
            else
            {
                devCtx->ioQPair = nullptr;
            }
        }
    }

    return closeSuccessful;
}

int
UnvmeDrv::_SubmitAsyncIOInternal(UnvmeDeviceContext* deviceContext,
    UnvmeIOContext* ioCtx)
{
    int retValue = 0, retValueComplete = 0;
    int completions = 0;

    ioCtx->ClearAsyncIOCompleted();
    deviceContext->IncreasePendingIO();

    if (unlikely(-ENOMEM ==
                (retValue = _RequestIO(deviceContext, AsyncIOComplete, ioCtx))))
    {
        if (unlikely(SUBMISSION_RETRY_LIMIT == ioCtx->GetSubmitRetryCount()))
        {
            // submission timed out.
            IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_SUBMISSION_RETRY_EXCEED;
            uint64_t offset = 0, sectorCount = 0;

            offset = ioCtx->GetStartSectorOffset();
            sectorCount = ioCtx->GetSectorCount();

            IBOF_TRACE_WARN(static_cast<int>(eventId),
                IbofEventId::GetString(eventId),
                offset,
                sectorCount,
                ioCtx->GetDeviceName(),
                spdk_nvme_ns_get_id(deviceContext->ns));
        }

        retValueComplete = _CompleteIOs(deviceContext, ioCtx);
        if (0 < retValueComplete)
        {
            completions += retValueComplete;
        }
        else
        {
            spdk_nvme_ctrlr *ctrl = spdk_nvme_ns_get_ctrlr(deviceContext->ns);
            if (spdk_nvme_ctrlr_is_failed(ctrl))
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
            completion.status.sct = SPDK_NVME_SCT_VENDOR_SPECIFIC;
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

    EventSmartPtr callback(bio->GetCallback());
    bool frontEnd = false;
    uint32_t retryCount = nvmeSsd->GetRetryCount(RetryType::RETRY_TYPE_BACKEND);

    if (callback)
    {
        if (callback->IsFrontEnd())
        {
            retryCount = nvmeSsd->GetRetryCount(RetryType::RETRY_TYPE_FRONTEND);
            frontEnd = true;
        }
    }

    UnvmeIOContext* ioCtx = new UnvmeIOContext(devCtx, bio, retryCount, frontEnd);

    return _SubmitAsyncIOInternal(devCtx, ioCtx);
}

int
UnvmeDrv::CompleteIOs(DeviceContext* deviceContext)
{
    return _CompleteIOs(deviceContext, nullptr);
}

int
UnvmeDrv::_CompleteIOs(DeviceContext* deviceContext, UnvmeIOContext* ioCtxToSkip)
{
    UnvmeDeviceContext* devCtx =
        static_cast<UnvmeDeviceContext*>(deviceContext);
    uint32_t completionCount = 0;
    devCtx->ioCompletionCount = 0;
    int returnCode = 0;

    if (unlikely(!devCtx->IsAdminCommandPendingZero()))
    {
        spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(devCtx->ns);
        returnCode = spdk_nvme_ctrlr_process_admin_completions(ctrlr);
        if (0 < returnCode)
        {
            completionCount = devCtx->ioCompletionCount;
            return completionCount;
        }
    }

    // if admin command is failed to completion (including not-yet-processed)
    // Also try to complete io.

    returnCode = spdk_nvme_qpair_process_completions(devCtx->ioQPair, 0);

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
    std::pair<IOContext*, bool> pendingError = deviceContext->GetPendingError();

    IOContext* ioCtx = pendingError.first;
    bool disregardError = pendingError.second;

    if (nullptr != ioCtx)
    {
        uint32_t errorCount = 0;

        deviceContext->RemovePendingError(*ioCtx);
        
        if (false == disregardError)
        {
            errorCount++;
        }

        if (errorCount > 0)
        {
            if (ioCtx->GetSubmitRetryCount() != 0)
            {
                UnvmeDeviceContext* unvmeDevCtx = static_cast<UnvmeDeviceContext*>(deviceContext);
                UnvmeIOContext* unvmeIoCtx = static_cast<UnvmeIOContext*>(ioCtx);
                _SubmitAsyncIOInternal(unvmeDevCtx, unvmeIoCtx);

                return completionCount;
            }
            else if (ioCtx->CheckAndDecreaseRetryCount() == true)
            {
                UnvmeDeviceContext* unvmeDevCtx = static_cast<UnvmeDeviceContext*>(deviceContext);
                UnvmeIOContext* unvmeIoCtx = static_cast<UnvmeIOContext*>(ioCtx);
                IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_DEBUG_RETRY_IO;

                IBOF_TRACE_WARN(eventId, IbofEventId::GetString(eventId),
                    ioCtx->GetStartSectorOffset(),
                    ioCtx->GetSectorCount(),
                    static_cast<int>(ioCtx->GetOpcode()),
                    ioCtx->GetDeviceName());
                _SubmitAsyncIOInternal(unvmeDevCtx, unvmeIoCtx);

                return completionCount;
            }
            else
            {
                ioCtx->CompleteIo(CallbackError::GENERIC_ERROR);
            }
        }

        delete ioCtx;

        completionCount++;
    }

    return completionCount;
}

int
UnvmeDrv::_RequestIO(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    struct spdk_nvme_ns* ns = deviceContext->ns;
    struct spdk_nvme_qpair* ioqpair = deviceContext->ioQPair;

    UbioDir dir = ioCtx->GetOpcode();
    uint64_t startLBA = ioCtx->GetStartSectorOffset();
    uint64_t sectorCount = ioCtx->GetSectorCount();
    void* data = ioCtx->GetBuffer();

    int ret = 0;

    switch (dir)
    {
        case UbioDir::Read:
        {
            ret = spdk_nvme_ns_cmd_read(ns, ioqpair, data,
                startLBA, sectorCount,
                callbackFunc, static_cast<void*>(ioCtx), 0);
            break;
        }
        case UbioDir::Write:
        {
            ret = spdk_nvme_ns_cmd_write(ns, ioqpair, data,
                startLBA, sectorCount,
                callbackFunc, static_cast<void*>(ioCtx), 0);
            break;
        }
        case UbioDir::WriteUncor:
        {
            ret = _RequestWriteUncorrectable(deviceContext,
                callbackFunc, ioCtx);
            break;
        }
        case UbioDir::Deallocate:
        {
            ret = _RequestDeallocate(deviceContext,
                callbackFunc, ioCtx);
            break;
        }
        case UbioDir::Abort:
        {
            AbortContext* abortContext = static_cast<AbortContext*>(data);
            deviceContext->IncAdminCommandCount();
            ioCtx->SetAdminCommand();

            ret = spdk_nvme_ctrlr_cmd_abort(abortContext->ctrlr,
                abortContext->qpair,
                abortContext->cid,
                callbackFunc, ioCtx);
            break;
        }

        default:
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::UNVME_OPERATION_NOT_SUPPORTED;
            IBOF_TRACE_WARN(static_cast<int>(eventId),
                IbofEventId::GetString(eventId),
                static_cast<int>(dir));
            ret = UNVME_OPERATION_NOT_SUPPORTED_ERROR;
            break;
        }
    }

    if (!ioCtx->IsFrontEnd())
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_DEBUG_REQUEST_IO;
        IBOF_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_GENERAL, eventId, IbofEventId::GetString(eventId),
                        startLBA, sectorCount, static_cast<int>(dir),
                        ioCtx->GetDeviceName(), ret);
    }
    return ret;
}

int
UnvmeDrv::_RequestWriteUncorrectable(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    uint64_t startingLBA = ioCtx->GetStartSectorOffset();
    uint32_t NumberOfLogicalBlocks = ioCtx->GetSectorCount() - 1; // Zero-based

    struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(deviceContext->ns);
    struct spdk_nvme_qpair* ioqpair = deviceContext->ioQPair;
    uint32_t namespaceID = spdk_nvme_ns_get_id(deviceContext->ns);
    struct spdk_nvme_cmd cmd;
    {
        memset(&cmd, 0, sizeof(cmd));
        cmd.opc = SPDK_NVME_OPC_WRITE_UNCORRECTABLE;
        cmd.nsid = namespaceID;
        cmd.cdw10 = startingLBA & 0xFFFFFFFF;
        cmd.cdw11 = startingLBA >> 32;
        cmd.cdw12 = NumberOfLogicalBlocks;
    }
    // The method copies spdk_nvme_cmd(cmd) inside the API,
    // so we don't need to allocate it for async io.
    int returnValue = spdk_nvme_ctrlr_cmd_io_raw(ctrlr, ioqpair, &cmd,
        nullptr, 0, callbackFunc,
        static_cast<void*>(ioCtx));
    return returnValue;
}

int
UnvmeDrv::_RequestDeallocate(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    uint64_t startingLBA = ioCtx->GetStartSectorOffset();
    if ((startingLBA % Ubio::UNITS_PER_BLOCK) != 0)
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
            "LBA is not 4K aligned : {} ", startingLBA);
        return -EFAULT;
    }
    uint32_t NumberOfLogicalBlocks = ioCtx->GetSectorCount();
    if (NumberOfLogicalBlocks % (Ubio::UNITS_PER_BLOCK))
    {
        IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
            "Block Address is not 4K aligned : {} ", NumberOfLogicalBlocks);
        return -EFAULT;
    }
    struct spdk_nvme_ns* ns = deviceContext->ns;
    struct spdk_nvme_qpair* ioqpair = deviceContext->ioQPair;

    struct spdk_nvme_dsm_range* rangeDefinition =
        (struct spdk_nvme_dsm_range*)malloc(sizeof(struct spdk_nvme_dsm_range));
    rangeDefinition->starting_lba = startingLBA;
    rangeDefinition->length = NumberOfLogicalBlocks;
    rangeDefinition->attributes.raw = 0;

    uint16_t num_range = 1;

    int returnValue = spdk_nvme_ns_cmd_dataset_management(ns, ioqpair, SPDK_NVME_DSM_ATTR_DEALLOCATE,
        rangeDefinition, num_range, callbackFunc, static_cast<void*>(ioCtx));
    IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::ARRAY_PARTITION_TRIM,
        "Requesting Trimming from {} with block number : {}", startingLBA, NumberOfLogicalBlocks);
    return returnValue;
}

int
UnvmeDrv::_CheckConstraints(const NsEntry* nsEntry)
{
    const uint32_t ALLOWED_DEVICE_SECTOR_SIZE = 512;

    struct spdk_nvme_ns* ns = nsEntry->u.nvme.ns;
    if (spdk_nvme_ns_get_sector_size(ns) != ALLOWED_DEVICE_SECTOR_SIZE)
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::UNVME_NOT_SUPPORTED_DEVICE;
        IBOF_TRACE_WARN(eventId,
            "Device {} is not supported. Sector size is not {}",
            nsEntry->name,
            ALLOWED_DEVICE_SECTOR_SIZE);
        return static_cast<int>(eventId);
    }

    return 0;
}

} // namespace ibofos
