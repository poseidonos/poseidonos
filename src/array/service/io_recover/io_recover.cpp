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

#include "io_recover.h"
#include "src/bio/ubio.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
IORecover::~IORecover(void)
{
}

bool
IORecover::Register(string array, ArrayRecover recov)
{
    if (recoveries.find(array) == recoveries.end())
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::RECOVER_DEBUG_MSG,
            "IORecover::Register, array:{} size:{}", array, recov.size());
        auto ret = recoveries.emplace(array, recov);
        return ret.second;
    }
    return true;
}

void
IORecover::Unregister(string array)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::RECOVER_DEBUG_MSG,
        "IORecover::Unregister, array:{}, size:{}", array, recoveries.size());
    _Erase(array);
}

int
IORecover::GetRecoverMethod(string array, UbioSmartPtr ubio, RecoverMethod& out)
{
    ArrayRecover* recover = _Find(array);
    if (recover == nullptr)
    {
        return (int)POS_EVENT_ID::ARRAY_WRONG_NAME;
    }

    int ret = 0;
    for (auto it = recover->begin(); it != recover->end(); ++it)
    {
        ret = it->second->GetRecoverMethod(ubio, out);
        if (ret == 0)
        {
            return ret;
        }
    }

    POS_TRACE_ERROR(ret, "IORecover::GetRecoverMethod Recover failed");
    return ret;
}

ArrayRecover*
IORecover::_Find(string array)
{
    if (array == "" && recoveries.size() == 1)
    {
        return &(recoveries.begin()->second);
    }

    auto it = recoveries.find(array);
    if (it == recoveries.end())
    {
        return nullptr;
    }
    return &(it->second);
}

void
IORecover::_Erase(string array)
{
    if (array == "" && recoveries.size() == 1)
    {
        recoveries.clear();
    }
    else
    {
        recoveries.erase(array);
    }

    POS_TRACE_INFO((int)POS_EVENT_ID::RECOVER_DEBUG_MSG,
        "IORecover::_Erase, array:{}, remaining:{}", array, recoveries.size());
}

} // namespace pos
