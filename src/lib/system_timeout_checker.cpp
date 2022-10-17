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

#include "system_timeout_checker.h"

namespace pos
{
const uint64_t
    SystemTimeoutChecker::NANOS_PER_SECOND = 1000000000ULL;

SystemTimeoutChecker::~SystemTimeoutChecker()
{
}

void
SystemTimeoutChecker::SetTimeout(uint64_t nanoSecsLeftFromNow)
{
    isActive = true;
    targetFromStartInNSec = nanoSecsLeftFromNow;
    clock_gettime(CLOCK_MONOTONIC_RAW, &startTime);
}

bool
SystemTimeoutChecker::CheckTimeout(void)
{
    if (isActive == false)
    {
        return false;
    }

    bool timeoutReached = false;
    if (0 < targetFromStartInNSec)
    {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);

        uint64_t elapsedInNSec =
            ((now.tv_sec - startTime.tv_sec) * NANOS_PER_SECOND) +
            now.tv_nsec - startTime.tv_nsec;

        if (elapsedInNSec > targetFromStartInNSec)
        {
            timeoutReached = true;
            targetFromStartInNSec = 0;
        }
    }
    else
    {
        timeoutReached = true;
    }

    return timeoutReached;
}

uint64_t
SystemTimeoutChecker::Elapsed(void)
{
    if (isActive == false)
    {
        return 0;
    }
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);

    uint64_t elapsedInNSec =
        ((now.tv_sec - startTime.tv_sec) * NANOS_PER_SECOND) +
        now.tv_nsec - startTime.tv_nsec;

    return elapsedInNSec;
}

void
SystemTimeoutChecker::Reset(void)
{
    isActive = false;
}

bool
SystemTimeoutChecker::IsActive(void)
{
    return isActive;
}

} // namespace pos
