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

#include <string>
#include <vector>

#include "src/bio/ubio.h"

namespace pos
{
class DeviceContext;
class UBlockDevice;

class DeviceDriver
{
public:
    typedef void (*DeviceAttachEvent)(UblockSharedPtr dev);
    typedef void (*DeviceDetachEvent)(std::string sn);

    DeviceDriver(void);
    virtual ~DeviceDriver(void);
    virtual int ScanDevs(std::vector<UblockSharedPtr>* devs) = 0;

    virtual bool Open(DeviceContext* deviceContext) = 0;
    virtual bool Close(DeviceContext* deviceContext) = 0;

    virtual int SubmitAsyncIO(DeviceContext* deviceContext, UbioSmartPtr bio) = 0;
    virtual int CompleteIOs(DeviceContext* deviceContext) = 0;
    virtual int CompleteErrors(DeviceContext* deviceContext);
    std::string GetName(void);
    void SetDeviceEventCallback(DeviceAttachEvent attach,
        DeviceDetachEvent detach);

protected:
    std::string name = "";

    static DeviceAttachEvent attach_event;
    static DeviceDetachEvent detach_event;
};

} // namespace pos
