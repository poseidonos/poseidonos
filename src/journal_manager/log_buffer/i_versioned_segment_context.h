/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include <vector>
#include "src/include/address_type.h"

namespace pos
{
class JournalConfiguration;
class SegmentInfo;
class VersionedSegmentInfo;

class IVersionedSegmentContext
{
public:
    virtual ~IVersionedSegmentContext(void) = default;

    virtual void Init(JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfos, uint32_t numSegments) = 0;
    virtual void Dispose(void) = 0;
    virtual void IncreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) = 0;
    virtual void DecreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) = 0;
    virtual void IncreaseOccupiedStripeCount(int logGroupId, SegmentId segId) = 0;
    virtual SegmentInfo* GetUpdatedInfoToFlush(int logGroupId) = 0;
    virtual void ResetFlushedInfo(int logGroupId) = 0;
    virtual int GetNumSegments(void) = 0;
    virtual int GetNumLogGroups(void) = 0;

    // For UT
    virtual void Init(JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo) = 0;
};

} // namespace pos
