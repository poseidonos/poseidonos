/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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
#include <cstdint>
#include <memory>

#include "callback.h"
#include "src/metadata/segment_context_updater.h"

namespace pos
{
class EventScheduler;
class VirtualBlks;

class MetaUpdateCallback : public Callback
{
public:
    MetaUpdateCallback(bool isFrontEnd, ISegmentCtx* segmentCtx_,
        CallbackType type = CallbackType_Unknown, uint32_t weight = 1,
        SystemTimeoutChecker* timeoutChecker = nullptr, EventScheduler* eventscheduler = nullptr);
    virtual ~MetaUpdateCallback(void);

    virtual void SetLogGroupId(int groupId);
    virtual int GetLogGroupId(void);

protected:
    virtual void ValidateBlks(VirtualBlks blks);
    virtual bool InvalidateBlks(VirtualBlks blks, bool isForced);
    virtual bool UpdateOccupiedStripeCount(StripeId lsid);

    ISegmentCtx* segmentCtx;
    int logGroupId;
};
} // namespace pos
