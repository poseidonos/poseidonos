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

#pragma once

#include <atomic>
#include <mutex>

namespace pos
{

// TODO (huijeong.kim) move it to private of LogGroupBufferStatus
enum class LogGroupStatus
{
    INVALID = -1,
    INIT = 0,
    ACTIVE,
    FULL
};

class LogGroupBufferStatus
{
public:
    LogGroupBufferStatus(uint64_t maxOffset, uint64_t metaPageSize);
    void Reset(void);

    void SetActive(uint64_t inputSeqNum);

    bool TryToAllocate(uint32_t logSize, uint64_t& offset);
    bool TryToSetFull(void);

    void LogFilled(void);

    inline LogGroupStatus
    GetStatus(void)
    {
        return status;
    }

    inline uint64_t
    GetSeqNum(void)
    {
        return seqNum;
    }

    inline uint64_t
    GetNumLogsAdded(void)
    {
        return numLogsAdded;
    }

    inline uint64_t
    GetNextOffset(void)
    {
        return nextOffset;
    }

    inline uint32_t
    GetNumLogsFilled(void)
    {
        return numLogsFilled;
    }

private:
    inline bool _IsFullyFilled(void)
    {
        // TODO (huijeong.kim) check if memory order needed?
        return (numLogsAdded == numLogsFilled);
    }

    inline uint64_t
    _GetMetaPageNumber(uint64_t offset)
    {
        return offset / metaPageSize;
    }

    bool _AllocateIfNotFull(uint32_t logSize, uint64_t& offset);

    std::mutex fullTriggerLock;
    uint32_t seqNum;

    LogGroupStatus status;
    std::atomic<bool> waitingToBeFilled;

    std::atomic<uint64_t> numLogsAdded;
    std::atomic<uint64_t> numLogsFilled;

    uint64_t nextOffset;
    uint64_t maxOffset;

    uint64_t metaPageSize;
};

} // namespace pos
