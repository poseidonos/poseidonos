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

#include "publish_pending_io.h"

#include <air/Air.h>
#include <thread>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/event_scheduler/callback_type.h"
#include "src/event_scheduler/io_timeout_checker.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
PublishPendingIo::PublishPendingIo(int periodTime_, int durationCnt_)
{
    flagStop = false;
    periodTime = periodTime_;
    durationCnt = durationCnt_;

    cpu_set_t cpuSet = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);

    new std::thread(&PublishPendingIo::Run, this);
}

PublishPendingIo::~PublishPendingIo(void)
{    
}


bool
PublishPendingIo::IsEnabled(void)
{
    return enabled;
}

void
PublishPendingIo::Run(void)
{
    enabled = true;
    while (1)
    {
        count = (count + 1) % durationCnt;

        IoTimeoutCheckerSingleton::Instance()->MoveCurrentIdx(count);

        if ((count % CHECK_TIMEOUT_THRESHOLD) == 0)
        {
            _PublishMetric();
        }
        usleep(periodTime * 1000);

        if (true == flagStop)
            break;
    }
}

void
PublishPendingIo::_PublishMetric()
{
    for (int idx = 0 ; idx < CallbackType::Total_CallbackType_Cnt; idx++)
    {
        IoTimeoutCheckerSingleton::Instance()->MoveOldestIdx(static_cast<CallbackType>(idx));
        if (IoTimeoutCheckerSingleton::Instance()->FindPendingIo(static_cast<CallbackType>(idx)))
        {
            std::vector<int> pendingIoCntList;
            IoTimeoutCheckerSingleton::Instance()->GetPendingIoCount(static_cast<CallbackType>(idx), pendingIoCntList);
            for (auto it : pendingIoCntList)
            {
                airlog("TimeOutIoCnt", "user", idx, it);
            }
        }
    }

    
}

void
PublishPendingIo::TimerStop(void)
{
    flagStop = true;
}

uint64_t
PublishPendingIo::GetCurrentRoughTime(void)
{
    return count;
}

} // namespace pos
