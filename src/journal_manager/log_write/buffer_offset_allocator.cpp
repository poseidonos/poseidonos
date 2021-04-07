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

#include "buffer_offset_allocator.h"

#include <functional>

#include "../checkpoint/log_group_releaser.h"
#include "src/include/ibof_event_id.h"
#include "src/logger/logger.h"

namespace ibofos
{
BufferOffsetAllocator::BufferOffsetAllocator(void)
: releaser(nullptr),
  currentLogGroupId(INT32_MAX),
  numLogGroups(0),
  maxOffsetPerGroup(UINT32_MAX)
{
}

BufferOffsetAllocator::~BufferOffsetAllocator(void)
{
    statusList.clear();
}

void
BufferOffsetAllocator::Init(int num, uint32_t groupSize,
    LogGroupReleaser* logGroupReleaser)
{
    releaser = logGroupReleaser;

    numLogGroups = num;
    maxOffsetPerGroup = groupSize;

    statusList.resize(numLogGroups);

    Reset();
}

void
BufferOffsetAllocator::Reset(void)
{
    for (auto it = statusList.begin(); it != statusList.end(); ++it)
    {
        (*it).Reset();
    }

    nextSeqNumber = 0;
    currentLogGroupId = 0;
}

OffsetInFile
BufferOffsetAllocator::AllocateBuffer(int size)
{
    std::unique_lock<std::mutex> lock(allocateLock);

    if (statusList[currentLogGroupId].status == LogGroupBufferStatus::INIT)
    {
        _SetActive(currentLogGroupId);

        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_DEBUG,
            "New log group {} is allocated", currentLogGroupId);
    }

    if (statusList[currentLogGroupId].status == LogGroupBufferStatus::FULL)
    {
        return {currentLogGroupId, -1 * (int)IBOF_EVENT_ID::JOURNAL_LOG_GROUP_FULL, 0};
    }

    if (_CanAllocate(currentLogGroupId, size) == false)
    {
        if (statusList[currentLogGroupId].waitingToBeFilled.exchange(true) == false)
        {
            _TryToSetFull(currentLogGroupId);

            int result = _GetNewActiveGroup();
            if (result < 0)
            {
                return {currentLogGroupId, -1 * (int)IBOF_EVENT_ID::JOURNAL_LOG_GROUP_FULL, 0};
            }
        }
    }

    int offset = _GetBufferOffsetToWrite(currentLogGroupId, size);
    return {currentLogGroupId, offset, statusList[currentLogGroupId].seqNum};
}

void
BufferOffsetAllocator::_SetActive(int id)
{
    statusList[id].seqNum = _GetNextSeqNum();
    statusList[id].status = LogGroupBufferStatus::ACTIVE;
}

uint64_t
BufferOffsetAllocator::_GetBufferOffsetToWrite(int id, int size)
{
    uint32_t bufferOffset = statusList[id].nextOffset;
    statusList[id].nextOffset += size;

    std::lock_guard<std::mutex> lock(statusList[id].countLock);
    statusList[id].numLogsAdded++;

    return bufferOffset;
}

bool
BufferOffsetAllocator::_CanAllocate(int id, int size)
{
    return (statusList[id].nextOffset + size < maxOffsetPerGroup);
}

int
BufferOffsetAllocator::_GetNewActiveGroup(void)
{
    currentLogGroupId = (currentLogGroupId + 1) % numLogGroups;

    if (statusList[currentLogGroupId].status != LogGroupBufferStatus::INIT)
    {
        IBOF_TRACE_WARN((int)IBOF_EVENT_ID::JOURNAL_NO_LOG_BUFFER_AVAILABLE,
            "No log buffer available for journal");
        return -1 * (int)IBOF_EVENT_ID::JOURNAL_NO_LOG_BUFFER_AVAILABLE;
    }
    else
    {
        _SetActive(currentLogGroupId);
        return 0;
    }
}

uint32_t
BufferOffsetAllocator::_GetNextSeqNum(void)
{
    std::unique_lock<std::mutex> lock(seqNumberLock);
    return nextSeqNumber++;
}

void
BufferOffsetAllocator::_TryToSetFull(int id)
{
    std::unique_lock<std::mutex> lock(fullTriggerLock);
    if ((statusList[id].waitingToBeFilled == true) && _IsFullyFilled(id))
    {
        IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::JOURNAL_LOG_GROUP_FULL,
            "Log group id {} is full (numLogsAdded {}, numLogsFilled {})",
            id, statusList[id].numLogsAdded, statusList[id].numLogsFilled);

        statusList[id].status = LogGroupBufferStatus::FULL;
        statusList[id].waitingToBeFilled = false;

        releaser->AddToFullLogGroup(id);
    }
}

bool
BufferOffsetAllocator::_IsFullyFilled(int id)
{
    return (statusList[id].numLogsAdded == statusList[id].numLogsFilled);
}

void
BufferOffsetAllocator::LogFilled(int id, MapPageList& dirty)
{
    {
        std::lock_guard<std::mutex> lock(statusList[id].countLock);
        statusList[id].numLogsFilled++;
        assert(statusList[id].numLogsAdded >= statusList[id].numLogsFilled);
    }
    _TryToSetFull(id);
}

void
BufferOffsetAllocator::LogBufferReseted(int logGroupId)
{
    statusList[logGroupId].Reset();
}

LogGroupBufferStatus
BufferOffsetAllocator::GetStatus(int logGroupId)
{
    return statusList[logGroupId].status;
}

uint32_t
BufferOffsetAllocator::GetNumLogsAdded(void)
{
    uint32_t numLogsAdded = 0;
    for (int id = 0; id < numLogGroups; id++)
    {
        std::lock_guard<std::mutex> lock(statusList[id].countLock);
        if (statusList[id].status != LogGroupBufferStatus::INVALID)
        {
            numLogsAdded += statusList[id].numLogsAdded;
        }
    }
    return numLogsAdded;
}

} // namespace ibofos
