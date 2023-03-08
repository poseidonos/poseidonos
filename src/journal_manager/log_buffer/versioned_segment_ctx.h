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

#include <cstdint>
#include <memory>
#include <vector>

#include "i_versioned_segment_context.h"
#include "src/include/address_type.h"

namespace pos
{
class VersionedSegmentInfo;
class JournalConfiguration;
class SegmentInfoData;

class DummyVersionedSegmentCtx : public IVersionedSegmentContext
{
public:
    DummyVersionedSegmentCtx(void) = default;
    virtual ~DummyVersionedSegmentCtx(void) = default;

    virtual void Init(JournalConfiguration* journalConfiguration, uint32_t numSegments, uint32_t numStripesPerSegment) override {}
    virtual void Load(SegmentInfoData* loadedSegmentInfos) override {}
    virtual void Dispose(void) override {}
    virtual void IncreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) override {}
    virtual void DecreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) override {}
    virtual void IncreaseOccupiedStripeCount(int logGroupId, SegmentId segId) override {}
    virtual SegmentInfoData* GetUpdatedInfoDataToFlush(int logGroupId) override { return nullptr; }
    virtual int GetNumSegments(void) override { return 0; }
    virtual int GetNumLogGroups(void) override { return 0; };
    virtual void Init(JournalConfiguration* journalConfiguration, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo) override {}

    // LogBufferWriteDoneEvent
    virtual void LogFilled(int logGroupId, const MapList& dirty) override {}
    virtual void LogBufferReseted(int logGroupId) override {}

    // ISegmentFreeSubscriber
    virtual void NotifySegmentFreed(SegmentId segmentId) override {}
};

class VersionedSegmentCtx : public IVersionedSegmentContext
{
public:
    VersionedSegmentCtx(void);
    virtual ~VersionedSegmentCtx(void);

    virtual void Init(JournalConfiguration* journalConfiguration, uint32_t numSegments, uint32_t numStripesPerSegment) override;
    virtual void Load(SegmentInfoData* loadedSegmentInfos) override;
    virtual void Dispose(void) override;

    // For UT
    virtual void Init(JournalConfiguration* journalConfiguration, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo);

    // Implementation of IVersionedSegmentContext interface
    virtual void IncreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) override;
    virtual void DecreaseValidBlockCount(int logGroupId, SegmentId segId, uint32_t cnt) override;
    virtual void IncreaseOccupiedStripeCount(int logGroupId, SegmentId segId) override;

    virtual SegmentInfoData* GetUpdatedInfoDataToFlush(int logGroupId) override;
    virtual int GetNumSegments(void) override;
    virtual int GetNumLogGroups(void) override;

    // Implementation of LogBufferWriteDoneEvent interface
    virtual void LogFilled(int logGroupId, const MapList& dirty) override;
    virtual void LogBufferReseted(int logGroupId) override;

    // Implementation of ISegmentFreeSubscriber interface
    virtual void NotifySegmentFreed(SegmentId segmentId) override;

private:
    void _Init(JournalConfiguration* journalConfiguration, uint32_t numSegments_);
    void _UpdateSegmentContext(int logGroupId);
    void _CheckLogGroupIdValidity(int logGroupId);
    void _CheckSegIdValidity(int segId);

    JournalConfiguration* config;
    uint32_t numSegments;
    std::vector<std::shared_ptr<VersionedSegmentInfo>> segmentInfoDiffs;
    SegmentInfoData* segmentInfoData;
    const int ALL_LOG_GROUP = -1;
};

} // namespace pos
