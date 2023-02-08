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

#include "segment_info.h"

#include <cassert>
#include <memory.h>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
SegmentInfo::SegmentInfo(void)
:data(nullptr)
{
}

SegmentInfo::~SegmentInfo(void)
{
}

void
SegmentInfo::AllocateAndInitSegmentInfoData(SegmentInfoData* segmentInfoData)
{
    assert(nullptr != segmentInfoData);
    data = segmentInfoData;
    SetValidBlockCount(0);
    SetOccupiedStripeCount(0);
    SetState(SegmentState::FREE);
}

uint32_t
SegmentInfo::GetValidBlockCount(void)
{
    return data->validBlockCount;
}

void
SegmentInfo::SetValidBlockCount(uint32_t cnt)
{
    // for wbt
    data->validBlockCount = cnt;
}

uint32_t
SegmentInfo::IncreaseValidBlockCount(uint32_t inc)
{
    return data->validBlockCount.fetch_add(inc) + inc;
}

std::pair<bool, SegmentState>
SegmentInfo::DecreaseValidBlockCount(uint32_t dec, bool allowVictimSegRelease)
{
    std::lock_guard<std::mutex> lock(seglock);
    int32_t decreased = data->validBlockCount.fetch_sub(dec) - dec;

    if (decreased == 0)
    {
        if (true == allowVictimSegRelease)
        {
            if (data->state == SegmentState::VICTIM || data->state == SegmentState::SSD)
            {
                std::pair<bool, SegmentState> result = {true, data->state};
                _MoveToFreeState();

                return result;
            }
        }
        else
        {
            if (data->state == SegmentState::SSD)
            {
                std::pair<bool, SegmentState> result = {true, data->state};
                _MoveToFreeState();

                return result;
            }
        }
    }
    else if (decreased < 0)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED),
            "Valid block count decreasedCount:{} total validCount:{} : UNDERFLOWED", dec, decreased);
        return {false, SegmentState::ERROR};
    }

    return {false, data->state};
}

void
SegmentInfo::SetOccupiedStripeCount(uint32_t cnt)
{
    data->occupiedStripeCount = cnt;
}

uint32_t
SegmentInfo::GetOccupiedStripeCount(void)
{
    return data->occupiedStripeCount;
}

uint32_t
SegmentInfo::IncreaseOccupiedStripeCount(void)
{
    // ++ is equivalent to fetch_add(1) + 1
    return ++(data->occupiedStripeCount);
}

void
SegmentInfo::SetState(SegmentState newState)
{
    std::lock_guard<std::mutex> lock(seglock);
    data->state = newState;
}

SegmentState
SegmentInfo::GetState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    return data->state;
}

void
SegmentInfo::_MoveToFreeState(void)
{
    data->occupiedStripeCount = 0;
    data->validBlockCount = 0;

    data->state = SegmentState::FREE;
}

void
SegmentInfo::MoveToNvramState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    if (data->state != SegmentState::FREE)
    {
        POS_TRACE_ERROR(EID(UNKNOWN_ALLOCATOR_ERROR),
            "Failed to move to NVRAM state. Segment state {} valid count {} occupied stripe count {}",
            data->state, data->validBlockCount, data->occupiedStripeCount);
        assert(false);
    }

    data->state = SegmentState::NVRAM;
}

bool
SegmentInfo::MoveToSsdStateOrFreeStateIfItBecomesEmpty(void)
{
    std::lock_guard<std::mutex> lock(seglock);

    if (data->validBlockCount == 0)
    {
        _MoveToFreeState();

        return true;
    }
    else
    {
        data->state = SegmentState::SSD;
        return false;
    }
}

bool
SegmentInfo::MoveToVictimState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    if (data->state != SegmentState::SSD)
    {
        POS_TRACE_ERROR(EID(UNKNOWN_ALLOCATOR_ERROR),
            "Cannot move to victim state as it's not SSD state, state: {}", data->state);
        return false;
    }
    else
    {
        data->state = SegmentState::VICTIM;

        return true;
    }
}

uint32_t
SegmentInfo::GetValidBlockCountIfSsdState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    if (data->state != SegmentState::SSD)
    {
        return UINT32_MAX;
    }
    else
    {
        return data->validBlockCount;
    }
}

void
SegmentInfo::UpdateFrom(SegmentInfo &segmentInfo)
{
    this->data->validBlockCount = (uint32_t)segmentInfo.data->validBlockCount;
    this->data->occupiedStripeCount = (uint32_t)segmentInfo.data->occupiedStripeCount;
    this->data->state = segmentInfo.data->state;
}


} // namespace pos
