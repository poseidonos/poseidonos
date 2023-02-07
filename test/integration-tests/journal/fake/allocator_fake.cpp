#include "allocator_fake.h"

#include "src/meta_file_intf/mock_file_intf.h"
#include "test/integration-tests/journal/fake/i_context_manager_fake.h"
#include "test/integration-tests/journal/fake/i_segment_ctx_fake.h"
#include "test/integration-tests/journal/fake/wbstripe_allocator_mock.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/integration-tests/journal/fake/i_context_replayer_mock.h"

using ::testing::StrictMock;
namespace pos
{
AllocatorFake::AllocatorFake(TestInfo* testInfo, IArrayInfo* info)
: Allocator(info, nullptr),
  testInfo(testInfo)
{
    uint32_t arrayId = 0;
    addrInfoMock = new StrictMock<MockAllocatorAddressInfo>();
    EXPECT_CALL(*addrInfoMock, GetblksPerSegment).WillRepeatedly(Return(testInfo->numBlksPerStripe * testInfo->numStripesPerSegment));
    EXPECT_CALL(*addrInfoMock, GetstripesPerSegment).WillRepeatedly(Return(testInfo->numStripesPerSegment));
    EXPECT_CALL(*addrInfoMock, GetnumUserAreaSegments).WillRepeatedly(Return(testInfo->numUserSegments));

    wbStripeAllocatorMock = new StrictMock<WBStripeAllocatorMock>();
    segmentCtxFake = new StrictMock<ISegmentCtxFake>(addrInfoMock, new MockFileIntf(GetSegmentContextFileName(), arrayId, MetaFileType::General, MetaVolumeType::NvRamVolume));
    contextManagerFake = new StrictMock<IContextManagerFake>(segmentCtxFake, addrInfoMock);
    contextReplayerMock = new StrictMock<IContextReplayerMock>();
}

AllocatorFake::~AllocatorFake(void)
{
    delete segmentCtxFake;
    delete wbStripeAllocatorMock;
    delete contextManagerFake;
    delete contextReplayerMock;
    delete addrInfoMock;
}

IWBStripeAllocator*
AllocatorFake::GetIWBStripeAllocator(void)
{
    return wbStripeAllocatorMock;
}

WBStripeAllocatorMock*
AllocatorFake::GetWBStripeAllocatorMock(void)
{
    return wbStripeAllocatorMock;
}

ISegmentCtx*
AllocatorFake::GetISegmentCtx(void)
{
    return segmentCtxFake;
}

ISegmentCtxFake*
AllocatorFake::GetISegmentCtxFake(void)
{
    return segmentCtxFake;
}

IContextManager*
AllocatorFake::GetIContextManager(void)
{
    return contextManagerFake;
}

IContextManagerFake*
AllocatorFake::GetIContextManagerFake(void)
{
    return contextManagerFake;
}

IContextReplayer*
AllocatorFake::GetIContextReplayer(void)
{
    return contextReplayerMock;
}

IContextReplayerMock*
AllocatorFake::GetIContextReplayerMock(void)
{
    return contextReplayerMock;
}
} // namespace pos
