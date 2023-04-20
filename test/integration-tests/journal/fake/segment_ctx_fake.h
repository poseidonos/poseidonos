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
#include <gmock/gmock.h>

#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{
class AllocatorAddressInfo;
class AsyncMetaFileIoCtx;
class IJournalWriter;
class MetaFileIntf;
class SegmentInfo;
class SegmentInfoData;

// This class is to fake a flush of ISegmentContext,
// LoadContext is to simulate a dirty bringup to load Segment Context.
class SegmentCtxFake : public SegmentCtx
{
public:
    explicit SegmentCtxFake(AllocatorAddressInfo* addrInfo, MetaFileIntf* segmentContextFile);
    virtual ~SegmentCtxFake(void);

    void LoadContext(void);
    int FlushContexts(SegmentInfoData* vscSegmentInfoDatas);
    uint64_t GetStoredVersion(void);
    virtual SegmentInfoData* GetSegmentInfoDataArray(void) override;

    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, ReplayBlockInvalidated, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId lsid), (override));

    void SetJournalWriter(IJournalWriter* _journalWriter);
    virtual void SegmentFreeUpdateCompleted(SegmentId segmentId, int logGroupId) override;

private:
    void _ValidateBlks(VirtualBlks blks);
    bool _InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease);
    bool _UpdateOccupiedStripeCount(StripeId lsid);
    void _ReplayBlockInvalidated(VirtualBlks blks, bool allowVictimSegRelease);
    void _ReplayStripeFlushed(StripeId userLsid);

    void _CompleteReadSegmentContext(AsyncMetaFileIoCtx* ctx);
    void _CompleteWriteSegmentContext(AsyncMetaFileIoCtx* ctx);
    void _WaitForReadDone(void);
    void _WaitForWriteDone(void);

    AllocatorAddressInfo* addrInfo;
    MetaFileIntf* segmentContextFile;
    std::atomic<uint64_t> ctxStoredVersion;
    SegmentInfo* segmentInfos;
    SegmentInfoData* segmentInfoData;
    IJournalWriter* journalWriter;

    uint32_t numSegments;
    uint64_t fileSize;

    // No lock is required because EventScheduler use this on single-threaded
    bool segmentContextWriteDone;
    bool segmentContextReadDone;
    bool isFlushedBefore;
};
} // namespace pos
