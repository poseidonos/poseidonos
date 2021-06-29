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

#include "unvme_ssd.h"

#include <cstdio>
#include <iostream>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "unvme_device_context.h"
#include "unvme_drv.h"

namespace pos
{
UnvmeSsd::UnvmeSsd(
    std::string name, uint64_t size, UnvmeDrv* driverToUse,
    struct spdk_nvme_ns* namespaceToUse,
    std::string addr)
: UBlockDevice(name, size, driverToUse),
  ns(namespaceToUse),
  outstandingAdminCommands(0)
{
    property.bdf = addr;
    property.type = DeviceType::SSD;
    property.mn = _GetMN();
    property.sn = _GetSN();
    property.numa = _GetNuma();
}

UnvmeSsd::~UnvmeSsd(void)
{
}

DeviceContext*
UnvmeSsd::_AllocateDeviceContext(void)
{
    DeviceContext* deviceContext = new UnvmeDeviceContext(ns);
    return deviceContext;
}

void
UnvmeSsd::_ReleaseDeviceContext(DeviceContext* deviceContextToRelease)
{
    if (nullptr != deviceContextToRelease)
    {
        UnvmeDeviceContext* deviceContext =
            static_cast<UnvmeDeviceContext*>(deviceContextToRelease);
        delete deviceContext;
    }
}

std::string
UnvmeSsd::_GetSN()
{
    spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(ns);
    const struct spdk_nvme_ctrlr_data* cdata;
    cdata = spdk_nvme_ctrlr_get_data(ctrlr);

    char str[128];
    snprintf(str, sizeof(cdata->sn) + 1, "%s", cdata->sn);
    return std::string(str);
}

std::string
UnvmeSsd::_GetMN()
{
    spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(ns);
    const struct spdk_nvme_ctrlr_data* cdata;
    cdata = spdk_nvme_ctrlr_get_data(ctrlr);

    char str[256];
    snprintf(str, sizeof(cdata->mn) + 1, "%s", cdata->mn);
    return std::string(str);
}

int
UnvmeSsd::_GetNuma()
{
    spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(ns);
    spdk_pci_device* pciDev = spdk_nvme_ctrlr_get_pci_device(ctrlr);
    int numa = spdk_pci_device_get_socket_id(pciDev);
    return numa;
}

/*	
int
UnvmeSsd::PassThroughNvmeAdminCommand(struct spdk_nvme_cmd *cmd,
        void *buffer, uint32_t bufferSizeInBytes)
{
    struct spdk_nvme_ctrlr *ctrlr = spdk_nvme_ns_get_ctrlr(ns);
    //int errorCode = spdk_nvme_ctrlr_cmd_admin_raw(ctrlr, cmd,
    //         buffer, bufferSizeInBytes, &CallbackAdminCommand, this);
	int errorCode = spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, SPDK_NVME_LOG_HEALTH_INFORMATION, SPDK_NVME_GLOBAL_NS_TAG,
					 buffer,
					 bufferSizeInBytes, 0, 0, 0);
					 //&CallbackAdminCommand, this);

    if (0 == errorCode)
    {
        outstandingAdminCommands++;

        while (0 < outstandingAdminCommands)
        {
            //errorCode = spdk_nvme_ctrlr_process_admin_completions(ctrlr);
            if (0 > errorCode)
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_COMPLETION_FAILED;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                            PosEventId::GetString(eventId),
                            spdk_nvme_ns_get_id(ns), errorCode);
            }
        }
    }
    else
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SUBMISSION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), errorCode);
    }

    return errorCode;
}
*/
int
UnvmeSsd::PassThroughNvmeAdminCommand(struct spdk_nvme_cmd* cmd,
    void* buffer, uint32_t bufferSizeInBytes)
{
    struct spdk_nvme_ctrlr* ctrlr = spdk_nvme_ns_get_ctrlr(ns);
    int errorCode = spdk_nvme_ctrlr_cmd_admin_raw(ctrlr, cmd,
        buffer, bufferSizeInBytes, &_CallbackAdminCommand, this);

    if (0 == errorCode)
    {
        outstandingAdminCommands++;

        while (0 < outstandingAdminCommands)
        {
            errorCode = spdk_nvme_ctrlr_process_admin_completions(ctrlr);
            if (0 > errorCode)
            {
                POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_COMPLETION_FAILED;
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    PosEventId::GetString(eventId),
                    spdk_nvme_ns_get_id(ns), errorCode);
            }
        }
    }
    else
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SUBMISSION_FAILED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId), errorCode);
    }

    return errorCode;
}

void
UnvmeSsd::DecreaseOutstandingAdminCount(void)
{
    uint32_t oldOutstandingCount = outstandingAdminCommands--;
    uint32_t currentCompletionCount = 1;
    if (unlikely(oldOutstandingCount < currentCompletionCount))
    {
        POS_EVENT_ID eventId = POS_EVENT_ID::UNVME_SSD_UNDERFLOW_HAPPENED;
        POS_TRACE_ERROR(static_cast<int>(eventId),
            PosEventId::GetString(eventId),
            oldOutstandingCount, currentCompletionCount);
    }
}

void
UnvmeSsd::_CallbackAdminCommand(void* arg, const struct spdk_nvme_cpl* cpl)
{
    UnvmeSsd* unvmeSsd = static_cast<UnvmeSsd*>(arg);

    if (spdk_nvme_cpl_is_error(cpl))
    {
        printf("get log page failed\n");
    }

    unvmeSsd->DecreaseOutstandingAdminCount();
}

} // namespace pos
