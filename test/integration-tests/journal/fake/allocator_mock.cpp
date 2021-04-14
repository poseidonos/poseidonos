#include "allocator_mock.h"

using ::testing::StrictMock;

namespace pos
{
AllocatorMock::AllocatorMock(IArrayInfo* info)
: Allocator(info, nullptr)
{
    allocatorCtxMock = new StrictMock<AllocatorCtxMock>();
    wbStripeAllocatorMock = new StrictMock<WBStripeAllocatorMock>();
    segmentCtxMock = new StrictMock<SegmentCtxMock>();
    blockAllocatorMock = new StrictMock<BlockAllocatorMock>();
    wbStripeCtxMock = new StrictMock<WBStripeCtxMock>();
}

AllocatorMock::~AllocatorMock(void)
{
    delete wbStripeCtxMock;
    delete blockAllocatorMock;
    delete segmentCtxMock;
    delete wbStripeAllocatorMock;
    delete allocatorCtxMock;
}

IAllocatorCtx*
AllocatorMock::GetIAllocatorCtx(void)
{
    return allocatorCtxMock;
}

AllocatorCtxMock*
AllocatorMock::GetAllocatorCtxMock(void)
{
    return allocatorCtxMock;
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

SegmentCtxMock*
AllocatorMock::GetSegmentCtxMock(void)
{
    return segmentCtxMock;
}

IBlockAllocator*
AllocatorMock::GetIBlockAllocator(void)
{
    return blockAllocatorMock;
}

BlockAllocatorMock*
AllocatorMock::GetBlockAllocatorMock(void)
{
    return blockAllocatorMock;
}

IWBStripeCtx*
AllocatorMock::GetIWBStripeCtx(void)
{
    return wbStripeCtxMock;
}

WBStripeCtxMock*
AllocatorMock::GetWBStripeCtxMock(void)
{
    return wbStripeCtxMock;
}

} // namespace pos
