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

#include "segment_info.h"

#include <memory.h>

namespace pos
{
SegmentInfo::SegmentInfo(uint32_t maxSegment)
: numSegment(maxSegment)
{
    validBlockCount = new uint32_t[maxSegment];
    memset(validBlockCount, 0, sizeof(uint32_t) * maxSegment);
    occupiedStripeCount = new uint32_t[maxSegment];
    memset(occupiedStripeCount, 0, sizeof(uint32_t) * maxSegment);
}

SegmentInfo::~SegmentInfo(void)
{
    delete[] validBlockCount;
    delete[] occupiedStripeCount;
}

uint32_t
SegmentInfo::GetValidBlockCount(SegmentId segId)
{
    return validBlockCount[segId];
}

uint32_t
SegmentInfo::IncreaseValidBlockCount(SegmentId segId, uint32_t inc)
{
    validBlockCount[segId] += inc;
    return validBlockCount[segId];
}

int32_t
SegmentInfo::DecreaseValidBlockCount(SegmentId segId, uint32_t dec)
{
    validBlockCount[segId] -= dec;
    return validBlockCount[segId];
}

void
SegmentInfo::CopySegmentInfoToBuffer(char* pBuffer)
{
    char* pDstBuf = pBuffer;
    memcpy(pDstBuf, validBlockCount, sizeof(uint32_t) * numSegment);
    pDstBuf += (sizeof(uint32_t) * numSegment);
    memcpy(pDstBuf, occupiedStripeCount, sizeof(uint32_t) * numSegment);
}

void
SegmentInfo::CopySegmentInfoFromBuffer(char* pBuffer)
{
    char* pSrcBuf = pBuffer;
    memcpy(validBlockCount, pSrcBuf, sizeof(uint32_t) * numSegment);
    pSrcBuf += (sizeof(uint32_t) * numSegment);
    memcpy(occupiedStripeCount, pSrcBuf, sizeof(uint32_t) * numSegment);
}

void
SegmentInfo::SetOccupiedStripeCount(SegmentId segmentId, uint32_t cnt)
{
    occupiedStripeCount[segmentId] = cnt;
}

uint32_t
SegmentInfo::GetOccupiedStripeCount(SegmentId segmentId)
{
    return occupiedStripeCount[segmentId];
}

uint32_t
SegmentInfo::IncreaseOccupiedStripeCount(SegmentId segmentId)
{
    return ++occupiedStripeCount[segmentId];
}

} // namespace pos
