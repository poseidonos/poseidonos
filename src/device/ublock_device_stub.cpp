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

#include "ublock_device.h"

namespace ibofos
{
static std::atomic<uint32_t> currentOutstandingIOCount;

UBlockDevice::UBlockDevice(
    std::string name, uint64_t size, DeviceDriver* drvieToUse)
{
    property.name = name;
    property.size = size;
    property.cls = DeviceClass::SYSTEM;
    currentOutstandingIOCount = 0;
}

UBlockDevice::~UBlockDevice(void)
{
}

bool
UBlockDevice::_RegisterThread(void)
{
    return false;
}

bool
UBlockDevice::Open(void)
{
    bool openSuccessful = true;
    return openSuccessful;
}

uint32_t
UBlockDevice::Close(void)
{
    return 0;
}

int
UBlockDevice::SubmitAsyncIO(UbioSmartPtr bio)
{
    int completions = 0;

    currentOutstandingIOCount++;

    return completions;
}

int
UBlockDevice::CompleteIOs(void)
{
    int completions = currentOutstandingIOCount.exchange(0);
    return completions;
}

bool
UBlockDevice::_OpenDeviceDriver(DeviceContext* ctx)
{
    bool openSuccessful = true;
    return openSuccessful;
}

bool
UBlockDevice::_CloseDeviceDriver(DeviceContext* ctx)
{
    bool closeSuccessful = true;
    return closeSuccessful;
}

bool
UBlockDevice::IsAlive()
{
    return true;
}

} // namespace ibofos
