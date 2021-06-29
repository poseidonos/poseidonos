#pragma once

#include "gmock/gmock.h"
#include "src/allocator/i_block_allocator.h"
#include "src/include/address_type.h"

namespace pos
{
class BlockAllocatorMock : public IBlockAllocator
{
public:
    BlockAllocatorMock(void) {}
    virtual ~BlockAllocatorMock(void) {}

    MOCK_METHOD(void, InvalidateBlks, (VirtualBlks bks), (override));
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks bks), (override));
    MOCK_METHOD(Stripe*, AllocateGcDestStripe, (uint32_t volumeId), (override));
    virtual VirtualBlks
    AllocateWriteBufferBlks(uint32_t volumeId, uint32_t numBlks) override
    {
        VirtualBlks blks;
        blks.startVsa = UNMAP_VSA;
        blks.numBlks = 0;
        return blks;
    }

    void ProhibitUserBlkAlloc(void) override {}
    void PermitUserBlkAlloc(void) override {}

    bool BlockAllocating(uint32_t volumeId) override { return true; }
    void UnblockAllocating(uint32_t volumeId) override {}
};
} // namespace pos
