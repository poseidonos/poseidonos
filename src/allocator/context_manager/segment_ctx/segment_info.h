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

#include <atomic>
#include <mutex>
#include <utility>

#include "src/include/address_type.h"
#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
enum SegmentState : int
{
    START,
    FREE = START,
    NVRAM,
    SSD,
    VICTIM,
    ERROR,
    NUM_STATES,
};

struct SegmentInfoData
{
public:
    std::atomic<uint32_t> validBlockCount;
    std::atomic<uint32_t> occupiedStripeCount;
    SegmentState state;
    // TODO(sang7.park) : add reserved field here.
    // DO NOT ADD ANY VIRTUAL METHODS HERE TO SUPPORT BACKWARD COMPATIBILITY
    SegmentInfoData(){
        SegmentInfoData(0, 0, SegmentState::FREE);
    }

    SegmentInfoData(const SegmentInfoData& target)
    {
        SegmentInfoData(target.validBlockCount, target.occupiedStripeCount, target.state);
    }

    SegmentInfoData(uint32_t validBlockCount, uint32_t occupiedStripeCount, SegmentState segmentState)
    {
        this->Set(validBlockCount, occupiedStripeCount, segmentState);
    }

    void Set(uint32_t validBlockCount, uint32_t occupiedStripeCount, SegmentState segmentState)
    {
        this->validBlockCount = validBlockCount;
        this->occupiedStripeCount = occupiedStripeCount;
        this->state = segmentState;
    }

    size_t SerializedSize() {
        // "protobuf-encoded size" + "zero-padded size"
        return ONSSD_SIZE;
    }

    // The fixed size for a single SegmentInfoDataProto
    const static size_t ONSSD_SIZE = 128; // in bytes

    // Serialize a single SegmentInfoData
    bool ToBytes(char* destBuf);

    // Deserialize a single SegmentInfoData
    bool FromBytes(char* srcBuf);
};

class SegmentInfo
{
public:
    SegmentInfo(void);
    ~SegmentInfo(void);

    virtual void AllocateSegmentInfoData(SegmentInfoData* segmentInfoData);
    virtual void AllocateAndInitSegmentInfoData(SegmentInfoData* segmentInfoData);
    virtual uint32_t GetValidBlockCount(void);
    virtual void SetValidBlockCount(uint32_t cnt);
    virtual uint32_t IncreaseValidBlockCount(uint32_t inc);
    virtual std::pair<bool, SegmentState> DecreaseValidBlockCount(uint32_t dec, bool allowVictimSegRelease);

    virtual void SetOccupiedStripeCount(uint32_t cnt);
    virtual uint32_t GetOccupiedStripeCount(void);
    virtual uint32_t IncreaseOccupiedStripeCount(void);

    virtual void SetState(SegmentState newState);
    virtual SegmentState GetState(void);

    /* arrayId and segmentId are for debuggability, not for operation */
    virtual void SetArrayId(int arrayId);
    virtual void SetSegmentId(SegmentId segmentId);

    virtual void MoveToNvramState(void);
    virtual bool MoveToSsdStateOrFreeStateIfItBecomesEmpty(void);
    virtual bool MoveToVictimState(void);
    virtual bool MoveVictimToFree(void);

    virtual uint32_t GetValidBlockCountIfSsdState(void);
    virtual void UpdateFrom(SegmentInfo &segmentInfo);
    static std::string ToSegmentStateString(SegmentState state);

private:
    void _MoveToFreeState(void);
    void _SetState(SegmentState newState);

    SegmentInfoData* data;
    std::mutex seglock;

    int arrayId;
    SegmentId segmentId;
};

} // namespace pos
