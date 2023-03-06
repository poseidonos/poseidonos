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

#include "i_segment_ctx_fake.h"

#include <unistd.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/allocator/context_manager/io_ctx/allocator_io_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_info.h"
#include "src/include/address_type.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/meta_file_include.h"
#include "src/meta_file_intf/mock_file_intf.h"

using ::testing::_;
using ::testing::AtLeast;

namespace pos
{
ISegmentCtxFake::ISegmentCtxFake(AllocatorAddressInfo* addrInfo, MetaFileIntf* segmentContextFile)
: addrInfo(addrInfo),
  segmentContextFile(segmentContextFile)
{
    numSegments = addrInfo->GetnumUserAreaSegments();
    segmentContextWriteDone = false;
    segmentContextReadDone = false;
    isFlushedBefore = false;
    ctxStoredVersion = 0;
    segmentInfos = new SegmentInfo[numSegments];
    segmentInfoData = new SegmentInfoData[numSegments];
    for (uint32_t i = 0; i < numSegments; ++i)
    {
        segmentInfos[i].AllocateAndInitSegmentInfoData(&segmentInfoData[i]);
    }

    fileSize = sizeof(SegmentInfoData) * numSegments;
    segmentContextFile->Create(fileSize);
    segmentContextFile->Open();

    ON_CALL(*this, ValidateBlks).WillByDefault(::testing::Invoke(this, &ISegmentCtxFake::_ValidateBlks));
    EXPECT_CALL(*this, ValidateBlks).Times(AtLeast(0));
    ON_CALL(*this, InvalidateBlks).WillByDefault(::testing::Invoke(this, &ISegmentCtxFake::_InvalidateBlks));
    EXPECT_CALL(*this, InvalidateBlks).Times(AtLeast(0));
    ON_CALL(*this, UpdateOccupiedStripeCount).WillByDefault(::testing::Invoke(this, &ISegmentCtxFake::_UpdateOccupiedStripeCount));
    EXPECT_CALL(*this, UpdateOccupiedStripeCount).Times(AtLeast(0));
}

ISegmentCtxFake::~ISegmentCtxFake(void)
{
    if (segmentInfos != nullptr)
    {
        delete[] segmentInfos;
        segmentInfos = nullptr;
    }
    if (segmentInfoData != nullptr)
    {
        delete[] segmentInfoData;
        segmentInfoData = nullptr;
    }

    if (segmentContextFile->IsOpened() == true)
    {
        int ret = segmentContextFile->Close();
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(MFS_FILE_CLOSE_FAILED), "Failed to close segment context");
        }
        segmentContextFile->Delete();
    }
}

void
ISegmentCtxFake::LoadContext(void)
{
    if (segmentContextFile->DoesFileExist() == false)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_NOT_FOUND),
            "Segment context file doesn't exist, name:{}, size:{}", segmentContextFile->GetFileName(), fileSize);
    }
    // It is assumed that this object will be reused without reconstructing the object on SPOR. Because there is no header, So it is impossible to check the version.
    else if (isFlushedBefore == true)
    {
        if (segmentContextFile->IsOpened() == false)
        {
            segmentContextFile->Open();
        }
        SegmentInfoData* buffer = new SegmentInfoData[numSegments];

        FnCompleteMetaFileIo callback = std::bind(&ISegmentCtxFake::_CompleteReadSegmentContext, this, std::placeholders::_1);
        AllocatorIoCtx ctx(nullptr);
        ctx.SetIoInfo(MetaFsIoOpcode::Read, 0, segmentContextFile->GetFileSize(), (char*)buffer);
        ctx.SetFileInfo(segmentContextFile->GetFd(), segmentContextFile->GetIoDoneCheckFunc());
        ctx.SetCallback(callback);
        segmentContextFile->AsyncIO(&ctx);
        _WaitForReadDone();

        delete[] segmentInfoData;
        segmentInfoData = buffer;
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            segmentInfos[i].AllocateAndInitSegmentInfoData(&segmentInfoData[i]);
        }
    }
    else
    {
        for (uint32_t i = 0; i < numSegments; ++i)
        {
            segmentInfos[i].SetValidBlockCount(0);
            segmentInfos[i].SetOccupiedStripeCount(0);
            segmentInfos[i].SetState(SegmentState::FREE);
        }
    }
}

