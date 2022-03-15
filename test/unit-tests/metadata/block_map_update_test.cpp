#include "src/metadata/block_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(BlockMapUpdate, BlockMapUpdate_testIfUTConstructedSuccessfully)
{
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockVsaRangeMaker>* vsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, &vsaMap);

    BlockMapUpdate blockMapUpdate(mockVolumeIoPtr, &vsaMap, &segmentCtx,
        &wbStripeAllocator, vsaRangeMaker);
}

TEST(BlockMapUpdate, DoSpecificJob_testIfMetaIsUpdatedSuccessfully)
{
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockStripe> stripe;
    NiceMock<MockVsaRangeMaker>* vsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, &vsaMap);

    int volumeId = 10;
    BlkAddr rba = 1203;
    VirtualBlks newVsas = {
        .startVsa = {
            .stripeId = 102,
            .offset = 1},
        .numBlks = 12};

    VirtualBlks oldVsas = {
        .startVsa = {
            .stripeId = 222,
            .offset = 12},
        .numBlks = 12};

    StripeAddr lsid = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 102};

    ON_CALL(*mockVolumeIo, GetVolumeId).WillByDefault(Return(volumeId));
    ON_CALL(*mockVolumeIo, GetSectorRba).WillByDefault(Return(ChangeBlockToSector(rba)));
    ON_CALL(*mockVolumeIo, GetSize).WillByDefault(Return(ChangeBlockToByte(newVsas.numBlks)));
    ON_CALL(*mockVolumeIo, GetVsa).WillByDefault(ReturnRef(newVsas.startVsa));
    ON_CALL(*mockVolumeIo, GetWbLsid).WillByDefault(Return(lsid.stripeId));

    ON_CALL(*vsaRangeMaker, GetCount).WillByDefault(Return(1));
    ON_CALL(*vsaRangeMaker, GetVsaRange).WillByDefault(ReturnRef(oldVsas));

    ON_CALL(wbStripeAllocator, GetStripe(lsid.stripeId)).WillByDefault(Return(&stripe));

    // Then 1. Map should be updated with new vsa
    EXPECT_CALL(vsaMap, SetVSAs(volumeId, rba, newVsas));

    // Then 2. Reverse map should be updated
    EXPECT_CALL(stripe, UpdateReverseMapEntry).Times(newVsas.numBlks);

    // Then 3. Old map should be invalidated
    EXPECT_CALL(*vsaRangeMaker, GetVsaRange).WillOnce(ReturnRef(oldVsas));
    EXPECT_CALL(segmentCtx, InvalidateBlks(oldVsas));

    // Then 4. New map should be validated
    EXPECT_CALL(segmentCtx, ValidateBlks(newVsas));

    BlockMapUpdate blockMapUpdate(mockVolumeIoPtr, &vsaMap, &segmentCtx,
        &wbStripeAllocator, vsaRangeMaker);
    bool actual = blockMapUpdate.Execute();
    EXPECT_EQ(actual, true);
}

