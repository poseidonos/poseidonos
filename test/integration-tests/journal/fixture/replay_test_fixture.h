#pragma once

#include "src/include/address_type.h"
#include "test/integration-tests/journal/fake/allocator_mock.h"
#include "test/integration-tests/journal/fake/mapper_mock.h"
#include "test/integration-tests/journal/utils/test_info.h"

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
    void ExpectReplayBlockLogsForStripe(int volId, BlockMapList blksToWrite, bool needToReplaySegment = true);
    void ExpectReplayStripeFlush(StripeTestFixture stripe, bool needToReplaySegment = true);

    void ExpectReplayOverwrittenBlockLog(StripeTestFixture stripe);
    void ExpectReplayFullStripe(StripeTestFixture stripe, bool needToReplaySegment = true);
    void ExpectReplayFullStripe(StripeTestFixture stripe, bool needToReplaySegmentForBlockMap, bool needToReplaySegmentForStripeMap);
    void ExpectReplayReusedStripe(StripeTestFixture stripe);

    void ExpectReplayUnflushedActiveStripe(VirtualBlkAddr tail, StripeTestFixture stripe);
    void ExpectReplayFlushedActiveStripe(void);

    static VirtualBlkAddr GetNextBlock(VirtualBlks blks);

    void UpdateMapper(MockMapper* _mapper);

private:
    VirtualBlks _GetBlock(VirtualBlks blks, uint32_t offset);

    MockMapper* mapper;
    AllocatorMock* allocator;

    TestInfo* testInfo;

    std::set<StripeId> usedVisd;
};
} // namespace pos
