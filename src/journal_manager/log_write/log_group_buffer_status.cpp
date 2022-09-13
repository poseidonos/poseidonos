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

#include "log_group_buffer_status.h"
#include "src/logger/logger.h"
#include "src/include/pos_event_id.h"

namespace pos
{
LogGroupBufferStatus::LogGroupBufferStatus(uint64_t startOffset, uint64_t maxOffset, uint64_t metaPageSize)
: startOffset(startOffset),
  maxOffset(maxOffset),
  metaPageSize(metaPageSize)
{
    Reset();
}

void
LogGroupBufferStatus::Reset(void)
{
    seqNum = 0;
    waitingToBeFilled = false;
    numLogsAdded = 0;
    numLogsFilled = 0;
    nextOffset = startOffset;

    memset(&statusChangedTime, 0x00, sizeof(statusChangedTime));

    // status update should be at last
    _SetStatus(LogGroupStatus::INIT);
}

void
LogGroupBufferStatus::SetActive(uint64_t inputSeqNum)
{
    seqNum = inputSeqNum;
    _SetStatus(LogGroupStatus::ACTIVE);
}

int
LogGroupBufferStatus::TryToAllocate(uint32_t logSize, uint64_t& offset)
{
    int result = 0;

    if (logSize > metaPageSize)
    {
        POS_TRACE_ERROR(EID(JOURNAL_INVALID_SIZE_LOG_REQUESTED),
            "Requested log size is bigger than meta page");
        return -1 * EID(JOURNAL_INVALID_SIZE_LOG_REQUESTED);
    }

    uint64_t currentMetaPage = _GetMetaPageNumber(nextOffset);
    uint64_t endMetaPage = _GetMetaPageNumber(nextOffset + logSize - 1);

    if (currentMetaPage != endMetaPage)
    {
        nextOffset = endMetaPage * metaPageSize;
    }
    result = _AllocateIfNotFull(logSize, offset);
    if (result != 0)
    {
        waitingToBeFilled = true;
    }

    return result;
}

int
LogGroupBufferStatus::_AllocateIfNotFull(uint32_t logSize, uint64_t& offset)
{
    if (nextOffset + logSize <= maxOffset)
    {
        offset = nextOffset;
        nextOffset += logSize;
        numLogsAdded++;

        return EID(SUCCESS);
    }
    else
    {
        return EID(JOURNAL_LOG_GROUP_FULL);
    }
}

bool
LogGroupBufferStatus::TryToSetFull(void)
{
    std::unique_lock<std::mutex> lock(fullTriggerLock);
    if (waitingToBeFilled.load(std::memory_order_seq_cst) == true
        && _IsFullyFilled() == true)
    {
        waitingToBeFilled = false;
        _SetStatus(LogGroupStatus::FULL);

        POS_TRACE_DEBUG(EID(JOURNAL_LOG_GROUP_FULL),
            "Log group is fully filled, added {} filled {}",
            numLogsAdded, numLogsFilled);

        return true;
    }
    return false;
}

void
LogGroupBufferStatus::LogFilled(void)
{
    uint64_t result = numLogsFilled.fetch_add(1) + 1;

    assert(numLogsAdded.load(std::memory_order_seq_cst) >= result);
}

void
LogGroupBufferStatus::_SetStatus(LogGroupStatus toStatus)
{
    status = toStatus;
    gettimeofday(&statusChangedTime[(int)toStatus], NULL);
}

} // namespace pos
