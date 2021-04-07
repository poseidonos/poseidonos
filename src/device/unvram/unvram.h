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

#pragma once

#include <cstdint>
#include <string>

#include "src/device/ublock_device.h"

namespace ibofos
{
class DeviceContext;
class UnvramDrv;

class UnvramBdev : public UBlockDevice
{
public:
    explicit UnvramBdev(std::string name, uint64_t size, UnvramDrv* driverToUse);
    ~UnvramBdev(void) override;
    int SubmitAsyncIO(UbioSmartPtr bio) override;

private:
    static const uint32_t MAX_THREAD_COUNT = 128;
    static const int32_t MAX_NUMA_COUNT = 2;
    uint32_t ioatReactor[MAX_THREAD_COUNT];
    uint32_t ioatReactorCount;
    uint32_t reactorCount;
    static thread_local uint32_t requestCount;
    bool useIoat;
    uint32_t ioatInNuma0;
    uint32_t ioatInNuma1;

    DeviceContext* _AllocateDeviceContext(void) override;
    void _ReleaseDeviceContext(DeviceContext* deviceContextToRelease) override;
    bool _WrapupOpenDeviceSpecific(DeviceContext* deviceContext) override;
    static void _RequestAsyncIo(void* arg1, void* arg2);
    void _SetIoatReactor(void);
    bool _RecoverBackup(DeviceContext* deviceContext);
    void _GetIoatCntInNuma(void);
};

} // namespace ibofos
