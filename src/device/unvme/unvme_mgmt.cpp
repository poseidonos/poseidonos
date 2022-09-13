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


#include "unvme_mgmt.h"

#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "spdk/thread.h"
#include "src/spdk_wrapper/nvme.hpp"
#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "unvme_device_context.h"
#include "unvme_ssd.h"

namespace pos
{
static bool
CompareNamespaceEntry(struct NsEntry* first, struct NsEntry* second)
{
    int result = memcmp(first->trAddr, second->trAddr, MAX_TR_ADDR_LENGTH);
    bool firstLess = (0 > result);

    if (0 == result)
    {
        uint32_t firstNsId = spdk_nvme_ns_get_id(first->u.nvme.ns);
        uint32_t secondNsId = spdk_nvme_ns_get_id(second->u.nvme.ns);

        firstLess = (firstNsId < secondNsId);
    }

    return firstLess;
}

UnvmeMgmt::UnvmeMgmt(SpdkNvmeCaller* spdkCaller, bool spdkInitDone)
: spdkInitDone(spdkInitDone),
  spdkCaller(spdkCaller)
{
}

UnvmeMgmt::~UnvmeMgmt(void)
{
    if (spdkCaller != nullptr)
    {
        delete spdkCaller;
    }
}

int
UnvmeMgmt::ScanDevs(vector<UblockSharedPtr>* devs, Nvme* nvmeSsd, UnvmeDrv* drv)
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

                POS_TRACE_INFO(EID(UNVME_SSD_SCANNED),
                    "uNVMe Device has been scanned: {}, {}",
                    name, nsEntry->trAddr);

                vector<UblockSharedPtr>::iterator it;
                for (it = devs->begin(); it != devs->end(); it++)
                {
                    if ((*it)->GetName() == name)
                    {
                        break;
                    }
                }

                if (it == devs->end())
                {
                    uint64_t diskSize =
                        spdkCaller->SpdkNvmeNsGetSize(nsEntry->u.nvme.ns);
                    UblockSharedPtr dev = make_shared<UnvmeSsd>(name, diskSize, drv,
                        nsEntry->u.nvme.ns, nsEntry->trAddr);

                    POS_EVENT_ID eventId = EID(UNVME_SSD_DEBUG_CREATED);
                    POS_TRACE_DEBUG(eventId, "Create Ublock, Pointer : {}", name);
                    devs->push_back(dev);
                    addedDeviceCount++;
                }

                nsIndex++;
            }

            spdkInitDone = true;
        }
        else
        {
            POS_TRACE_ERROR(EID(UNVME_SSD_SCAN_FAILED),
                "Failed to Scan uNVMe devices");
        }
    }

    return addedDeviceCount;
}

bool
UnvmeMgmt::Open(DeviceContext* deviceContext)
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
                    spdkCaller->SpdkNvmeNsGetCtrlr(devCtx->ns);
                struct spdk_nvme_qpair* qpair =
                    spdkCaller->SpdkNvmeCtrlrAllocIoQpair(ctrlr, NULL, 0);

                if (nullptr == qpair)
                {
                    POS_TRACE_ERROR(EID(UNVME_SSD_OPEN_FAILED),
                        "uNVMe Device open failed: namespace #{}",
                        spdkCaller->SpdkNvmeNsGetId(devCtx->ns));
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
UnvmeMgmt::Close(DeviceContext* deviceContext)
{
    bool closeSuccessful = true;
    if (nullptr != deviceContext)
    {
        UnvmeDeviceContext* devCtx =
            static_cast<UnvmeDeviceContext*>(deviceContext);
        if (nullptr != devCtx->ioQPair)
        {
            int retError = spdkCaller->SpdkNvmeCtrlrFreeIoQpair(devCtx->ioQPair);
            if (0 != retError)
            {
                POS_EVENT_ID eventId = EID(UNVME_SSD_CLOSE_FAILED);
                POS_TRACE_ERROR(static_cast<int>(eventId),
                    "uNVMe Device close failed: namespace #{}",
                    spdkCaller->SpdkNvmeNsGetId(devCtx->ns));
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
UnvmeMgmt::_CheckConstraints(const NsEntry* nsEntry)
{
    const uint32_t ALLOWED_DEVICE_SECTOR_SIZE = 512;

    struct spdk_nvme_ns* ns = nsEntry->u.nvme.ns;
    if (spdkCaller->SpdkNvmeNsGetSectorSize(ns) != ALLOWED_DEVICE_SECTOR_SIZE)
    {
        POS_EVENT_ID eventId = EID(UNVME_NOT_SUPPORTED_DEVICE);
        POS_TRACE_WARN(eventId,
            "Device {} is not supported. Sector size is not {}",
            nsEntry->name,
            ALLOWED_DEVICE_SECTOR_SIZE);
        return static_cast<int>(eventId);
    }

    return 0;
}

} // namespace pos