int
ISegmentCtxFake::FlushContexts(SegmentInfoData* vscSegmentInfoDatas)
{
    char* targetBuffer = (vscSegmentInfoDatas != nullptr) ? (char*)vscSegmentInfoDatas : (char*)segmentInfoData;

    FnCompleteMetaFileIo callback = std::bind(&ISegmentCtxFake::_CompleteWriteSegmentContext, this, std::placeholders::_1);
    AllocatorIoCtx ctx(nullptr);
    ctx.SetIoInfo(MetaFsIoOpcode::Write, 0, segmentContextFile->GetFileSize(), targetBuffer);
    ctx.SetFileInfo(segmentContextFile->GetFd(), segmentContextFile->GetIoDoneCheckFunc());
    ctx.SetCallback(callback);

    segmentContextFile->AsyncIO(&ctx);
    _WaitForWriteDone();
    ctxStoredVersion++;
    isFlushedBefore = true;
}

uint64_t
ISegmentCtxFake::GetStoredVersion(void)
{
    return ctxStoredVersion;
}

SegmentInfo*
ISegmentCtxFake::GetSegmentInfoDataArray(void)
{
    return segmentInfos;
}

void
ISegmentCtxFake::_ValidateBlks(VirtualBlks blks)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    uint32_t increasedValue = segmentInfos[segId].IncreaseValidBlockCount(blks.numBlks);
    if (increasedValue > addrInfo->GetblksPerSegment())
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_VALID_BLOCK_COUNT_OVERFLOW),
            "segment_id:{}, vsid: {}, offset: {}, increase_count:{}, before_validate_block_count: {}, maximum_valid_block_count:{}", segId, blks.startVsa.stripeId, blks.startVsa.offset, blks.numBlks, increasedValue - blks.numBlks, addrInfo->GetblksPerSegment());
        throw std::runtime_error("Assertion failed, An overflow occurred with valid block count");
    }
}

bool
ISegmentCtxFake::_InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease)
{
    SegmentId segId = blks.startVsa.stripeId / addrInfo->GetstripesPerSegment();
    auto result = segmentInfos[segId].DecreaseValidBlockCount(blks.numBlks, allowVictimSegRelease);
    if (result.second == SegmentState::ERROR)
    {
        POS_TRACE_ERROR(EID(VALID_COUNT_UNDERFLOWED),
            "segment_id{}, vsid: {}, offset: {}, decase_count: {}, current_valid_block_count: {}, allow {}", segId, blks.startVsa.stripeId, blks.startVsa.offset, blks.numBlks, segmentInfos[segId].GetValidBlockCount(), allowVictimSegRelease);
        throw std::runtime_error("Assertion failed, An underflow occurred with valid block count");
    }
    return true;
}

bool
ISegmentCtxFake::_UpdateOccupiedStripeCount(StripeId lsid)
{
    SegmentId segId = lsid / addrInfo->GetstripesPerSegment();
    uint32_t occupiedStripeCount = segmentInfos[segId].IncreaseOccupiedStripeCount();
    if (occupiedStripeCount > addrInfo->GetstripesPerSegment())
    {
        // TODO (cheolho.kang): Add state changing scenario
        POS_TRACE_ERROR(EID(ALLOCATOR_VALID_BLOCK_COUNT_OVERFLOW),
            "segment_id:{}, lsid:{}, total_stripe_count_per_segment:{}", segId, lsid, addrInfo->GetstripesPerSegment());
        throw std::runtime_error("Assertion failed, An overflow occurred with occpied stripe count");
    }
}

void
ISegmentCtxFake::_CompleteReadSegmentContext(AsyncMetaFileIoCtx* ctx)
{
    segmentContextReadDone = true;
}

void
ISegmentCtxFake::_CompleteWriteSegmentContext(AsyncMetaFileIoCtx* ctx)
{
    segmentContextWriteDone = true;
}

void
ISegmentCtxFake::_WaitForReadDone(void)
{
    while (segmentContextReadDone != true)
    {
        usleep(1);
    }
    segmentContextReadDone = false;
}

void
ISegmentCtxFake::_WaitForWriteDone(void)
{
    while (segmentContextWriteDone != true)
    {
        usleep(1);
    }
    segmentContextWriteDone = false;
}
} // namespace pos
