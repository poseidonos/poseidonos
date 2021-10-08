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

#include "spdk/bdev.h"
#include "src/device/base/io_context.h"
#include "src/spdk_wrapper/caller/spdk_bdev_caller.h"

namespace pos
{
class Ubio;
class UramDeviceContext;

class UramIOContext : public IOContext
{
public:
    UramIOContext(void) {} // For MockClass
    explicit UramIOContext(UramDeviceContext* inputDeviceContext,
        UbioSmartPtr inputUbio,
        uint32_t inputRetry = 0,
        SpdkBdevCaller* spdkBdevCaller = new SpdkBdevCaller());

    virtual ~UramIOContext(void);

    virtual UramDeviceContext* GetDeviceContext(void);
    virtual uint32_t GetRetryCount(void);
    virtual void AddRetryCount(void);
    virtual bool RequestRetry(spdk_bdev_io_wait_cb callbackFunc);
    virtual SpdkBdevCaller* GetBdevCaller(void);

private:
    void _PrepareRetryContext(spdk_bdev_io_wait_cb callbackFunc);

    UramDeviceContext* devCtx;
    uint32_t retryCnt;
    spdk_bdev_io_wait_entry retryCtx;
    SpdkBdevCaller* spdkBdevCaller;
};
} // namespace pos
