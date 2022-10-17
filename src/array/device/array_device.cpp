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

#include "array_device.h"

#include <pthread.h>

#include <tuple>

#include "src/device/base/ublock_device.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayDevice::ArrayDevice(UblockSharedPtr uBlock,
    ArrayDeviceState state, uint32_t dataIndex)
: uBlock(uBlock),
  state(state),
  dataIndex(dataIndex)
{
}

ArrayDevice::~ArrayDevice(void)
{
}

UblockSharedPtr
ArrayDevice::GetUblock(void)
{
    return uBlock;
}

UBlockDevice*
ArrayDevice::GetUblockPtr(void)
{
    return uBlock.get();
}

void
ArrayDevice::SetUblock(UblockSharedPtr uBlock)
{
    _UpdateTrace(this->uBlock);
    this->uBlock = uBlock;
}

string
ArrayDevice::GetName(void)
{
    if (uBlock == nullptr)
    {
        return "null";
    }
    return uBlock->GetName();
}

string
ArrayDevice::GetSerial(void)
{
    if (uBlock == nullptr)
    {
        return "null";
    }
    return uBlock->GetSN();
}

ArrayDeviceState
ArrayDevice::GetState(void)
{
    ArrayDeviceState retStatus = state;
    return retStatus;
}

void
ArrayDevice::SetState(ArrayDeviceState input)
{
    string devName = "nullptr";
    if (uBlock != nullptr)
    {
        devName = uBlock->GetName();
    }
    if (state != input)
    {
        POS_TRACE_TRACE(EID(ARRAY_EVENT_DEV_STATE_CHANGED),
            "Array device [" + devName + "]'s state is changed from {} to {} (0=normal, 1=fault, 2=rebuild)", state, input);
        state = input;
    }
}

void
ArrayDevice::_UpdateTrace(UblockSharedPtr uBlock)
{
    if (uBlock != nullptr)
    {
        string name = uBlock->GetName();
        string sn = uBlock->GetSN();
        prevUblockInfo = name + "(" + sn + ")";
    }
}

} // namespace pos
