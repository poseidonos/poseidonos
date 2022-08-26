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

#include "uram_drv.h"

#include <air/Air.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "spdk/log.h"
#include "spdk/thread.h"
#include "src/bio/ubio.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "uram.h"
#include "uram_device_context.h"
#include "uram_io_context.h"

using namespace pos;

static void
AsyncIOComplete(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    UramIOContext* ioCtx = static_cast<UramIOContext*>(cb_arg);
    SpdkBdevCaller* spdkBdevCaller = ioCtx->GetBdevCaller();
    UramDeviceContext* devCtx = ioCtx->GetDeviceContext();
    devCtx->ioCompleCnt++;
    uint64_t ramId = reinterpret_cast<uint64_t>(ioCtx->GetDeviceContext());

    if (nullptr != bdev_io)
    {
        spdkBdevCaller->SpdkBdevFreeIo(bdev_io);
    }

    IOErrorType IOErrorType = IOErrorType::SUCCESS;
    if (unlikely(false == success))
    {
        IOErrorType = IOErrorType::GENERIC_ERROR;
        POS_TRACE_ERROR(EID(URAM_COMPLETION_FAILED),
            "URAM Async IO Failed");
    }

    devCtx->DecreasePendingIO();
    airlog("CNT_PendingIO", "nvram", ramId, -1);
    ioCtx->CompleteIo(IOErrorType);

    delete ioCtx;
}

UramDrv::UramDrv(SpdkBdevCaller* spdkBdevCaller,
    SpdkThreadCaller* spdkThreadCaller,
    EventFrameworkApi* eventFrameworkApi)
: spdkBdevCaller(spdkBdevCaller),
  spdkThreadCaller(spdkThreadCaller),
  eventFrameworkApi(eventFrameworkApi)
{
    name = "UramDrv";
}

UramDrv::~UramDrv(void)
{
    if (spdkBdevCaller != nullptr)
    {
        delete spdkBdevCaller;
    }

    if (spdkThreadCaller != nullptr)
    {
        delete spdkThreadCaller;
    }
}

int
UramDrv::ScanDevs(std::vector<UblockSharedPtr>* devs)
{
    const string URAM_PRODUCT_NAME("Malloc disk");

    uint32_t addedDeviceCount = 0;

    for (struct spdk_bdev* bdev = spdkBdevCaller->SpdkBdevFirst();
         bdev != nullptr;
         bdev = spdkBdevCaller->SpdkBdevNext(bdev))
    {
        if (bdev->product_name != URAM_PRODUCT_NAME)
        {
            continue;
        }

        uint32_t blockSize = spdkBdevCaller->SpdkBdevGetBlockSize(bdev);
        uint64_t blockCount = spdkBdevCaller->SpdkBdevGetNumBlocks(bdev);
        uint64_t bdevSize = blockSize * blockCount;
        uint32_t numa = spdkBdevCaller->SpdkPosMallocBdevGetNuma(bdev);

        UblockSharedPtr dev =
            make_shared<Uram>(bdev->name, bdevSize, this, numa);
        devs->push_back(dev);
        addedDeviceCount++;
    }

    return addedDeviceCount;
}

bool
UramDrv::_OpenBdev(UramDeviceContext* devCtx)
{
    spdk_bdev_desc* bdev_desc = nullptr;

    devCtx->bdev = spdkBdevCaller->SpdkBdevGetByName(devCtx->name);
    if (NULL == devCtx->bdev)
    {
        POS_TRACE_ERROR(EID(DEVICE_OPEN_FAILED),
            "Could not find the bdev: {}", devCtx->name);
        return false;
    }
    POS_TRACE_INFO(EID(DEVICE_INFO_MSG),
        "Opening the bdev {}", devCtx->name);
    spdk_bdev_event_cb_t cb =
        [](enum spdk_bdev_event_type type,
            struct spdk_bdev* bdev,
            void* event_ctx) -> void {
        POS_TRACE_WARN(EID(DEVICE_INFO_MSG),
            "Unsupported bdev event: type {}", type);
    };

    int rc = spdkBdevCaller->SpdkBdevOpenExt(
        devCtx->name, true, cb, NULL, &bdev_desc);
    if (rc < 0)
    {
        POS_TRACE_ERROR(EID(DEVICE_OPEN_FAILED),
            "Could not open bdev: {}", devCtx->name);
        return false;
    }

    devCtx->bdev_desc = bdev_desc;
    devCtx->bdev_io_channel = spdkBdevCaller->SpdkBdevGetIoChannel(devCtx->bdev_desc);
    if (devCtx->bdev_io_channel == NULL)
    {
        POS_TRACE_WARN(EID(DEVICE_INFO_MSG),
            "Bdev opened, but not enough io channel");
        spdkBdevCaller->SpdkBdevClose(devCtx->bdev_desc);
        devCtx->bdev_desc = nullptr;
    }
    devCtx->opened = true;

    return true;
}

void
UramDrv::_CloseBdev(UramDeviceContext* devCtx)
{
    if (nullptr != devCtx->bdev_io_channel)
    {
        spdkThreadCaller->SpdkPutIoChannel(devCtx->bdev_io_channel);
        devCtx->bdev_io_channel = nullptr;
    }

    if (nullptr != devCtx->bdev_desc)
    {
        spdkBdevCaller->SpdkBdevClose(devCtx->bdev_desc);
        devCtx->bdev_desc = nullptr;
    }

    devCtx->ioCompleCnt = 0;
    devCtx->opened = false;
}

