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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "spdk/nvme.h"
#include "src/device/base/device_driver.h"
#include "src/device/unvme/unvme_cmd.h"
#include "src/device/unvme/unvme_mgmt.h"
#include "src/lib/singleton.h"

struct spdk_nvme_qpair;
namespace pos
{
class DeviceContext;

class DeviceMonitor;
class Ubio;

class UnvmeDeviceContext;
class UnvmeIOContext;
class SpdkNvmeCaller;

class UnvmeDrv : public DeviceDriver
{
public:
    UnvmeDrv(UnvmeCmd* unvmeCmd = nullptr,
        SpdkNvmeCaller* spdkNvmeCaller = nullptr);
    ~UnvmeDrv(void) override;
    int ScanDevs(std::vector<UblockSharedPtr>* devs) override;

    bool Open(DeviceContext* deviceContext) override;
    bool Close(DeviceContext* deviceContext) override;

    int CompleteIOs(DeviceContext* deviceContext) override;
    int CompleteErrors(DeviceContext* deviceContext) override;

    int SubmitAsyncIO(DeviceContext* deviceContext, UbioSmartPtr bio) override;

    DeviceMonitor* GetDaemon(void);
    int DeviceAttached(struct spdk_nvme_ns* ns, int num_devs,
        const spdk_nvme_transport_id* trid);
    int DeviceDetached(std::string sn);
    void SpdkDetach(struct spdk_nvme_ns* ns);

private:
    int _RequestIO(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

    int _SubmitAsyncIOInternal(UnvmeDeviceContext* deviceContext,
        UnvmeIOContext* ioCtx);

    int _RequestWriteUncorrectable(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

    int _RequestDeallocate(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioCtx);

    int _CompleteIOs(DeviceContext* deviceContext, UnvmeIOContext* ioCtxToSkip);

    Nvme* nvmeSsd;
    UnvmeMgmt unvmeMgmt;
    UnvmeCmd* unvmeCmd;
    SpdkNvmeCaller* spdkNvmeCaller;
};

const uint32_t UNVME_DRV_OUT_OF_MEMORY_RETRY_LIMIT = 10000000;

using UnvmeDrvSingleton = Singleton<UnvmeDrv>;
} // namespace pos
