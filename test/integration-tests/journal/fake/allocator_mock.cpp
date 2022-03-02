#include "allocator_mock.h"

using ::testing::StrictMock;

namespace pos
{
AllocatorMock::AllocatorMock(IArrayInfo* info)
: Allocator(info, nullptr)
{
    wbStripeAllocatorMock = new StrictMock<WBStripeAllocatorMock>();
    segmentCtxMock = new StrictMock<MockISegmentCtx>();
    contextManagerMock = new StrictMock<IContextManagerMock>();
    contextReplayerMock = new StrictMock<IContextReplayerMock>();
}

AllocatorMock::~AllocatorMock(void)
{
    delete segmentCtxMock;
    delete wbStripeAllocatorMock;
    delete contextManagerMock;
    delete contextReplayerMock;
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

MockISegmentCtx*
AllocatorMock::GetISegmentCtxMock(void)
{
    return segmentCtxMock;
}

IContextManager*
AllocatorMock::GetIContextManager(void)
{
    return contextManagerMock;
}

IContextManagerMock*
AllocatorMock::GetIContextManagerMock(void)
{
    return contextManagerMock;
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
