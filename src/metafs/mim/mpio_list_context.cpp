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

#include "mpio_list_context.h"

#include "metafs_common.h"

#if (1 == MFS_DEBUG)
#define OUTSTANDING_MPIO_LIST_TRACKING 1
#else
#define OUTSTANDING_MPIO_LIST_TRACKING 0
#endif

namespace pos
{
MpioListContext::MpioListContext(void)
: totalIssuedMpio(0),
  remainingMpioToComplete(0)
{
}

MpioListContext::~MpioListContext(void)
{
    // when this gets called,
    // in general, all mpio storing in mpioMap should be completed successfully
}

void
MpioListContext::Reset(void)
{
    totalIssuedMpio = 0;
    remainingMpioToComplete = 0;
#if (1 == OUTSTANDING_MPIO_LIST_TRACKING)
    pendingMpioMap.clear();
    outstandingMpioMap.clear();
#endif
}

void
MpioListContext::PushMpio(Mpio& mpio)
{
    PushMpioToOutstandingJobQ(mpio);

    totalIssuedMpio++;
}

void
MpioListContext::PushMpioToOutstandingJobQ(Mpio& mpio)
{
#if (1 == OUTSTANDING_MPIO_LIST_TRACKING)
    SPIN_LOCK_GUARD_IN_SCOPE(outstandingMpioMapLock);
    outstandingMpioMap.insert(std::make_pair(mpio.io.metaLpn, &mpio));
#endif
}

void
MpioListContext::SetTotalMpioCntForExecution(uint32_t mpioCnt)
{
    assert(remainingMpioToComplete == 0);
    remainingMpioToComplete = mpioCnt;
}

void
MpioListContext::MarkMpioCompletion(Mpio& mpio)
{
#if (1 == OUTSTANDING_MPIO_LIST_TRACKING)
    SPIN_LOCK_GUARD_IN_SCOPE(outstandingMpioMapLock);

    auto item = outstandingMpioMap.find(mpio.io.metaLpn);
    assert(item != outstandingMpioMap.end());
    outstandingMpioMap.erase(mpio.io.metaLpn);
#endif
    assert(remainingMpioToComplete > 0);
    remainingMpioToComplete--;
}

bool
MpioListContext::IsAllMpioDone(void)
{
    return remainingMpioToComplete == 0;
}
} // namespace pos
