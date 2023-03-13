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
: data(nullptr),
  arrayId(-1),
  segmentId(0)
{
}

SegmentInfo::~SegmentInfo(void)
{
}

void
SegmentInfo::AllocateSegmentInfoData(SegmentInfoData* segmentInfoData)
{
    assert(nullptr != segmentInfoData);
    this->data = segmentInfoData;
}

void
SegmentInfo::AllocateAndInitSegmentInfoData(SegmentInfoData* segmentInfoData)
{
    AllocateSegmentInfoData(segmentInfoData);
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
            "Valid block count UNDERFLOWED, decreasedCount:{}, totalValidCount:{} segmentId:{}", dec, decreased, segmentId);
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
    _SetState(newState);
}

void
SegmentInfo::_SetState(SegmentState newState)
{
    SegmentState prevState = data->state;
    data->state = newState;

    POS_TRACE_DEBUG(EID(ALLOCATOR_SEGINFO_SETSTATE), "segment_id:{}, prev_state:{}, next_state:{}, array_id:{}",
        segmentId, ToSegmentStateString(prevState), ToSegmentStateString(data->state), arrayId);
}

void
SegmentInfo::SetArrayId(int arrayId)
{
    this->arrayId = arrayId;
}

void
SegmentInfo::SetSegmentId(SegmentId segmentId)
{
    this->segmentId = segmentId;
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

    _SetState(SegmentState::FREE);
}

void
SegmentInfo::MoveToNvramState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    if (data->state != SegmentState::FREE)
    {
        POS_TRACE_ERROR(EID(UNKNOWN_ALLOCATOR_ERROR),
            "Failed to move to NVRAM state, state:{}, valid_block_count:{}, occupied_stripe_count:{}, segmentId:{}, arrayId {}",
            ToSegmentStateString(data->state), data->validBlockCount, data->occupiedStripeCount, segmentId, arrayId);
        assert(false);
    }

    _SetState(SegmentState::NVRAM);
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
        _SetState(SegmentState::SSD);
        return false;
    }
}

bool
SegmentInfo::MoveVictimToFree(void)
{
    std::lock_guard<std::mutex> lock(seglock);

    if (data->state == SegmentState::VICTIM && data->validBlockCount == 0)
    {
        _MoveToFreeState();
        return true;
    }
    else
    {
        POS_TRACE_DEBUG(EID(CANNOT_FREE_SEGMENT),
            "segment_id:{}, state:{}, valid_block_count:{}, array_id:{}",
            segmentId, ToSegmentStateString(data->state), data->validBlockCount, arrayId);
        return false;
    }
}

bool
SegmentInfo::MoveToVictimState(void)
{
    std::lock_guard<std::mutex> lock(seglock);
    if (data->state != SegmentState::SSD)
    {
        POS_TRACE_WARN(EID(UNKNOWN_ALLOCATOR_ERROR),
            "Cannot move to victim state as it's not SSD state, state:{}, segment_id:{}, array_id:{}",
            ToSegmentStateString(data->state), segmentId, arrayId);
        return false;
    }
    else
    {
        _SetState(SegmentState::VICTIM);
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

const std::unordered_map<SegmentState, std::string> segmentStateToString =
    {
        {SegmentState::FREE, "FREE"},
        {SegmentState::NVRAM, "NVRAM"},
        {SegmentState::SSD, "SSD"},
        {SegmentState::VICTIM, "VICTIM"},
        {SegmentState::ERROR, "ERROR"}};

bool
SegmentInfoData::ToBytes(char* destBuf)
{
    pos_bc::SegmentInfoDataProto proto;
    proto.set_valid_block_count( this->validBlockCount );
    proto.set_occupied_stripe_count( this->occupiedStripeCount );
    proto.set_state( (pos_bc::SegmentState) this->state );
    size_t effectiveSize = proto.ByteSizeLong();
    if (effectiveSize <= ONSSD_SIZE)
    {
        // normal case.
        int ret = proto.SerializeToArray(destBuf, ONSSD_SIZE);
        if (ret == 0)
        {
            POS_TRACE_ERROR(EID(SEGMENTINFODATA_FAILED_TO_SERIALIZE), 
                "validBlockCount: {}, occupiedStripeCount: {}, segmentState: {}",
                this->validBlockCount, this->occupiedStripeCount, this->state);
            return false;
        }
        return true;
    }
    else
    {
        // error case. we don't serialize here to avoid memory corruption on destBuf.
        size_t expectedSize = ONSSD_SIZE;
        POS_TRACE_ERROR(EID(SEGMENTINFODATA_SERIALIZE_OVERFLOW),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }
}

bool
SegmentInfoData::FromBytes(char* srcBuf)
{
    pos_bc::SegmentInfoDataProto proto;
    int ret = proto.ParseFromArray(srcBuf, ONSSD_SIZE);
    if (ret == 0)
    {
        // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
        // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
        // The current implementation has a risk of not detecting a parse error when there has been
        // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
        // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf. 
    } 

    size_t effectiveSize = proto.ByteSizeLong();
    if (effectiveSize <= ONSSD_SIZE)
    {
        // normal case. update internal member variables with proto
        Set(proto.valid_block_count(), proto.occupied_stripe_count(), (SegmentState) proto.state());
        return true;
    }
    else
    {
        size_t expectedSize = ONSSD_SIZE;
        POS_TRACE_ERROR(EID(SEGMENTINFODATA_DESERIALIZE_CORRUPTION),
            "effectiveSize: {}, expectedSize: {}", effectiveSize, expectedSize);
        return false;
    }
}

std::string
SegmentInfo::ToSegmentStateString(SegmentState state)
{
    auto stateStr = segmentStateToString.find(state);
    if (stateStr == segmentStateToString.end())
    {
        return "OTHER";
    }
    else
    {
        return stateStr->second;
    }
}
} // namespace pos
