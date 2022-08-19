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

#include <string>
#include <memory>
#include <set>

#include "rebuild_behavior.h"
#include "src/array/service/io_locker/i_io_locker.h"

namespace pos
{
class StripeBasedRaceRebuild : public RebuildBehavior
{
public:
    explicit StripeBasedRaceRebuild(unique_ptr<RebuildContext> c);
    ~StripeBasedRaceRebuild(void);
    virtual bool Rebuild(void) override;
    virtual void UpdateProgress(uint32_t val) override;

private:
    virtual bool _Init(void);
    virtual bool _Recover(void);
    void _RecoverCompleted(uint32_t targetId, int result);
    bool _Finish(void);
    int _TryLock(uint32_t from, uint32_t to);

    StripeId baseStripe = 0;
    IIOLocker* locker = nullptr;
    static const int TRY_LOCK_MAX_RETRY = 50000;
    int tryLockRetryCnt = 0;
    int resetLockRetryCnt = 0;
    set<IArrayDevice*> targetDevs;
    static const int INIT_REBUILD_MAX_RETRY = 1000;
    int initRetryCnt = 0;
};

} // namespace pos
