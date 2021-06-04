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

#include "io_device_checker.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
bool
IODeviceChecker::Register(string array, IDeviceChecker* checker)
{
    if (_Find(array) == nullptr)
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
            "IODeviceChecker::Register, array:{}", array);
        auto ret = devCheckers.emplace(array, checker);
        return ret.second;
    }
    return true;
}

void
IODeviceChecker::Unregister(string array)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
        "IODeviceChecker::Unregister, array:{}", array);
    _Erase(array);
}

bool
IODeviceChecker::IsRecoverable(string array, IArrayDevice* target, UBlockDevice* uBlock)
{
    IDeviceChecker* checker = _Find(array);
    if (checker == nullptr)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
            "IsRecoverableDevice, array {} does not exist", array);
        return false;
    }

    bool ret = checker->IsRecoverable(target, uBlock);
    POS_TRACE_INFO((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
        "IsRecoverableDevice, {}", ret);
    return ret;
}

IArrayDevice*
IODeviceChecker::FindDevice(string array, string devSn)
{
    IDeviceChecker* checker = _Find(array);
    if (checker == nullptr)
    {
        POS_TRACE_ERROR((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
            "FindDevice, array {} does not exist", array);
        return nullptr;
    }

    return checker->FindDevice(devSn);
}

IDeviceChecker*
IODeviceChecker::_Find(string array)
{
    if (array == "" && devCheckers.size() == 1)
    {
        return devCheckers.begin()->second;
    }
    auto it = devCheckers.find(array);
    if (it == devCheckers.end())
    {
        return nullptr;
    }

    return it->second;
}

void
IODeviceChecker::_Erase(string array)
{
    if (array == "" && devCheckers.size() == 1)
    {
        devCheckers.clear();
    }
    else
    {
        devCheckers.erase(array);
    }

    POS_TRACE_INFO((int)POS_EVENT_ID::DEV_CHECKER_DEBUG_MSG,
        "IODeviceChecker::_Erase, array:{}, remaining:{}",
        array, devCheckers.size());
}

} // namespace pos
