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

#pragma once

#include <atomic>

namespace pos
{
class StripeLoadStatus
{
public:
    StripeLoadStatus(void) = default;
    virtual ~StripeLoadStatus(void) = default;

    virtual void Reset(void)
    {
        numStripesToload = 0;
        numStripesLoaded = 0;
        numStripesFailed = 0;
    }

    virtual void StripeLoadStarted(void)
    {
        numStripesToload++;
    }

    virtual void StripeLoaded(void)
    {
        numStripesLoaded++;
    }

    virtual void StripeLoadFailed(void)
    {
        numStripesFailed++;
    }

    virtual bool IsDone(void)
    {
        return (numStripesToload == (numStripesLoaded + numStripesFailed));
    }

    int GetNumStripesToLoad(void)
    {
        return numStripesToload;
    }
    int GetNumStripesLoaded(void)
    {
        return numStripesLoaded;
    }
    int GetNumStripesFailed(void)
    {
        return numStripesFailed;
    }

private:
    std::atomic<int> numStripesToload;
    std::atomic<int> numStripesLoaded;
    std::atomic<int> numStripesFailed;
};

} // namespace pos
