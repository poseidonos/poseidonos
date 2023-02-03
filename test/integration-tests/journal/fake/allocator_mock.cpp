#include "allocator_mock.h"

#include "test/integration-tests/journal/utils/test_info.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_mock.h"

using ::testing::StrictMock;
namespace pos
{
AllocatorMock::AllocatorMock(TestInfo* testInfo, IArrayInfo* info)
: Allocator(info, nullptr),
  testInfo(testInfo)
{
    uint32_t arrayId = 0;
    addrInfoMock = new StrictMock<MockAllocatorAddressInfo>();
    EXPECT_CALL(*addrInfoMock, GetblksPerSegment).WillRepeatedly(Return(testInfo->numBlksPerStripe * testInfo->numStripesPerSegment));
    EXPECT_CALL(*addrInfoMock, GetstripesPerSegment).WillRepeatedly(Return(testInfo->numStripesPerSegment));
    EXPECT_CALL(*addrInfoMock, GetnumUserAreaSegments).WillRepeatedly(Return(testInfo->numUserSegments));

    wbStripeAllocatorMock = new StrictMock<WBStripeAllocatorMock>();
    segmentCtxMock = new StrictMock<ISegmentCtxMock>(addrInfoMock, new MockFileIntf(GetSegmentContextFileName(), arrayId, MetaFileType::General, MetaVolumeType::NvRamVolume));
    contextManagerFake = new StrictMock<IContextManagerFake>(segmentCtxMock, addrInfoMock);
    contextReplayerMock = new StrictMock<IContextReplayerMock>();
}

AllocatorMock::~AllocatorMock(void)
{
    delete segmentCtxMock;
    delete wbStripeAllocatorMock;
    delete contextManagerFake;
    delete contextReplayerMock;
    delete addrInfoMock;
}

IWBStripeAllocator*
AllocatorMock::GetIWBStripeAllocator(void)
{
    return wbStripeAllocatorMock;
}

WBStripeAllocatorMock*
AllocatorMock::GetWBStripeAllocatorMock(void)
{
    return wbStripeAllocatorMock;
}

ISegmentCtx*
AllocatorMock::GetISegmentCtx(void)
{
    return segmentCtxMock;
}

ISegmentCtxMock*
AllocatorMock::GetISegmentCtxMock(void)
{
    return segmentCtxMock;
}

IContextManager*
AllocatorMock::GetIContextManager(void)
{
    return contextManagerFake;
}

IContextManagerFake*
AllocatorMock::GetIContextManagerFake(void)
{
    return contextManagerFake;
}

IContextReplayer*
AllocatorMock::GetIContextReplayer(void)
{
    return contextReplayerMock;
}

IContextReplayerMock*
AllocatorMock::GetIContextReplayerMock(void)
{
    return contextReplayerMock;
}

} // namespace pos