TEST(BlockMapUpdate, DoSpecificJob_testIfMetaIsUpdatedSuccessfullyWhenOldVsasAreAllUnmap)
{
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockStripe> stripe;
    NiceMock<MockVsaRangeMaker>* vsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, &vsaMap);

    int volumeId = 10;
    BlkAddr rba = 1203;
    VirtualBlks newVsas = {
        .startVsa = {
            .stripeId = 102,
            .offset = 1},
        .numBlks = 12};

    StripeAddr lsid = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 102};

    ON_CALL(*mockVolumeIo, GetVolumeId).WillByDefault(Return(volumeId));
    ON_CALL(*mockVolumeIo, GetSectorRba).WillByDefault(Return(ChangeBlockToSector(rba)));
    ON_CALL(*mockVolumeIo, GetSize).WillByDefault(Return(ChangeBlockToByte(newVsas.numBlks)));
    ON_CALL(*mockVolumeIo, GetVsa).WillByDefault(ReturnRef(newVsas.startVsa));
    ON_CALL(*mockVolumeIo, GetWbLsid).WillByDefault(Return(lsid.stripeId));

    ON_CALL(*vsaRangeMaker, GetCount).WillByDefault(Return(0));

    ON_CALL(wbStripeAllocator, GetStripe(lsid.stripeId)).WillByDefault(Return(&stripe));

    // Then 1. Map should be updated with new vsa
    EXPECT_CALL(vsaMap, SetVSAs(volumeId, rba, newVsas));

    // Then 2. Reverse map should be updated
    EXPECT_CALL(stripe, UpdateReverseMapEntry).Times(newVsas.numBlks);

    // Then 3. Old map should not be invalidated

    EXPECT_CALL(*vsaRangeMaker, GetVsaRange).Times(0);
    EXPECT_CALL(segmentCtx, InvalidateBlks).Times(0);

    // Then 4. New map should be validated
    EXPECT_CALL(segmentCtx, ValidateBlks(newVsas));

    BlockMapUpdate blockMapUpdate(mockVolumeIoPtr, &vsaMap, &segmentCtx,
        &wbStripeAllocator, vsaRangeMaker);
    bool actual = blockMapUpdate.Execute();
    EXPECT_EQ(actual, true);
}

TEST(BlockMapUpdate, DoSpecificJob_testIfMetaIsUpdatedSuccessfullyWhenOldVsasAreNotSequential)
{
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockStripe> stripe;
    NiceMock<MockVsaRangeMaker>* vsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, &vsaMap);

    int volumeId = 10;
    BlkAddr rba = 1203;
    VirtualBlks newVsas = {
        .startVsa = {
            .stripeId = 102,
            .offset = 1},
        .numBlks = 12};

    int numOldVsaRange = 2;
    VirtualBlks oldVsas[numOldVsaRange];
    oldVsas[0] = {
        .startVsa = {
            .stripeId = 1,
            .offset = 0},
        .numBlks = 1};
    oldVsas[1] = {
        .startVsa = {
            .stripeId = 40,
            .offset = 100},
        .numBlks = 10};

    StripeAddr lsid = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 102};

    ON_CALL(*mockVolumeIo, GetVolumeId).WillByDefault(Return(volumeId));
    ON_CALL(*mockVolumeIo, GetSectorRba).WillByDefault(Return(ChangeBlockToSector(rba)));
    ON_CALL(*mockVolumeIo, GetSize).WillByDefault(Return(ChangeBlockToByte(newVsas.numBlks)));
    ON_CALL(*mockVolumeIo, GetVsa).WillByDefault(ReturnRef(newVsas.startVsa));
    ON_CALL(*mockVolumeIo, GetWbLsid).WillByDefault(Return(lsid.stripeId));

    ON_CALL(*vsaRangeMaker, GetCount).WillByDefault(Return(numOldVsaRange));

    ON_CALL(wbStripeAllocator, GetStripe(lsid.stripeId)).WillByDefault(Return(&stripe));

    // Then 1. Map should be updated with new vsa
    EXPECT_CALL(vsaMap, SetVSAs(volumeId, rba, newVsas));

    // Then 2. Reverse map should be updated
    EXPECT_CALL(stripe, UpdateReverseMapEntry).Times(newVsas.numBlks);

    // Then 3. Old map should not be invalidated
    EXPECT_CALL(*vsaRangeMaker, GetVsaRange).Times(numOldVsaRange).WillOnce(ReturnRef(oldVsas[0])).WillOnce(ReturnRef(oldVsas[1]));
    EXPECT_CALL(segmentCtx, InvalidateBlks(oldVsas[0])).Times(1);
    EXPECT_CALL(segmentCtx, InvalidateBlks(oldVsas[1])).Times(1);

    // Then 4. New map should be validated
    EXPECT_CALL(segmentCtx, ValidateBlks(newVsas));

    BlockMapUpdate blockMapUpdate(mockVolumeIoPtr, &vsaMap, &segmentCtx,
        &wbStripeAllocator, vsaRangeMaker);
    bool actual = blockMapUpdate.Execute();
    EXPECT_EQ(actual, true);
}
} // namespace pos
