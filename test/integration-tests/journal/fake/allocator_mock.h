#pragma once

#include "gmock/gmock.h"
#include "src/allocator/allocator.h"

#include "allocator_ctx_mock.h"
#include "wbstripe_allocator_mock.h"
#include "segment_ctx_mock.h"
#include "block_allocator_mock.h"
#include "wbstripe_ctx_mock.h"

namespace pos
{
class AllocatorMock : public Allocator
{
public:
    explicit AllocatorMock(IArrayInfo* info);
    virtual ~AllocatorMock(void);

    virtual IAllocatorCtx* GetIAllocatorCtx(void) override;
    AllocatorCtxMock* GetAllocatorCtxMock(void);

    virtual IWBStripeAllocator* GetIWBStripeAllocator(void) override;
    WBStripeAllocatorMock* GetWBStripeAllocatorMock(void);

    virtual ISegmentCtx* GetISegmentCtx(void) override;
    SegmentCtxMock* GetSegmentCtxMock(void);

    virtual IBlockAllocator* GetIBlockAllocator(void) override;
    BlockAllocatorMock* GetBlockAllocatorMock(void);

    virtual IWBStripeCtx* GetIWBStripeCtx(void) override;
    WBStripeCtxMock* GetWBStripeCtxMock(void);

private:
    AllocatorCtxMock* allocatorCtxMock;
    WBStripeAllocatorMock* wbStripeAllocatorMock;
    SegmentCtxMock* segmentCtxMock;
    BlockAllocatorMock* blockAllocatorMock;
    WBStripeCtxMock* wbStripeCtxMock;
};

} // namespace pos
