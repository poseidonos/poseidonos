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

#include "uram_drv.h"

#include <iostream>
#include <string>
#include <vector>

#include "Air.h"
#include "spdk/log.h"
#include "spdk/thread.h"
#include "src/bio/ubio.h"
#include "src/include/branch_prediction.h"
#include "src/include/memory.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "uram.h"
#include "uram_device_context.h"
#include "uram_io_context.h"

#define BDEV_NAME "uram"

namespace pos
{
static void
BdevOpenComplete(enum spdk_bdev_event_type type, struct spdk_bdev* bdev,
    void* event_ctx)
{
    SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
}

static void
AsyncIOComplete(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    UramIOContext* ioCtx = static_cast<UramIOContext*>(cb_arg);
    UramDeviceContext* devCtx = ioCtx->GetDeviceContext();
    devCtx->ioCompleCnt++;

    if (nullptr != bdev_io)
    {
        spdk_bdev_free_io(bdev_io);
    }

    IOErrorType IOErrorType = IOErrorType::SUCCESS;
    if (unlikely(false == success))
    {
        IOErrorType = IOErrorType::GENERIC_ERROR;
        SPDK_ERRLOG("Async I/O failed\n");
    }

    devCtx->DecreasePendingIO();
    ioCtx->CompleteIo(IOErrorType);

    delete ioCtx;
}

static void
RetryIO(void* arg)
{
    try
    {
        UramIOContext* ioCtx = static_cast<UramIOContext*>(arg);
        UramDrvSingleton::Instance()->SubmitIO(ioCtx);
    }
    catch (POS_EVENT_ID eventId)
    {
        std::string message(" at RetryIO");
        PosEventId::Print(eventId, EventLevel::WARNING, message);
    }
}

UramDrv::UramDrv(SpdkBdevCaller* bdevCaller)
: bdevCaller(bdevCaller)
{
    name = "UramDrv";
}

UramDrv::~UramDrv(void)
{
    if (bdevCaller != nullptr)
    {
        delete bdevCaller;
    }
}

int
UramDrv::ScanDevs(std::vector<UblockSharedPtr>* devs)
{
    uint32_t addedDeviceCount = 0;
    struct spdk_bdev* bdev = bdevCaller->SpdkBdevFirst();
    string ssd_prefix = "unvme-ns-";

    while (nullptr != bdev)
    {
        string bdev_name(bdev->name);
        if (bdev_name.compare(0, ssd_prefix.length(), ssd_prefix) == 0)
        {
            POS_TRACE_WARN((uint32_t)POS_EVENT_ID::ARRAY_DEVICE_WRONG_NAME, "URAM's name does not conform to the naming rule\n");
            bdev = bdevCaller->SpdkBdevNext(bdev);
            continue;
        }

        string bdev_product_name(bdev->product_name);
        if (bdev_product_name.compare("Malloc disk") != 0)
        {
            bdev = bdevCaller->SpdkBdevNext(bdev);
            continue;
        }

        bool isExist = false; // check if in devs
        for (UblockSharedPtr dev : *devs)
        {
            if (dev->GetName() == bdev->name)
            {
                isExist = true;
                break;
            }
        }

        if (false == isExist)
        {
            uint32_t blockSize = bdevCaller->SpdkBdevGetBlockSize(bdev);
            uint64_t blockCount = bdevCaller->SpdkBdevGetNumBlocks(bdev);
            uint64_t bdevSize = blockSize * blockCount;
            uint32_t numa = bdevCaller->SpdkPosMallocBdevGetNuma(bdev);

            UblockSharedPtr dev =
                make_shared<Uram>(bdev->name, bdevSize, this, numa);
            std::cout << "name: " << dev->GetName() << std::endl;
            devs->push_back(dev);
            addedDeviceCount++;
        }

        bdev = bdevCaller->SpdkBdevNext(bdev);
    }

    return addedDeviceCount;
}

bool
UramDrv::_OpenBdev(UramDeviceContext* devCtx)
{
    if (false == EventFrameworkApiSingleton::Instance()->IsReactorNow())
    {
        return true;
    }
    spdk_bdev_desc* bdev_desc = nullptr;

    devCtx->bdev = bdevCaller->SpdkBdevGetByName(devCtx->name);
    if (NULL == devCtx->bdev)
    {
        SPDK_ERRLOG("Could not find the bdev: %s\n", devCtx->name);
        return false;
    }
    SPDK_NOTICELOG("Opening the bdev %s\n", devCtx->name);
    int rc = bdevCaller->SpdkBdevOpenExt(
        devCtx->name, true, BdevOpenComplete, NULL, &bdev_desc);
    if (rc)
    {
        SPDK_ERRLOG("Could not open bdev: %s\n", devCtx->name);
        return false;
    }

    devCtx->bdev_desc = bdev_desc;
    devCtx->bdev_io_channel = bdevCaller->SpdkBdevGetIoChannel(devCtx->bdev_desc);
    if (devCtx->bdev_io_channel == NULL)
    {
        SPDK_NOTICELOG("Bdev opened, but not enough io channel\n");
        bdevCaller->SpdkBdevClose(devCtx->bdev_desc);
        devCtx->bdev_desc = nullptr;
    }
    devCtx->opened = true;

    return true;
}

void
UramDrv::_CloseBdev(UramDeviceContext* devCtx)
{
    if (nullptr != devCtx)
    {
        if (nullptr != devCtx->bdev_io_channel)
        {
            spdk_put_io_channel(devCtx->bdev_io_channel);
            devCtx->bdev_io_channel = nullptr;
        }

        if (nullptr != devCtx->bdev_desc)
        {
            bdevCaller->SpdkBdevClose(devCtx->bdev_desc);
            devCtx->bdev_desc = nullptr;
        }

        devCtx->ioCompleCnt = 0;
        devCtx->opened = false;
    }
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
            if (EventFrameworkApiSingleton::Instance()->IsReactorNow())
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

    devCtx->IncreasePendingIO();

    completions = SubmitIO(ioCtx);
    return completions;
}

int
UramDrv::SubmitIO(UramIOContext* ioCtx)
{
    int retValue = 0;
    UramDeviceContext* devCtx = ioCtx->GetDeviceContext();
    spdk_bdev_io_completion_cb callbackFunc;

    callbackFunc = &AsyncIOComplete;

    retValue = _RequestIO(devCtx, callbackFunc, ioCtx);

    if (unlikely(-ENOMEM == retValue))
    {
        if (ioCtx->GetRetryCount() < RETRYLIMIT)
        {
            ioCtx->AddRetryCount();
            if (unlikely(false == ioCtx->RequestRetry(&RetryIO)))
            {
                callbackFunc(nullptr, false, ioCtx);
                throw POS_EVENT_ID::URAM_FAIL_TO_RETRY_IO;
            }
        }
        else
        {
            callbackFunc(nullptr, false, ioCtx);
            throw POS_EVENT_ID::URAM_SUBMISSION_TIMEOUT;
        }
    }
    else if (unlikely(retValue))
    {
        callbackFunc(nullptr, false, ioCtx);
        throw POS_EVENT_ID::URAM_SUBMISSION_FAILED;
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
        ioChannel = spdk_bdev_get_io_channel(devCtx->bdev_desc);
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
            ret = spdk_bdev_read(desc, ioChannel, data, offset, sizeInBytes,
                callbackFunc, static_cast<void*>(ioCtx));
            break;
        case UbioDir::Write:
            ret = spdk_bdev_write(desc, ioChannel, data, offset, sizeInBytes,
                callbackFunc, static_cast<void*>(ioCtx));
            break;
        default:
            std::cout << "Neither Read Nor Write " << static_cast<int>(dir)
                      << std::endl;
            break;
    }
    return ret;
}
} // namespace pos
