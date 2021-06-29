#pragma once

#include "test/integration-tests/journal/utils/test_info.h"

#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/fake/allocator_mock.h"

#include "src/include/address_type.h"

namespace pos
{
class ReplayTestFixture
{
public:
    ReplayTestFixture(MockMapper* mapper, AllocatorMock* allocator, TestInfo* testInfo);
    virtual ~ReplayTestFixture(void);

    void ExpectReturningUnmapStripes(void);
    void ExpectReturningStripeAddr(StripeId vsid, StripeAddr addr);

    void ExpectReplaySegmentAllocation(StripeId userLsid);
    void ExpectReplayStripeAllocation(StripeId vsid, StripeId wbLsid);
    void ExpectReplayBlockLogsForStripe(int volId, BlockMapList blksToWrite);
    void ExpectReplayStripeFlush(StripeTestFixture stripe);

    void ExpectReplayOverwrittenBlockLog(StripeTestFixture stripe);
    void ExpectReplayFullStripe(StripeTestFixture stripe);

    void ExpectReplayUnflushedActiveStripe(VirtualBlkAddr tail, StripeTestFixture stripe);
    void ExpectReplayFlushedActiveStripe(void);

    static VirtualBlkAddr GetNextBlock(VirtualBlks blks);

private:
    VirtualBlks _GetBlock(VirtualBlks blks, uint32_t offset);

    MockMapper* mapper;
    AllocatorMock* allocator;

    TestInfo* testInfo;
};
} // namespace pos
