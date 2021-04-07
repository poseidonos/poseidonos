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

#include "array_device.h"

#include <pthread.h>

#include <tuple>

#include "src/device/ublock_device.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
ArrayDevice::ArrayDevice(UBlockDevice* uBlock,
    ArrayDeviceState state)
: uBlock(uBlock),
  pendingIo(0),
  state(state)
{
    pthread_rwlock_init(&stateLock, nullptr);
};

ArrayDevice::~ArrayDevice(void)
{
}

std::tuple<bool, ArrayDeviceState>
ArrayDevice::GetStatusAndAddPendingIo(bool isWrite)
{
    pthread_rwlock_rdlock(&stateLock);
    bool increased = false;
    ArrayDeviceState currState = state;
    if (state == ArrayDeviceState::NORMAL || (isWrite && state == ArrayDeviceState::REBUILD))
    {
        pendingIo++;
        increased = true;
    }

    pthread_rwlock_unlock(&stateLock);

    return std::make_tuple(increased, currState);
}

bool
ArrayDevice::TryRemoveUblock(void)
{
    pthread_rwlock_rdlock(&stateLock);
    if (state != ArrayDeviceState::NORMAL && _IsPendingIoZero() == false)
    {
        pthread_rwlock_unlock(&stateLock);
        return false;
    }
    IBOF_TRACE_INFO((int)IBOF_EVENT_ID::ARRAY_DEVICE_REMOVED,
        "Device {} is removed",
        uBlock->GetName());
    uBlock = nullptr;
    pthread_rwlock_unlock(&stateLock);
    return true;
}

void
ArrayDevice::RemovePendingIo(void)
{
    pendingIo--;
}

bool
ArrayDevice::_IsPendingIoZero(void)
{
    bool isZero = (pendingIo == 0);
    return isZero;
}

ArrayDeviceState
ArrayDevice::GetState(void)
{
    pthread_rwlock_rdlock(&stateLock);
    ArrayDeviceState retStatus = state;
    pthread_rwlock_unlock(&stateLock);
    return retStatus;
}

void
ArrayDevice::SetState(ArrayDeviceState input)
{
    pthread_rwlock_wrlock(&stateLock);
    state = input;
    pthread_rwlock_unlock(&stateLock);
}

} // namespace ibofos
