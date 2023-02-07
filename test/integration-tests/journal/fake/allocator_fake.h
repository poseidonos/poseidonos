#pragma once

#include "src/allocator/allocator.h"

#include <gmock/gmock.h>

namespace pos
{
class IContextManagerFake;
class IContextReplayerMock;
class ISegmentCtxFake;
class MockAllocatorAddressInfo;
class TestInfo;
class WBStripeAllocatorMock;
class AllocatorFake : public Allocator
{
public:
    explicit AllocatorFake(TestInfo* testInfo, IArrayInfo* info);
    virtual ~AllocatorFake(void);

    virtual IWBStripeAllocator* GetIWBStripeAllocator(void) override;
    WBStripeAllocatorMock* GetWBStripeAllocatorMock(void);

    virtual ISegmentCtx* GetISegmentCtx(void) override;
    ISegmentCtxFake* GetISegmentCtxFake(void);

    virtual IContextManager* GetIContextManager(void) override;
    IContextManagerFake* GetIContextManagerFake(void);

    virtual IContextReplayer* GetIContextReplayer(void) override;
    IContextReplayerMock* GetIContextReplayerMock(void);

private:
    WBStripeAllocatorMock* wbStripeAllocatorMock;
    ISegmentCtxFake* segmentCtxFake;
    IContextManagerFake* contextManagerFake;
    IContextReplayerMock* contextReplayerMock;
    MockAllocatorAddressInfo* addrInfoMock;
    TestInfo* testInfo;
};

} // namespace pos
