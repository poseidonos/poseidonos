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

#ifndef __IBOFOS_UNVME_SSD_H__
#define __IBOFOS_UNVME_SSD_H__

#include <cstdint>
#include <string>

#include "spdk/nvme.h"
#include "src/device/ublock_device.h"

namespace ibofos
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
        std::string addr);
    ~UnvmeSsd() override;

    struct spdk_nvme_ns*
    GetNs(void)
    {
        return ns;
    }

    int PassThroughNvmeAdminCommand(struct spdk_nvme_cmd* cmd,
        void* buffer, uint32_t bufferSizeInBytes);

    void DecreaseOutstandingAdminCount(void);

private:
    DeviceContext* _AllocateDeviceContext(void) override;
    void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) override;

    static void
    _CallbackAdminCommand(void* arg, const struct spdk_nvme_cpl* cpl);

    std::string _GetSN();
    std::string _GetMN();

    spdk_nvme_ns* ns;
    std::atomic<uint32_t> outstandingAdminCommands;
};
} // namespace ibofos

#endif // __IBOFOS_UNVME_SSD_H__
