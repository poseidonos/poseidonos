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

#include <pthread.h>

#include <atomic>

#include "src/volume/volume_base.h"

namespace pos
{
class BlockAllocationStatus
{
public:
    BlockAllocationStatus(void);
    virtual ~BlockAllocationStatus(void);

    virtual bool IsUserBlockAllocationProhibited(int volumeId);
    virtual bool IsBlockAllocationProhibited(int volumeId);

    virtual void PermitUserBlockAllocation(void);
    virtual void PermitBlockAllocation(void);
    virtual void PermitBlockAllocation(int volumeId);

    virtual void ProhibitUserBlockAllocation(void);
    virtual bool IsProhibitedUserBlockAllocation(void);
    virtual void ProhibitBlockAllocation(void);

    virtual bool TryProhibitBlockAllocation(int volumeId);

    virtual void Lock(void);
    virtual void Unlock(void);
    virtual bool TryRdLock(int volumeId);
    virtual bool Unlock(int volumeId);

private:
    std::atomic<bool> blkAllocProhibited[MAX_VOLUME_COUNT];
    std::atomic<bool> userBlkAllocProhibited;

    pthread_rwlock_t lock_volume[MAX_VOLUME_COUNT];
};
} // namespace pos