bool
UramDrv::Open(DeviceContext* deviceContext)
{
    bool openSuccessful = false;
    if (nullptr != deviceContext)
    {
        UramDeviceContext* devCtx =
            static_cast<UramDeviceContext*>(deviceContext);

        if (devCtx->opened == false)
        {
            if (eventFrameworkApi->IsReactorNow())
            {
                openSuccessful = _OpenBdev(devCtx);
            }
            else
            {
                openSuccessful = true;
            }
        }
    }

    return openSuccessful;
}

bool
UramDrv::Close(DeviceContext* deviceContext)
{
    if (nullptr != deviceContext)
    {
        UramDeviceContext* devCtx =
            static_cast<UramDeviceContext*>(deviceContext);
        if (devCtx->opened == true)
        {
            _CloseBdev(devCtx);
            return true;
        }
    }

    return false;
}

int
UramDrv::SubmitAsyncIO(DeviceContext* deviceContext, UbioSmartPtr bio)
{
    int completions = 0;
    UramDeviceContext* devCtx =
        static_cast<UramDeviceContext*>(deviceContext);
    UramIOContext* ioCtx = new UramIOContext(devCtx, bio);
    uint64_t ramId = reinterpret_cast<uint64_t>(ioCtx->GetDeviceContext());

    devCtx->IncreasePendingIO();
    airlog("CNT_PendingIO", "nvram", ramId, 1);

    completions = SubmitIO(ioCtx);
    return completions;
}

int
UramDrv::SubmitIO(UramIOContext* ioCtx)
{
    const int RETRYLIMIT = 1;
    int retValue = 0;
    if (ioCtx == nullptr)
    {
        POS_EVENT_ID eventId = EID(DEVICE_WARN_MSG);
        POS_REPORT_WARN(eventId, "Failed to SumbitIO. IOContext is null");
        return 0;
    }
    UramDeviceContext* devCtx = ioCtx->GetDeviceContext();
    spdk_bdev_io_completion_cb callbackFunc;

    callbackFunc = &AsyncIOComplete;

    retValue = _RequestIO(devCtx, callbackFunc, ioCtx);
    spdk_bdev_io_wait_cb retryFunc = [](void* arg) -> void {
        UramIOContext* ioCtx = static_cast<UramIOContext*>(arg);
        UramDrvSingleton::Instance()->SubmitIO(ioCtx);
    };

    if (unlikely(-ENOMEM == retValue))
    {
        if (ioCtx->GetRetryCount() < RETRYLIMIT)
        {
            ioCtx->AddRetryCount();
            if (unlikely(false == ioCtx->RequestRetry(retryFunc)))
            {
                callbackFunc(nullptr, false, ioCtx);
                POS_EVENT_ID eventId = EID(URAM_FAIL_TO_RETRY_IO);
                POS_TRACE_WARN(eventId, "");
            }
            return 0;
        }
        else
        {
            callbackFunc(nullptr, false, ioCtx);
            POS_EVENT_ID eventId = EID(URAM_SUBMISSION_TIMEOUT);
            POS_TRACE_WARN(eventId, "");
            return 0;
        }
    }
    else if (unlikely(retValue))
    {
        callbackFunc(nullptr, false, ioCtx);
        POS_EVENT_ID eventId = EID(URAM_SUBMISSION_FAILED);
        POS_TRACE_WARN(eventId, "");
        return 0;
    }

    return retValue;
}

int
UramDrv::CompleteIOs(DeviceContext* deviceContext)
{
    UramDeviceContext* devCtx =
        static_cast<UramDeviceContext*>(deviceContext);
    int completions = static_cast<int32_t>(devCtx->ioCompleCnt);
    devCtx->ioCompleCnt = 0;
    return completions;
}

int
UramDrv::_RequestIO(UramDeviceContext* devCtx,
    spdk_bdev_io_completion_cb callbackFunc,
    UramIOContext* ioCtx)
{
    struct spdk_bdev_desc* desc = devCtx->bdev_desc;
    struct spdk_io_channel* ioChannel = devCtx->bdev_io_channel;

    if (unlikely(!ioChannel))
    {
        ioChannel = spdkBdevCaller->SpdkBdevGetIoChannel(devCtx->bdev_desc);
        devCtx->bdev_io_channel = ioChannel;
    }
    UbioDir dir = ioCtx->GetOpcode();
    uint64_t offset = ioCtx->GetStartByteOffset();
    uint64_t sizeInBytes = ioCtx->GetByteCount();
    void* data = ioCtx->GetBuffer();
    int ret = EINVAL;

    switch (dir)
    {
        case UbioDir::Read:
            ret = spdkBdevCaller->SpdkBdevRead(
                desc, ioChannel, data, offset, sizeInBytes,
                callbackFunc, static_cast<void*>(ioCtx));
            break;
        case UbioDir::Write:
            ret = spdkBdevCaller->SpdkBdevWrite(
                desc, ioChannel, data, offset, sizeInBytes,
                callbackFunc, static_cast<void*>(ioCtx));
            break;
        default:
            POS_TRACE_INFO(EID(DEVICE_INFO_MSG),
                "Neither Read Nor Write {}", static_cast<int>(dir));
            break;
    }
    return ret;
}
