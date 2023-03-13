#pragma once

#include "src/allocator/allocator.h"

#include <gmock/gmock.h>

namespace pos
{
class IContextManagerFake;
class IContextReplayerFake;
class SegmentCtxFake;
class MockAllocatorAddressInfo;
class TestInfo;
class WBStripeAllocatorMock;
class AllocatorMock : public Allocator
{
public:
    explicit AllocatorMock(TestInfo* testInfo, IArrayInfo* info);
    virtual ~AllocatorMock(void);

    virtual IWBStripeAllocator* GetIWBStripeAllocator(void) override;
    WBStripeAllocatorMock* GetWBStripeAllocatorMock(void);

    virtual ISegmentCtx* GetISegmentCtx(void) override;
    SegmentCtxFake* GetSegmentCtxFake(void);

    virtual IContextManager* GetIContextManager(void) override;
    IContextManagerFake* GetIContextManagerFake(void);

    virtual IContextReplayer* GetIContextReplayer(void) override;
    IContextReplayerFake* GetIContextReplayerFake(void);

private:
    WBStripeAllocatorMock* wbStripeAllocatorMock;
    SegmentCtxFake* segmentCtxFake;
    IContextManagerFake* contextManagerFake;
    IContextReplayerFake* contextReplayerFake;
    MockAllocatorAddressInfo* addrInfoMock;
    TestInfo* testInfo;
};

} // namespace pos
