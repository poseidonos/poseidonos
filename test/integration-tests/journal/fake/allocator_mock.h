#pragma once

#include "block_allocator_mock.h"
#include "gmock/gmock.h"
#include "i_context_manager_mock.h"
#include "i_context_replayer_mock.h"
#include "src/allocator/allocator.h"
#include "wbstripe_allocator_mock.h"

namespace pos
{
class AllocatorMock : public Allocator
{
public:
    explicit AllocatorMock(IArrayInfo* info);
    virtual ~AllocatorMock(void);

    virtual IWBStripeAllocator* GetIWBStripeAllocator(void) override;
    WBStripeAllocatorMock* GetWBStripeAllocatorMock(void);

    virtual IBlockAllocator* GetIBlockAllocator(void) override;
    BlockAllocatorMock* GetBlockAllocatorMock(void);

    virtual IContextManager* GetIContextManager(void) override;
    IContextManagerMock* GetIContextManagerMock(void);

    virtual IContextReplayer* GetIContextReplayer(void) override;
    IContextReplayerMock* GetIContextReplayerMock(void);

private:
    WBStripeAllocatorMock* wbStripeAllocatorMock;
    BlockAllocatorMock* blockAllocatorMock;
    IContextManagerMock* contextManagerMock;
    IContextReplayerMock* contextReplayerMock;
};

} // namespace pos
