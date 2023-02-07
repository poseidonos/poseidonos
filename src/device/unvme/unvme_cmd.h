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

#include "spdk/nvme.h"
#include "src/spdk_wrapper/caller/spdk_nvme_caller.h"

struct spdk_nvme_qpair;
namespace pos
{
class UnvmeDeviceContext;
class UnvmeIOContext;
class AbortContext;
class SpdkNvmeCaller;

class UnvmeCmd
{
public:
    UnvmeCmd(SpdkNvmeCaller* spdkNvmeCaller = new SpdkNvmeCaller());
    virtual ~UnvmeCmd(void);
    virtual int RequestIO(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

private:
    int _RequestWriteUncorrectable(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

    int _RequestDeallocate(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioCtx);

    int _RequestDirective(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioCtx);

    int _RequestAdminPassThu(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

    int _RequestNvmeCli(UnvmeDeviceContext* deviceContext,
        spdk_nvme_cmd_cb callbackFunc,
        UnvmeIOContext* ioContext);

    SpdkNvmeCaller* spdkNvmeCaller;
};
} // namespace pos
