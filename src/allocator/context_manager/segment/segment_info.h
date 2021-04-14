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

#include "src/include/address_type.h"

namespace pos
{
class SegmentInfo
{
public:
    SegmentInfo(uint32_t maxSegment);
    ~SegmentInfo(void);

    uint32_t GetNumSegment(void) { return numSegment;}

    uint32_t GetValidBlockCount(SegmentId segId);
    uint32_t IncreaseValidBlockCount(SegmentId segId, uint32_t inc);
    int32_t DecreaseValidBlockCount(SegmentId segId, uint32_t dec);

    void SetOccupiedStripeCount(SegmentId segmentId, uint32_t cnt);
    uint32_t GetOccupiedStripeCount(SegmentId segmentId);
    uint32_t IncreaseOccupiedStripeCount(SegmentId segmentId);

    void CopySegmentInfoToBuffer(char* pBuffer);
    void CopySegmentInfoFromBuffer(char* pBuffer);

    uint32_t* GetValidBlockCountPool(void) { return validBlockCount;}
    uint32_t* GetOccupiedStripeCountPool(void) { return occupiedStripeCount;}

private:
    uint32_t numSegment;
    uint32_t* validBlockCount;
    uint32_t* occupiedStripeCount;
};

} // namespace pos
