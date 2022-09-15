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

#include "src/device/base/ublock_device.h"

namespace pos
{
class DeviceContext;
class UramDrv;

class Uram : public UBlockDevice,
                   public std::enable_shared_from_this<Uram>
{
public:
    explicit Uram(std::string name,
        uint64_t size,
        UramDrv* driverToUse,
        uint32_t numa);
    ~Uram(void) override;
    int SubmitAsyncIO(UbioSmartPtr ubio) override;
    void* GetByteAddress(void) override;
    bool WrapupOpenDeviceSpecific(void) override;

private:
    static const uint32_t MAX_THREAD_COUNT = 128;
    static const int32_t MAX_NUMA_COUNT = 2;
    static uint32_t reactorCount;
    static uint32_t ioatReactorCountNuma0;
    static uint32_t ioatReactorCountNuma1;
    std::atomic<uint32_t> requestCount;
    DeviceContext* _AllocateDeviceContext(void) override;
    void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) override;
    void _InitByteAddress(void);
    static void _RequestAsyncIo(void* arg1);
    bool _RecoverBackup(void);
    void *baseByteAddress;
};

} // namespace pos
