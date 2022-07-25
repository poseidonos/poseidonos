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

#include "src/allocator/context_manager/block_allocation_status.h"
#include "src/logger/logger.h"

namespace pos
{
BlockAllocationStatus::BlockAllocationStatus(void)
: userBlkAllocProhibited(false)
{
    for (int volume = 0; volume < MAX_VOLUME_COUNT; volume++)
    {
        blkAllocProhibited[volume] = false;
        pthread_rwlock_init(&lock_volume[volume], nullptr);
    }
}

BlockAllocationStatus::~BlockAllocationStatus(void)
{
    for (int volume = 0; volume < MAX_VOLUME_COUNT; volume++)
    {
        pthread_rwlock_destroy(&lock_volume[volume]);
    }
}

bool
BlockAllocationStatus::IsUserBlockAllocationProhibited(int volumeId)
{
    return ((blkAllocProhibited[volumeId] == true) || (userBlkAllocProhibited == true));
}

bool
BlockAllocationStatus::IsBlockAllocationProhibited(int volumeId)
{
    return blkAllocProhibited[volumeId];
}

void
BlockAllocationStatus::PermitUserBlockAllocation(void)
{
    userBlkAllocProhibited = false;
}

void
BlockAllocationStatus::PermitBlockAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        blkAllocProhibited[i] = false;
    }
}

void
BlockAllocationStatus::PermitBlockAllocation(int volumeId)
{
    blkAllocProhibited[volumeId] = false;
}

void
BlockAllocationStatus::ProhibitUserBlockAllocation(void)
{
    userBlkAllocProhibited = true;
}

void
BlockAllocationStatus::ProhibitBlockAllocation(void)
{
    for (auto i = 0; i < MAX_VOLUME_COUNT; i++)
    {
        // Wait for flag to be reset
        while (blkAllocProhibited[i].exchange(true) == true)
        {
        }
    }
}

bool
BlockAllocationStatus::TryProhibitBlockAllocation(int volumeId)
{
    return (blkAllocProhibited[volumeId].exchange(true) == false);
}

void
BlockAllocationStatus::Lock(void)
{
    for (auto volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        auto result = pthread_rwlock_wrlock(&lock_volume[volumeId]);
        if (result)
        {
            POS_TRACE_ERROR(EID(BLOCK_ALLOCATION_LOCK), "volumeId:{}, result:{}", volumeId, result);
        }
    }
}

void
BlockAllocationStatus::Unlock(void)
{
    for (auto volumeId = 0; volumeId < MAX_VOLUME_COUNT; volumeId++)
    {
        auto result = pthread_rwlock_unlock(&lock_volume[volumeId]);
        if (result)
        {
            POS_TRACE_ERROR(EID(BLOCK_ALLOCATION_LOCK), "volumeId:{}, result:{}", volumeId, result);
        }
    }
}

bool
BlockAllocationStatus::TryRdLock(int volumeId)
{
    auto result = pthread_rwlock_tryrdlock(&lock_volume[volumeId]);
    if (result)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool
BlockAllocationStatus::Unlock(int volumeId)
{
    auto result = pthread_rwlock_unlock(&lock_volume[volumeId]);
    if (result)
    {
        return false;
    }
    else
    {
        return true;
    }
}
} // namespace pos
