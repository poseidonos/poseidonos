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

#ifndef ARRAY_DEVICE_H_
#define ARRAY_DEVICE_H_

#include <atomic>
#include <thread>
#include <tuple>

namespace ibofos
{
class UBlockDevice;

enum class ArrayDeviceState
{
    NORMAL = 0,
    FAULT,
    REBUILD,
};

class ArrayDevice
{
public:
    ArrayDevice(UBlockDevice* uBlock,
        ArrayDeviceState state = ArrayDeviceState::NORMAL);
    ~ArrayDevice(void);
    void SetState(ArrayDeviceState state);
    ArrayDeviceState GetState(void);

    std::tuple<bool, ArrayDeviceState> GetStatusAndAddPendingIo(bool isWrite);
    void RemovePendingIo(void);
    bool TryRemoveUblock(void);

    UBlockDevice* uBlock;

private:
    bool _IsPendingIoZero(void);

    std::atomic<uint32_t> pendingIo;

    pthread_rwlock_t stateLock;
    ArrayDeviceState state;
};

} // namespace ibofos

#endif // ARRAY_DEVICE_H_
