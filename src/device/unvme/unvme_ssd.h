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

#ifndef __IBOFOS_UNVME_SSD_H__
#define __IBOFOS_UNVME_SSD_H__

#include <cstdint>
#include <string>

#include "spdk/nvme.h"
#include "src/device/base/ublock_device.h"
#include "src/spdk_wrapper/caller/spdk_env_caller.h"
#include "src/spdk_wrapper/caller/spdk_nvme_caller.h"

namespace pos
{
class DeviceContext;
class UnvmeDrv;

class UnvmeSsd : public UBlockDevice
{
public:
    explicit UnvmeSsd(std::string name,
        uint64_t size,
        UnvmeDrv* driverToUse,
        struct spdk_nvme_ns* namespaceToUse,
        std::string addr,
        SpdkNvmeCaller* spdkNvmeCaller = new SpdkNvmeCaller(),
        SpdkEnvCaller* spdkEnvCaller = new SpdkEnvCaller());
    virtual ~UnvmeSsd() override;
    struct spdk_nvme_ns* GetNs(void);
    void DecreaseOutstandingAdminCount(void);
    void SetHotDetached(void)
    {
        isHotDetached = true;
    }
    bool IsSupportedExtSmart(void)
    {
        return isSupportedExtendedSmart;
    }

private:
    DeviceContext* _AllocateDeviceContext(void) override;
    void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) override;

    std::string _GetSN();
    std::string _GetFR();
    std::string _GetMN();
    int _GetNuma();
    void _SetSupportedExtSmart(void)
    {
        isSupportedExtendedSmart = true;
    }
    void _ClassifyDevice(DeviceProperty* property);

    UnvmeDrv* driver;
    spdk_nvme_ns* ns;
    SpdkNvmeCaller* spdkNvmeCaller;
    SpdkEnvCaller* spdkEnvCaller;
    bool isHotDetached = false;
    bool isSupportedExtendedSmart;
};
} // namespace pos

#endif // __IBOFOS_UNVME_SSD_H__
