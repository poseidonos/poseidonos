#include "i_segment_ctx_mock.h"

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
ISegmentCtxMock::ISegmentCtxMock(AllocatorAddressInfo* addrInfo, MetaFileIntf* segmentContextFile)
: addrInfo(addrInfo),
  segmentContextFile(segmentContextFile)
{
    uint32_t numSegments = addrInfo->GetnumUserAreaSegments();
    uint32_t totalStripes = addrInfo->GetstripesPerSegment();

    segmentInfos = new SegmentInfo[numSegments];
    fileSize = sizeof(SegmentInfo) * numSegments;
    segmentContextFile->Create(fileSize);
    segmentContextFile->Open();

    ON_CALL(*this, ValidateBlks).WillByDefault(::testing::Invoke(this, &ISegmentCtxMock::_ValidateBlks));
    EXPECT_CALL(*this, ValidateBlks).Times(AtLeast(0));
    ON_CALL(*this, InvalidateBlks).WillByDefault(::testing::Invoke(this, &ISegmentCtxMock::_InvalidateBlks));
    EXPECT_CALL(*this, InvalidateBlks).Times(AtLeast(0));
    ON_CALL(*this, UpdateOccupiedStripeCount).WillByDefault(::testing::Invoke(this, &ISegmentCtxMock::_UpdateOccupiedStripeCount));
    EXPECT_CALL(*this, UpdateOccupiedStripeCount).Times(AtLeast(0));
}

ISegmentCtxMock::~ISegmentCtxMock(void)
{
    if (segmentInfos != nullptr)
    {
        delete[] segmentInfos;
        segmentInfos = nullptr;
    }
    if (segmentContextFile->IsOpened() == true)
    {
        int ret = segmentContextFile->Close();
        if (ret != 0)
        {
            POS_TRACE_ERROR(EID(MFS_FILE_CLOSE_FAILED),
                "Failed to close segment context");
        }
    }
}

void
ISegmentCtxMock::LoadContext(void)
{
    if (segmentContextFile->DoesFileExist() == false)
    {
        POS_TRACE_ERROR(EID(MFS_FILE_NOT_FOUND),
            "Segment context file doesn't exist, name:{}, size:{}", segmentContextFile->GetFileName(), fileSize);
    }
    else
    {
        if (segmentContextFile->IsOpened() == false)
        {
            segmentContextFile->Open();
        }
        SegmentInfo* buffer = new SegmentInfo[fileSize / sizeof(SegmentInfo)];
        AllocatorIoCtx ctx(MetaFsIoOpcode::Read, segmentContextFile->GetFd(), 0, segmentContextFile->GetFileSize(), (char*)buffer, std::bind(&ISegmentCtxMock::_SegmentContextReadDone, this));
        segmentContextReadDone = false;
        int ret = segmentContextFile->AsyncIO(&ctx);
        while (segmentContextReadDone != true)
        {
        }
        delete[] segmentInfos;
        segmentInfos = (SegmentInfo*)buffer;
    }
}

int
ISegmentCtxMock::FlushContexts(SegmentInfo* vscSegmentInfos)
{
    char* targetBuffer = (char*)segmentInfos;
    if (vscSegmentInfos != nullptr)
    {
        targetBuffer = (char*)vscSegmentInfos;
    }
    AllocatorIoCtx ctx(MetaFsIoOpcode::Write, segmentContextFile->GetFd(), 0, segmentContextFile->GetFileSize(), targetBuffer, nullptr);
    int ret = segmentContextFile->AsyncIO(&ctx);
    if (ret == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

SegmentInfo*
ISegmentCtxMock::GetSegmentInfos(void)
{
    return segmentInfos;
}

void
ISegmentCtxMock::_ValidateBlks(VirtualBlks blks)
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
ISegmentCtxMock::_InvalidateBlks(VirtualBlks blks, bool allowVictimSegRelease)
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
ISegmentCtxMock::_UpdateOccupiedStripeCount(StripeId lsid)
{
    SegmentId segId = lsid / addrInfo->GetstripesPerSegment();
    uint32_t occupiedStripeCount = segmentInfos[segId].IncreaseOccupiedStripeCount();
    if (occupiedStripeCount > addrInfo->GetstripesPerSegment())
    {
        // TODO (cheolho.kang): Add state changing scenario
        POS_TRACE_ERROR(EID(ALLOCATOR_VALID_BLOCK_COUNT_OVERFLOW),
            "segment_id:{}, lsid:{}, total_stripe_count_per_segment:{}", segId, lsid, addrInfo->GetstripesPerSegment());
        throw std::runtime_error("Assertion failed");
    }
}

void
ISegmentCtxMock::_SegmentContextReadDone(void)
{
    segmentContextReadDone = true;
}
} // namespace pos
