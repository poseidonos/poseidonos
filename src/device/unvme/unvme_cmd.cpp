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

#include "unvme_cmd.h"

#include "src/bio/ubio.h"
#include "src/include/pos_error_code.hpp"
#include "src/logger/logger.h"
#include "unvme_device_context.h"
#include "unvme_io_context.h"
#ifdef _ADMIN_ENABLED
#include "spdk/include/spdk/nvme_spec.h"
#include "src/admin/disk_query_manager.h"
#endif
namespace pos
{
AbortContext::AbortContext(struct spdk_nvme_ctrlr* inputCtrlr,
    struct spdk_nvme_qpair* inputQpair, uint16_t inputCid)
: ctrlr(inputCtrlr),
  qpair(inputQpair),
  cid(inputCid)
{
}

int
UnvmeCmd::RequestIO(UnvmeDeviceContext* deviceContext,
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
#ifdef _ADMIN_ENABLED
        case UbioDir::GetLogPage:
        {
            deviceContext->IncAdminCommandCount();
            ioCtx->SetAdminCommand();
            GetLogPageContext* pageContext = static_cast<GetLogPageContext*>(data);
            if (pageContext->lid == SPDK_NVME_LOG_HEALTH_INFORMATION)
            {
                struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(deviceContext->ns);
                ret = spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, pageContext->lid,
                    SPDK_NVME_GLOBAL_NS_TAG, pageContext->payload,
                    sizeof(struct spdk_nvme_health_information_page),
                    0, callbackFunc, ioCtx);
            }
            break;
        }
#endif
        case UbioDir::AdminPassTh:
        {
            deviceContext->IncAdminCommandCount();
            ioCtx->SetAdminCommand();
            ret = _RequestAdminPassThu(deviceContext, callbackFunc, ioCtx);
            break;
        }
        case UbioDir::NvmeCli:
        {
            deviceContext->IncAdminCommandCount();
            ioCtx->SetAdminCommand();
            ret = _RequestNvmeCli(deviceContext, callbackFunc, ioCtx);
            break;
        }
        default:
        {
            POS_EVENT_ID eventId =
                POS_EVENT_ID::UNVME_OPERATION_NOT_SUPPORTED;
            POS_TRACE_WARN(static_cast<int>(eventId),
                PosEventId::GetString(eventId),
                static_cast<int>(dir));
            ret = UNVME_OPERATION_NOT_SUPPORTED_ERROR;
            break;
        }
    }

    if (!ioCtx->IsFrontEnd())
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_DEBUG_REQUEST_IO;
        POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::IO_GENERAL, eventId, PosEventId::GetString(eventId),
            startLBA, sectorCount, static_cast<int>(dir),
            ioCtx->GetDeviceName(), ret);
    }
    return ret;
}

int
UnvmeCmd::_RequestWriteUncorrectable(UnvmeDeviceContext* deviceContext,
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
UnvmeCmd::_RequestDeallocate(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    uint64_t startingLBA = ioCtx->GetStartSectorOffset();
    if ((startingLBA % Ubio::UNITS_PER_BLOCK) != 0)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
            "LBA is not 4K aligned : {} ", startingLBA);
        return -EFAULT;
    }
    uint32_t NumberOfLogicalBlocks = ioCtx->GetSectorCount();
    if (NumberOfLogicalBlocks % (Ubio::UNITS_PER_BLOCK))
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
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
    POS_TRACE_DEBUG((int)POS_EVENT_ID::ARRAY_PARTITION_TRIM,
        "Requesting Trimming from {} with block number : {}", startingLBA, NumberOfLogicalBlocks);
    return returnValue;
}

int
UnvmeCmd::_RequestAdminPassThu(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(deviceContext->ns);
    struct spdk_nvme_cmd cmd;
    {
        memcpy(&cmd, ioCtx->GetBuffer(), sizeof(cmd));
    }

    // The method copies spdk_nvme_cmd(cmd) inside the API,
    // so we don't need to allocate it for async io.
    int returnValue = spdk_nvme_ctrlr_cmd_admin_raw(ctrlr,
                                                    &cmd,
                                                    nullptr,
                                                    0,
                                                    callbackFunc,
                                                    static_cast<void*>(ioCtx));

    return returnValue;
}

int
UnvmeCmd::_RequestNvmeCli(UnvmeDeviceContext* deviceContext,
    spdk_nvme_cmd_cb callbackFunc, UnvmeIOContext* ioCtx)
{
    struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(deviceContext->ns);
    struct spdk_nvme_cmd cmd;

    memcpy(&cmd, ioCtx->GetBuffer(), sizeof(cmd));

    int returnValue = -1;

    switch (cmd.opc)
    {
        case spdk_nvme_admin_opcode::SPDK_NVME_OPC_GET_LOG_PAGE:
        {
            returnValue = spdk_nvme_ctrlr_cmd_get_log_page(
                ctrlr /*spdk_nvme_ctrlr*/,
                cmd.cdw10 /*log_page*/,
                SPDK_NVME_GLOBAL_NS_TAG,
                ioCtx->GetBuffer(),
                ioCtx->GetByteCount(),
                0,
                callbackFunc,
                static_cast<void*>(ioCtx));
            break;
        }
    }

    return returnValue;
}
} // namespace pos
