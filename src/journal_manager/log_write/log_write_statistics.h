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

#include <list>
#include <mutex>
#include <vector>

#include "src/include/address_type.h"

namespace pos
{
class StripeLogWriteStatus;
class StripeInfo;
class LogWriteContext;

using ContextList = std::list<LogWriteContext*>;

class LogWriteStatistics
{
public:
    LogWriteStatistics(void);
    virtual ~LogWriteStatistics(void);

    virtual void Init(int numLogGroups);
    virtual void Dispose(void);

    virtual bool UpdateStatus(LogWriteContext* context);
    virtual void AddToList(LogWriteContext* context);

    virtual void PrintStats(int groupId);

    bool IsEnabled(void);

private:
    void _Reset(int groupId);

    void _ResetContextList(int groupId);
    void _ResetStripeStatus(int groupId);

    StripeLogWriteStatus* _FindStripeLogs(int groupId, StripeId vsid);

    bool enabled;

    std::mutex stripeStatusLock;
    std::vector<std::vector<StripeLogWriteStatus*>> stripeLogsAdded;

    std::mutex contextLock;
    std::vector<ContextList> contextAdded;
};

} // namespace pos
