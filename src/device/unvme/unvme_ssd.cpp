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

#include "unvme_ssd.h"

#include <cstdio>
#include <iostream>

#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "unvme_device_context.h"
#include "unvme_drv.h"

using namespace pos;

UnvmeSsd::UnvmeSsd(
    std::string name,
    uint64_t size,
    UnvmeDrv* driverToUse,
    struct spdk_nvme_ns* namespaceToUse,
    std::string addr,
    SpdkNvmeCaller* spdkNvmeCaller,
    SpdkEnvCaller* spdkEnvCaller)
: UBlockDevice(name, size, driverToUse),
  ns(namespaceToUse),
  spdkNvmeCaller(spdkNvmeCaller),
  spdkEnvCaller(spdkEnvCaller)
{
    property->bdf = addr;
    property->type = DeviceType::SSD;
    property->mn = _GetMN();
    property->sn = _GetSN();
    property->numa = _GetNuma();
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
UnvmeSsd::~UnvmeSsd(void)
{
    if (spdkNvmeCaller != nullptr)
    {
        delete spdkNvmeCaller;
    }
    if (spdkEnvCaller != nullptr)
    {
        delete spdkEnvCaller;
    }
    if (destuctionCallback != nullptr)
    {
        destuctionCallback();
    }
}
// LCOV_EXCL_STOP

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
    spdk_nvme_ctrlr* ctrlr = spdkNvmeCaller->SpdkNvmeNsGetCtrlr(ns);
    const struct spdk_nvme_ctrlr_data* cdata =
        spdkNvmeCaller->SpdkNvmeCtrlrGetData(ctrlr);

    char str[128];
    snprintf(str, sizeof(cdata->sn) + 1, "%s", cdata->sn);
    return std::string(str);
}

std::string
UnvmeSsd::_GetMN()
{
    spdk_nvme_ctrlr* ctrlr = spdkNvmeCaller->SpdkNvmeNsGetCtrlr(ns);
    const struct spdk_nvme_ctrlr_data* cdata =
        spdkNvmeCaller->SpdkNvmeCtrlrGetData(ctrlr);

    char str[256];
    snprintf(str, sizeof(cdata->mn) + 1, "%s", cdata->mn);
    return std::string(str);
}

int
UnvmeSsd::_GetNuma()
{
    spdk_nvme_ctrlr* ctrlr = spdkNvmeCaller->SpdkNvmeNsGetCtrlr(ns);
    spdk_pci_device* pciDev = spdkNvmeCaller->SpdkNvmeCtrlrGetPciDevice(ctrlr);
    int numa = spdkEnvCaller->SpdkPciDeviceGetSocketId(pciDev);
    return numa;
}

struct spdk_nvme_ns*
UnvmeSsd::GetNs(void)
{
    return ns;
}
