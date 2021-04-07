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

#include "unvram_io_context.h"

#include "uram_device_context.h"

namespace ibofos
{
UnvramIOContext::UnvramIOContext(UramDeviceContext* inputDeviceContext,
    UbioSmartPtr inputUbio, uint32_t inputRetry)
: IOContext(inputUbio, inputRetry),
  devCtx(inputDeviceContext),
  retryCnt(inputRetry)
{
    retryCtx.bdev = nullptr;
    retryCtx.cb_arg = nullptr;
    retryCtx.cb_fn = nullptr;
    retryCtx.link.tqe_next = nullptr;
    retryCtx.link.tqe_prev = nullptr;
}

UnvramIOContext::~UnvramIOContext(void)
{
}

UramDeviceContext*
UnvramIOContext::GetDeviceContext(void)
{
    return devCtx;
}

uint32_t
UnvramIOContext::GetRetryCount(void)
{
    return retryCnt;
}

void
UnvramIOContext::AddRetryCount(void)
{
    retryCnt++;
}

void
UnvramIOContext::_PrepareRetryContext(spdk_bdev_io_wait_cb callbackFunc)
{
    retryCtx.bdev = devCtx->bdev;
    retryCtx.cb_fn = callbackFunc;
    retryCtx.cb_arg = static_cast<void*>(this);
}

bool
UnvramIOContext::RequestRetry(spdk_bdev_io_wait_cb callbackFunc)
{
    _PrepareRetryContext(callbackFunc);

    int errorCode = spdk_bdev_queue_io_wait(devCtx->bdev,
        devCtx->bdev_io_channel, &retryCtx);
    return (0 == errorCode);
}

} // namespace ibofos
