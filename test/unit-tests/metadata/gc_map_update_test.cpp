#include "src/metadata/gc_map_update.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>

#include "src/journal_manager/log/gc_map_update_list.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(GcMapUpdate, GcMapUpdate_testIfConstructedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockStripe> stripe;
    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t> invalidSegCnt;

    PartitionLogicalSize partitionLogicalSize;
    partitionLogicalSize.stripesPerSegment = 1;
    EXPECT_CALL(arrayInfo, GetSizeInfo).WillRepeatedly(Return(&partitionLogicalSize));
    GcMapUpdate gcMapUpdate(&vsaMap, &stripeMap, &segmentCtx, &contextManager, &arrayInfo, &stripe, mapUpdateInfoList, invalidSegCnt);
}

TEST(GcMapUpdate, _DoSpecificJob_testWithValidMapAndInvalidSegBlks)
{
    SegmentId segId = 1;
    StripeId vsid = 215;
    StripeId userLsid = 215;
    uint32_t validMapCnt = 60;
    uint32_t testVolumeId = 1;

    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;
    NiceMock<MockStripe> stripe;
    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t> invalidSegCnt;
    PartitionLogicalSize partitionLogicalSize;
    partitionLogicalSize.stripesPerSegment = 1;
    EXPECT_CALL(arrayInfo, GetSizeInfo).WillRepeatedly(Return(&partitionLogicalSize));

    mapUpdateInfoList.volumeId = testVolumeId;
    for (uint32_t index = 0; index < validMapCnt; index++)
    {
        GcBlockMapUpdate validMap = {
            .rba = index,
            .vsa = {
                .stripeId = vsid,
                .offset = index}
        };
        mapUpdateInfoList.blockMapUpdateList.push_back(validMap);
    }
    invalidSegCnt[1] = 10;

    GcMapUpdate gcMapUpdate(&vsaMap, &stripeMap, &segmentCtx, &contextManager, &arrayInfo, &stripe, mapUpdateInfoList, invalidSegCnt);

    ON_CALL(stripe, GetUserLsid).WillByDefault(Return(userLsid));
    ON_CALL(stripe, GetVsid).WillByDefault(Return(vsid));
    EXPECT_CALL(contextManager, UpdateOccupiedStripeCount(userLsid)).Times(1);

    for (auto it : mapUpdateInfoList.blockMapUpdateList)
    {
        VirtualBlks vsaRange = {it.vsa, 1};
        EXPECT_CALL(vsaMap, SetVSAsInternal(testVolumeId, it.rba, vsaRange)).Times(1);
    }

    EXPECT_CALL(segmentCtx, InvalidateBlks).Times(1);
    VirtualBlkAddr writeVsa = {vsid, 0};
    VirtualBlks writeVsaRange = {writeVsa, validMapCnt};
    EXPECT_CALL(segmentCtx, ValidateBlks(writeVsaRange)).Times(1);

    bool result = gcMapUpdate.Execute();
    EXPECT_EQ(result, true);
}

} // namespace pos
