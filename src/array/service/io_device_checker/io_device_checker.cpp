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

#include "io_device_checker.h"

#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
IODeviceChecker::IODeviceChecker(void)
{
    devCheckers = new IDeviceChecker*[ArrayMgmtPolicy::MAX_ARRAY_CNT];
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        devCheckers[i] = nullptr;
    }
}

IODeviceChecker::~IODeviceChecker(void)
{
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        if (devCheckers[i] != nullptr)
        {
            delete devCheckers[i];
            devCheckers[i] = nullptr;
        }
    }
    delete devCheckers;
    devCheckers = nullptr;
}

bool
IODeviceChecker::Register(unsigned int arrayIndex, IDeviceChecker* checker)
{
    if (devCheckers[arrayIndex] == nullptr)
    {
        int eventId = EID(ARRAY_DEV_DEBUG_MSG);
        if (checker == nullptr)
        {
            POS_TRACE_WARN(eventId,
                "IODeviceChecker::Register, no checker exists, array:{}",
                arrayIndex);
            return false;
        }
        POS_TRACE_INFO(eventId,
            "IODeviceChecker::Register, array:{}", arrayIndex);
        devCheckers[arrayIndex] = checker;
        return true;
    }
    return false;
}

void
IODeviceChecker::Unregister(unsigned int arrayIndex)
{
    POS_TRACE_INFO(EID(ARRAY_DEV_DEBUG_MSG),
        "IODeviceChecker::Unregister, array:{}", arrayIndex);
    devCheckers[arrayIndex] = nullptr;
}

int
IODeviceChecker::IsRecoverable(unsigned int arrayIndex, IArrayDevice* target, UBlockDevice* uBlock)
{
    if (devCheckers[arrayIndex] == nullptr)
    {
        POS_TRACE_ERROR(EID(ARRAY_DEV_DEBUG_MSG),
            "IsRecoverableDevice, array {} does not exist", arrayIndex);
        return false;
    }

    int ret = devCheckers[arrayIndex]->IsRecoverable(target, uBlock);
    return ret;
}

IArrayDevice*
IODeviceChecker::FindDevice(unsigned int arrayIndex, string devSn)
{
    if (devCheckers[arrayIndex] == nullptr)
    {
        POS_TRACE_ERROR(EID(ARRAY_DEV_DEBUG_MSG),
            "FindDevice, array {} does not exist", arrayIndex);
        return nullptr;
    }

    return devCheckers[arrayIndex]->FindDevice(devSn);
}
} // namespace pos
