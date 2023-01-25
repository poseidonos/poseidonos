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

#include <vector>

#include "src/mapper/include/mpage_info.h"

namespace pos
{
class LogBufferWriteDoneEvent
{
public:
    virtual void LogFilled(int logGroupId, const MapList& dirty) = 0;
    virtual void LogBufferReseted(int logGroupId) = 0;
};

class LogBufferWriteDoneNotifier
{
public:
    LogBufferWriteDoneNotifier(void) = default;
    virtual ~LogBufferWriteDoneNotifier(void) = default;

    virtual void Register(LogBufferWriteDoneEvent* notified);
    virtual void Dispose(void);

    virtual void NotifyLogFilled(int logGroupId, const MapList& dirty);
    virtual void NotifyLogBufferReseted(int logGroupId);

    // For UT
    inline std::vector<LogBufferWriteDoneEvent*>
    GetSubscribers(void)
    {
        return subscribers;
    }

private:
    std::vector<LogBufferWriteDoneEvent*> subscribers;
};

} // namespace pos
