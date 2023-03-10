#include "allocator_mock.h"

#include "src/allocator/i_segment_ctx.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "test/integration-tests/journal/fake/i_context_manager_fake.h"
#include "test/integration-tests/journal/fake/i_context_replayer_fake.h"
#include "test/integration-tests/journal/fake/segment_ctx_fake.h"
#include "test/integration-tests/journal/fake/wbstripe_allocator_mock.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"

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
    segmentCtxFake = new StrictMock<SegmentCtxFake>(addrInfoMock, new MockFileIntf(GetSegmentContextFileName(), arrayId, MetaFileType::General, MetaVolumeType::SsdVolume));
    contextManagerFake = new StrictMock<IContextManagerFake>(segmentCtxFake, addrInfoMock);
    contextReplayerFake = new StrictMock<IContextReplayerFake>(segmentCtxFake);
}

AllocatorMock::~AllocatorMock(void)
{
    delete segmentCtxFake;
    delete wbStripeAllocatorMock;
    delete contextManagerFake;
    delete contextReplayerFake;
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
    return dynamic_cast<ISegmentCtx*>(segmentCtxFake);
}

SegmentCtxFake*
AllocatorMock::GetSegmentCtxFake(void)
{
    return segmentCtxFake;
}

IContextManager*
AllocatorMock::GetIContextManager(void)
{
    return dynamic_cast<IContextManager*>(contextManagerFake);
}

IContextManagerFake*
AllocatorMock::GetIContextManagerFake(void)
{
    return contextManagerFake;
}

IContextReplayer*
AllocatorMock::GetIContextReplayer(void)
{
    return dynamic_cast<IContextReplayer*>(contextReplayerFake);
}

IContextReplayerFake*
AllocatorMock::GetIContextReplayerFake(void)
{
    return contextReplayerFake;
}
} // namespace pos
