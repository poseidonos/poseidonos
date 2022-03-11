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

#include "io_recover.h"

#include "src/bio/ubio.h"
#include "src/include/array_mgmt_policy.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
IORecover::IORecover(void)
{
    recoveries = new ArrayRecover[ArrayMgmtPolicy::MAX_ARRAY_CNT];
}

IORecover::~IORecover(void)
{
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        for (auto recover : recoveries[i])
        {
            delete recover.second;
        }
        recoveries[i].clear();
    }
}

bool
IORecover::Register(unsigned int arrayIndex, ArrayRecover recov)
{
    if (recoveries[arrayIndex].empty())
    {
        int eventId = (int)POS_EVENT_ID::IO_RECOVER_DEBUG_MSG;
        if (recov.empty())
        {
            POS_TRACE_WARN(eventId,
                "IORecover::Register, no recover exists, array:{} size:{}",
                arrayIndex, recov.size());
        }
        POS_TRACE_INFO(eventId,
            "IORecover::Register, array:{} size:{}", arrayIndex, recov.size());
        recoveries[arrayIndex] = recov;
        return true;
    }
    return false;
}

void
IORecover::Unregister(unsigned int arrayIndex)
{
    POS_TRACE_INFO((int)POS_EVENT_ID::IO_RECOVER_DEBUG_MSG,
        "IORecover::Unregister, array:{}", arrayIndex);
    recoveries[arrayIndex].clear();
}

int
IORecover::GetRecoverMethod(unsigned int arrayIndex, UbioSmartPtr ubio, RecoverMethod& out)
{
    ArrayRecover* recover = &recoveries[arrayIndex];
    if (recover->empty())
    {
        return EID(IO_RECOVER_NOT_FOUND);
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

} // namespace pos
