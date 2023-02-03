#pragma once

#include "gmock/gmock.h"
#include "i_context_manager_fake.h"
#include "i_context_replayer_mock.h"
#include "src/allocator/allocator.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "wbstripe_allocator_mock.h"

namespace pos
{
class ISegmentCtxMock;
class TestInfo;
class AllocatorMock : public Allocator
{
public:
    explicit AllocatorMock(TestInfo* testInfo, IArrayInfo* info);
    virtual ~AllocatorMock(void);

    virtual IWBStripeAllocator* GetIWBStripeAllocator(void) override;
    WBStripeAllocatorMock* GetWBStripeAllocatorMock(void);

    virtual ISegmentCtx* GetISegmentCtx(void) override;
    ISegmentCtxMock* GetISegmentCtxMock(void);

    virtual IContextManager* GetIContextManager(void) override;
    // IContextManagerMock* GetIContextManagerMock(void);
    IContextManagerFake* GetIContextManagerFake(void);

    virtual IContextReplayer* GetIContextReplayer(void) override;
    IContextReplayerMock* GetIContextReplayerMock(void);

private:
    WBStripeAllocatorMock* wbStripeAllocatorMock;
    ISegmentCtxMock* segmentCtxMock;
    IContextManagerFake* contextManagerFake;
    IContextReplayerMock* contextReplayerMock;
    MockAllocatorAddressInfo* addrInfoMock;
    TestInfo* testInfo;
};

} // namespace pos
