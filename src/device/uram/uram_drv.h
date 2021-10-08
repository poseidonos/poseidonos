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

#include <vector>

#include "spdk/bdev.h"
#include "src/device/base/device_driver.h"
#include "src/spdk_wrapper/caller/spdk_bdev_caller.h"
#include "src/spdk_wrapper/caller/spdk_thread_caller.h"
#include "src/lib/singleton.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
class DeviceContext;
class Ubio;
class UBlockDevice;
class UramDeviceContext;
class UramIOContext;

class UramDrv : public DeviceDriver
{
public:
    UramDrv(SpdkBdevCaller* spdkBdevCaller = new SpdkBdevCaller(),
        SpdkThreadCaller* spdkThreadCaller = new SpdkThreadCaller(),
        EventFrameworkApi* eventFrameworkApi =
            EventFrameworkApiSingleton::Instance());
    ~UramDrv() override;

    int ScanDevs(std::vector<UblockSharedPtr>* devs) override;
    bool Open(DeviceContext* deviceContext) override;
    bool Close(DeviceContext* deviceContext) override;

    int CompleteIOs(DeviceContext* deviceContext) override;
    int SubmitAsyncIO(DeviceContext* deviceContext, UbioSmartPtr bio) override;
    int SubmitIO(UramIOContext* ioCtx);

private:
    int _RequestIO(UramDeviceContext* deviceContext,
        spdk_bdev_io_completion_cb callbackFunc,
        UramIOContext* ioCtx);
    bool _OpenBdev(UramDeviceContext* bdevCtx);
    void _CloseBdev(UramDeviceContext* bdevCtx);

    SpdkBdevCaller* spdkBdevCaller;
    SpdkThreadCaller* spdkThreadCaller;
    EventFrameworkApi* eventFrameworkApi;
};

using UramDrvSingleton = Singleton<UramDrv>;
} // namespace pos
