#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/include/allocator_const.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_info_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_list_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class SegmentCtxTestFixture : public ::testing::Test
{
public:
    SegmentCtxTestFixture(void) = default;
    virtual ~SegmentCtxTestFixture(void) = default;

    virtual void SetUp(void)
    {
        // Only 1 mock SegmentInfo is injected to the segCtx
        // Do not use this test fixture when injecting number of segment infos
        ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(1));
        ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(1024));
        ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(arrayId));

        segmentInfoData = new SegmentInfoData();
        segmentInfoData->validBlockCount = 0;
        segmentInfoData->occupiedStripeCount = 0;
        segmentInfoData->state = SegmentState::FREE;

        rebuildSegmentList = new NiceMock<MockSegmentList>();
        segCtx = new SegmentCtx(&tp, &header, segmentInfoData, rebuildSegmentList, &rebuildCtx,
            &addrInfo, &gcCtx);
        for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
        {
            segmentList[state] = new NiceMock<MockSegmentList>();
            segCtx->SetSegmentList((SegmentState)state, segmentList[state]);
        }

        segCtx->Init();
    }

    virtual void TearDown(void)
    {
        delete segCtx;
    }

protected:
    SegmentCtx* segCtx;

    int arrayId = 0;

    SegmentCtxHeader header;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList>* rebuildSegmentList;
    NiceMock<MockSegmentList>* segmentList[SegmentState::NUM_STATES];
    SegmentInfoData* segmentInfoData;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockGcCtx> gcCtx;
};

TEST(SegmentCtx, AfterLoad_testIfSegmentSignatureSuccess)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    int numSegments = 1;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentCtxHeader header;

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++) {
        segmentInfoData[i].Set(0, 0, SegmentState::FREE);
    }
    SegmentCtx segCtx(nullptr, &header, segmentInfoData, nullptr, nullptr, &addrInfo, nullptr);
    segCtx.Init();

    // when
    char buf[segCtx.GetTotalDataSize()];
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(buf);
    headerPtr->sig = SIG_SEGMENT_CTX;
    headerPtr->ctxVersion = 6;

    SegmentInfoData* segmentInfoDataPtr = reinterpret_cast<SegmentInfoData*>(buf + sizeof(SegmentCtxHeader));
    for (auto segId = 0; segId < numSegments; segId++)
    {
        segmentInfoDataPtr[segId].validBlockCount = 0;
        segmentInfoDataPtr[segId].occupiedStripeCount = 0;
        segmentInfoDataPtr[segId].state = SegmentState::FREE;
    }

    segCtx.AfterLoad(buf);

    EXPECT_EQ(segCtx.GetStoredVersion(), 6);
}

TEST(SegmentCtx, AfterLoad_testIfSegmentListIsRebuilt)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    int numSegments = 4;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(4));

    SegmentCtxHeader header;

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::FREE);
    }
    SegmentCtx segCtx(nullptr, &header, segmentInfoData, nullptr, nullptr, &addrInfo, nullptr);
    segCtx.Init();

    NiceMock<MockSegmentList>* segmentList[SegmentState::NUM_STATES];
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        segmentList[state] = new NiceMock<MockSegmentList>();
        segCtx.SetSegmentList((SegmentState)state, segmentList[state]);
    }

    EXPECT_CALL(*segmentList[SegmentState::FREE], AddToList).Times(numSegments);

    // when
    char buf[segCtx.GetTotalDataSize()];
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(buf);
    headerPtr->sig = SIG_SEGMENT_CTX;
    memcpy(buf + sizeof(SegmentCtxHeader), segmentInfoData, sizeof(SegmentInfoData) * numSegments);

    segCtx.AfterLoad(buf);
}


TEST_F(SegmentCtxTestFixture, BeforeFlush_TestSimpleSetter)
{
    // given
    char buf[segCtx->GetTotalDataSize()];
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(buf);
    headerPtr->sig = SIG_SEGMENT_CTX;

    // when
    segCtx->BeforeFlush(buf);
}

TEST_F(SegmentCtxTestFixture, AfterFlush_TestSimpleSetter)
{
    // given
    char buf[segCtx->GetTotalDataSize()];
    SegmentCtxHeader* headerPtr = reinterpret_cast<SegmentCtxHeader*>(buf);
    headerPtr->sig = SIG_SEGMENT_CTX;

    // when
    segCtx->AfterFlush(buf);
}

TEST_F(SegmentCtxTestFixture, UpdateOccupiedStripeCount_IfOccupiedStripeCountSmallerThanMax)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));
    segmentInfoData->occupiedStripeCount = 1;

    // when
    bool segmentFreed = segCtx->UpdateOccupiedStripeCount(0);

    // then
    EXPECT_EQ(segmentFreed, false);
}

TEST_F(SegmentCtxTestFixture, IncreaseOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndSegmentFreed)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));

    // when
    segmentInfoData->occupiedStripeCount = 99;
    segmentInfoData->validBlockCount = 0;
    segmentInfoData->state = SegmentState::NVRAM;

    EXPECT_CALL(*segmentList[SegmentState::NVRAM], RemoveFromList(0));
    EXPECT_CALL(*rebuildSegmentList, RemoveFromList(0)).WillOnce(Return(false));
    EXPECT_CALL(*segmentList[SegmentState::FREE], AddToList).Times(1);

    bool segmentFreed = segCtx->UpdateOccupiedStripeCount(0);

    // then
    EXPECT_EQ(segmentFreed, true);
}

TEST_F(SegmentCtxTestFixture, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndSegmentIsNotFreed)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));

    // when
    segmentInfoData->occupiedStripeCount = 99;
    segmentInfoData->validBlockCount = 10;
    segmentInfoData->state = SegmentState::NVRAM;

    EXPECT_CALL(*segmentList[SegmentState::NVRAM], RemoveFromList(0));
    EXPECT_CALL(*segmentList[SegmentState::SSD], AddToList).Times(1);

    bool segmentFreed = segCtx->UpdateOccupiedStripeCount(0);

    // then
    EXPECT_EQ(segmentFreed, false);
}

TEST_F(SegmentCtxTestFixture, InvalidateBlks_TestDecreaseValue)
{
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(1024));

    segmentInfoData->validBlockCount = 3;
    segmentInfoData->occupiedStripeCount = 1024;
    segmentInfoData->state = SegmentState::NVRAM;

    VirtualBlks blks = {
        .startVsa = {
            .stripeId = 0,
            .offset = 0},
        .numBlks = 1};
    bool ret = segCtx->InvalidateBlks(blks, false);
    EXPECT_EQ(false, ret);
}

TEST_F(SegmentCtxTestFixture, InvalidateBlks_TestDecreaseValueWhenValidCountZeroAndSSDState)
{
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(1024));

    segmentInfoData->validBlockCount = 1;
    segmentInfoData->occupiedStripeCount = 1024;
    segmentInfoData->state = SegmentState::SSD;

    EXPECT_CALL(*segmentList[SegmentState::SSD], RemoveFromList).WillOnce(Return(true));
    EXPECT_CALL(*segmentList[SegmentState::FREE], AddToList).Times(1);
    EXPECT_CALL(*rebuildSegmentList, RemoveFromList).WillOnce(Return(false));

    VirtualBlks blks = {
        .startVsa = {
            .stripeId = 0,
            .offset = 0},
        .numBlks = 1};
    bool ret = segCtx->InvalidateBlks(blks, false);
    EXPECT_EQ(true, ret);
}

TEST_F(SegmentCtxTestFixture, InvalidateBlks_testIfSegmentFreedAndRemovedFromTheRebuildList)
{
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(1024));

    segmentInfoData->validBlockCount = 1;
    segmentInfoData->occupiedStripeCount = 1024;
    segmentInfoData->state = SegmentState::SSD;

    EXPECT_CALL(*segmentList[SegmentState::SSD], RemoveFromList).WillOnce(Return(true));
    EXPECT_CALL(*segmentList[SegmentState::FREE], AddToList).Times(1);
    EXPECT_CALL(*rebuildSegmentList, RemoveFromList).WillOnce(Return(true));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    VirtualBlks blks = {
        .startVsa = {
            .stripeId = 0,
            .offset = 0},
        .numBlks = 1};

    bool ret = segCtx->InvalidateBlks(blks, false);
    EXPECT_EQ(true, ret);
}

TEST(SegmentCtx, _SegmentFreed_testWhenSegmentIsInRebuilding)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, rebuildSegmentList;

    uint32_t stripesPerSegment = 1024;
    int numSegments = 4;
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(stripesPerSegment));
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(1, stripesPerSegment, SegmentState::SSD);
    }

    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segmentCtx(nullptr, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);

    // given: segment 3 is in rebuild
    EXPECT_CALL(rebuildSegmentList, PopSegment).WillOnce(Return(3));
    SegmentId rebuildingSegment = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(rebuildingSegment, 3);

    // When segment is freed, it will not be added to the free list
    VirtualBlks blks = {
        .startVsa = {
            .stripeId = rebuildingSegment * stripesPerSegment,
            .offset = 0,
        },
        .numBlks = 1};

    EXPECT_CALL(ssdSegmentList, RemoveFromList(rebuildingSegment)).Times(1);
    bool ret = segmentCtx.InvalidateBlks(blks, false);
    EXPECT_EQ(ret, true);
}

TEST(SegmentCtx, _SegmentFreed_testWhenSegmentIsRemovedFromTheRebuildList)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, rebuildSegmentList;

    uint32_t stripesPerSegment = 1024;
    int numSegments = 4;
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(stripesPerSegment));
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(1, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    NiceMock<MockTelemetryPublisher> tp;
    SegmentCtx segmentCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);

    // given: segment 3 is in rebuild
    EXPECT_CALL(rebuildSegmentList, PopSegment).WillOnce(Return(3));
    SegmentId rebuildingSegment = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(rebuildingSegment, 3);

    SegmentId targetSegment = 2;
    EXPECT_CALL(rebuildSegmentList, RemoveFromList(targetSegment)).WillOnce(Return(true));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);
    EXPECT_CALL(freeSegmentList, AddToList(targetSegment)).Times(1);
    EXPECT_CALL(gcCtx, UpdateCurrentGcMode);

    // When segment is freed, it will be added to the free list
    VirtualBlks blks = {
        .startVsa = {
            .stripeId = targetSegment * stripesPerSegment,
            .offset = 0,
        },
        .numBlks = 1};
    bool ret = segmentCtx.InvalidateBlks(blks, false);
    EXPECT_EQ(ret, true);
}

TEST_F(SegmentCtxTestFixture, LoadRebuildList_testWhenEmptyRebuildListIsLoaded)
{
    std::set<SegmentId> emptySet;
    EXPECT_CALL(rebuildCtx, GetList).WillOnce(Return(emptySet));
    EXPECT_CALL(*rebuildSegmentList, AddToList).Times(0);

    bool ret = segCtx->LoadRebuildList();
    EXPECT_EQ(ret, false);
}

TEST_F(SegmentCtxTestFixture, LoadRebuildList_testWhenRebuildListIsLoaded)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, rebuildSegmentList;

    int numSegments = 4;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(1, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segmentCtx(nullptr, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);

    std::set<SegmentId> nonEmptySet = {1, 2, 3};
    EXPECT_CALL(rebuildCtx, GetList).WillOnce(Return(nonEmptySet));

    EXPECT_CALL(ssdSegmentList, RemoveFromList(1)).WillOnce(Return(true));
    EXPECT_CALL(rebuildSegmentList, AddToList(1));

    EXPECT_CALL(ssdSegmentList, RemoveFromList(2)).WillOnce(Return(true));
    EXPECT_CALL(rebuildSegmentList, AddToList(2));

    EXPECT_CALL(ssdSegmentList, RemoveFromList(3)).WillOnce(Return(true));
    EXPECT_CALL(rebuildSegmentList, AddToList(3));

    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillRepeatedly(Return(3));

    bool ret = segmentCtx.LoadRebuildList();
    EXPECT_EQ(ret, true);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegmentCount_TestSimpleGetter)
{
    EXPECT_CALL(*rebuildSegmentList, GetNumSegments).WillOnce(Return(10));

    int ret = segCtx->GetRebuildTargetSegmentCount();
    EXPECT_EQ(ret, 10);
}

TEST_F(SegmentCtxTestFixture, StopRebuilding_testWhenRebuidlStoppedWithEmptyRebuildList)
{
    EXPECT_CALL(*rebuildSegmentList, GetNumSegments).WillOnce(Return(0));

    segCtx->StopRebuilding();
}

TEST_F(SegmentCtxTestFixture, StopRebuilding_testWhenRebuidlStopped)
{
    EXPECT_CALL(*rebuildSegmentList, GetNumSegments).WillOnce(Return(1)).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_CALL(*rebuildSegmentList, PopSegment).WillOnce(Return(0));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    segCtx->StopRebuilding();
}

TEST_F(SegmentCtxTestFixture, GetValidBlockCount_TestSimpleGetter)
{
    segmentInfoData->validBlockCount = 10;

    int ret = segCtx->GetValidBlockCount(0);
    EXPECT_EQ(10, ret);
}

TEST_F(SegmentCtxTestFixture, GetOccupiedStripeCount_TestSimpleGetter)
{
    segmentInfoData->occupiedStripeCount = 5;

    int ret = segCtx->GetOccupiedStripeCount(0);
    EXPECT_EQ(5, ret);
}

TEST_F(SegmentCtxTestFixture, UpdateOccupiedStripeCount)
{
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));

    segmentInfoData->occupiedStripeCount = 5;

    bool segmentFreed = segCtx->UpdateOccupiedStripeCount(0);
    EXPECT_EQ(false, segmentFreed);
}

TEST_F(SegmentCtxTestFixture, GetSegmentState_TestSimpleGetter)
{
    // given
    EXPECT_CALL(addrInfo, GetstripesPerSegment).WillRepeatedly(Return(100));

    segmentInfoData->validBlockCount = 0;
    segmentInfoData->occupiedStripeCount = 0;
    segmentInfoData->state = SegmentState::FREE;

    SegmentState ret = segCtx->GetSegmentState(0);
    EXPECT_EQ(SegmentState::FREE, ret);
}

TEST_F(SegmentCtxTestFixture, CopySegmentInfoToBufferforWBT_CheckCopiedBuffer)
{
    uint32_t result = 0;
    // given 1.
    segmentInfoData->validBlockCount = 8;
    segmentInfoData->occupiedStripeCount = 1;
    segmentInfoData->state = SegmentState::NVRAM;

    // when 1.
    segCtx->CopySegmentInfoToBufferforWBT(WBT_SEGMENT_VALID_COUNT, (char*)&result);
    // then 1.
    EXPECT_EQ(8, result);

    // given 2.
    segmentInfoData->validBlockCount = 0;
    segmentInfoData->occupiedStripeCount = 12;
    segmentInfoData->state = SegmentState::NVRAM;

    // when 2.
    segCtx->CopySegmentInfoToBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, (char*)&result);
    // then 2.
    EXPECT_EQ(12, result);
}

TEST(SegmentCtx, Init_testInitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    SegmentCtx segCtx(nullptr, nullptr, nullptr, nullptr, &addrInfo, nullptr);

    // when
    segCtx.Init();

    // then
    segCtx.Dispose();
}

TEST_F(SegmentCtxTestFixture, GetSectionInfo_TestSimpleGetter)
{
    uint64_t expectedOffset = 0;

    auto ret = segCtx->GetSectionInfo(SC_HEADER);
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(sizeof(SegmentCtxHeader), ret.size);
    expectedOffset += sizeof(SegmentCtxHeader);

    // when 2.
    ret = segCtx->GetSectionInfo(SC_SEGMENT_INFO);
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(sizeof(SegmentInfoData), ret.size);
}

TEST_F(SegmentCtxTestFixture, GetStoredVersion_TestSimpleGetter)
{
    segCtx->GetStoredVersion();
}

TEST_F(SegmentCtxTestFixture, CopySegmentInfoFromBufferforWBT_TestSimpleSetter)
{
    uint32_t buf;

    // given 1.
    buf = 10;

    // when 1.
    segCtx->CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_VALID_COUNT, (char*)&buf);

    // then 1.
    EXPECT_EQ(segmentInfoData->validBlockCount, 10);

    // given 2.
    buf = 1002;

    // when 2.
    segCtx->CopySegmentInfoFromBufferforWBT(WBT_SEGMENT_OCCUPIED_STRIPE, (char*)&buf);

    // then 2.
    EXPECT_EQ(segmentInfoData->occupiedStripeCount, 1002);
}

TEST_F(SegmentCtxTestFixture, ResetDirtyVersion_TestSimpleSetter)
{
    segCtx->ResetDirtyVersion();
}

TEST_F(SegmentCtxTestFixture, AllocateSegment_TestSimpleInterfaceFunc)
{
    segmentInfoData->validBlockCount = 0;
    segmentInfoData->occupiedStripeCount = 0;
    segmentInfoData->state = SegmentState::FREE;

    segCtx->AllocateSegment(0);

    EXPECT_EQ(segmentInfoData->state, SegmentState::NVRAM);
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenFreeListIsEmpty)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>;
    int numSegInfos = 100;
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegInfos];
    for(int i=0; i<numSegInfos; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::FREE);
    }
    NiceMock<MockSegmentList> freeSegmentList;
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();

    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segCtx(tp, nullptr, segmentInfoData, nullptr, rebuildCtx, addrInfo, &gcCtx);
    segCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(UNMAP_SEGMENT));
    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete addrInfo;
    delete rebuildCtx;
    delete tp;
}

TEST(SegmentCtx, AllocateFreeSegment_testWhenSegmentIsAllocated)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList> freeSegmentList, nvramSegmentList;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockGcCtx> gcCtx;

    int numSegments = 100;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::FREE);
    }

    SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, &rebuildCtx, &addrInfo, &gcCtx);
    segCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segCtx.SetSegmentList(SegmentState::NVRAM, &nvramSegmentList);

    EXPECT_CALL(freeSegmentList, PopSegment).WillOnce(Return(8));
    EXPECT_CALL(nvramSegmentList, AddToList(8));

    EXPECT_CALL(gcCtx, UpdateCurrentGcMode);

    int ret = segCtx.AllocateFreeSegment();
    EXPECT_EQ(8, ret);
    EXPECT_EQ(segmentInfoData[8].state, SegmentState::NVRAM);
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, victimSegmentList, rebuildSegmentList;

    int numSegments = 4;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(numSegments));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, nullptr, &addrInfo, &gcCtx);
    segCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);

    segmentInfoData[0].validBlockCount = 10;
    segmentInfoData[1].validBlockCount = 6;
    segmentInfoData[2].validBlockCount = 4;
    segmentInfoData[3].validBlockCount = 9;

    EXPECT_CALL(rebuildSegmentList, Contains(2)).WillOnce(Return(false));
    EXPECT_CALL(victimSegmentList, AddToList(2));

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, 2);
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsFoundFromTheRebuildList)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, victimSegmentList, rebuildSegmentList;

    int numSegments = 4;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(numSegments));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, nullptr, &addrInfo, &gcCtx);
    segCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);

    segmentInfoData[0].validBlockCount = 10;
    segmentInfoData[1].validBlockCount = 6;
    segmentInfoData[2].validBlockCount = 4;
    segmentInfoData[3].validBlockCount = 9;

    EXPECT_CALL(rebuildSegmentList, Contains(2)).WillOnce(Return(true));
    EXPECT_CALL(victimSegmentList, AddToList).Times(0);

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, 2);
}

TEST(SegmentCtx, AllocateGCVictimSegment_testWhenVictimSegmentIsNotFound)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList;
    NiceMock<MockGcCtx> gcCtx;

    int numSegments = 4;
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(numSegments));
    EXPECT_CALL(addrInfo, GetblksPerSegment).WillRepeatedly(Return(10));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(10, 0, SegmentState::SSD);
    }

    SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, nullptr, &addrInfo, &gcCtx);
    segCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);

    SegmentId victimSegment = segCtx.AllocateGCVictimSegment();
    EXPECT_EQ(victimSegment, UNMAP_SEGMENT);
}

TEST(SegmentCtx, DISABLED_ResetSegmentState_testIfSegmentStateChangedAsIntended)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockSegmentList> segmentList[SegmentState::NUM_STATES];
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(1));

    NiceMock<MockGcCtx> gcCtx;
    NiceMock<MockTelemetryPublisher> tp;

    {
        SegmentInfoData* segmentInfoData = new SegmentInfoData(100, 10, SegmentState::SSD);
        SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, nullptr, &addrInfo, &gcCtx);
        for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
        {
            segCtx.SetSegmentList((SegmentState)state, &segmentList[state]);
        }

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segmentInfoData->state, SegmentState::SSD);
    }
    {
        SegmentInfoData* segmentInfoData = new SegmentInfoData(100, 10, SegmentState::SSD);
        SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, nullptr, &addrInfo, &gcCtx);
        for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
        {
            segCtx.SetSegmentList((SegmentState)state, &segmentList[state]);
        }

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segmentInfoData->state, SegmentState::SSD);
    }
    {
        SegmentInfoData* segmentInfoData = new SegmentInfoData(0, 10, SegmentState::SSD);
        SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, nullptr, &addrInfo, &gcCtx);
        for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
        {
            segCtx.SetSegmentList((SegmentState)state, &segmentList[state]);
        }

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segmentInfoData->state, SegmentState::NVRAM);
    }
    {
        SegmentInfoData* segmentInfoData = new SegmentInfoData(0, 0, SegmentState::FREE);
        SegmentCtx segCtx(&tp, nullptr, segmentInfoData, nullptr, nullptr, &addrInfo, &gcCtx);
        for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
        {
            segCtx.SetSegmentList((SegmentState)state, &segmentList[state]);
        }

        segCtx.ResetSegmentsStates();
        EXPECT_EQ(segmentInfoData->state, SegmentState::FREE);
    }
}

TEST_F(SegmentCtxTestFixture, GetNumOfFreeSegment_TestSimpleGetter)
{
    EXPECT_CALL(*segmentList[SegmentState::FREE], GetNumSegments).WillOnce(Return(7));
    int ret = segCtx->GetNumOfFreeSegment();
    EXPECT_EQ(7, ret);

    EXPECT_CALL(*segmentList[SegmentState::FREE], GetNumSegmentsWoLock).WillOnce(Return(7));
    ret = segCtx->GetNumOfFreeSegmentWoLock();
    EXPECT_EQ(7, ret);
}

TEST_F(SegmentCtxTestFixture, GetAllocatedSegmentCount_TestSimpleGetter)
{
    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillOnce(Return(100));
    EXPECT_CALL(*segmentList[SegmentState::FREE], GetNumSegments).WillOnce(Return(10));

    // when
    int ret = segCtx->GetAllocatedSegmentCount();
    EXPECT_EQ(90, ret);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetFindFail)
{
    EXPECT_CALL(*rebuildSegmentList, PopSegment).WillOnce(Return(UNMAP_SEGMENT));

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetIsAlreadyFree)
{
    segmentInfoData->validBlockCount = 0;
    segmentInfoData->occupiedStripeCount = 0;
    segmentInfoData->state = SegmentState::FREE;

    EXPECT_CALL(*rebuildSegmentList, PopSegment).WillOnce(Return(0)).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*segmentList[SegmentState::FREE], AddToList(0));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).Times(1);

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, UINT32_MAX);
}

TEST_F(SegmentCtxTestFixture, GetRebuildTargetSegment_TestRebuildTargetFindSuccess)
{
    segmentInfoData->validBlockCount = 10;
    segmentInfoData->occupiedStripeCount = 1024;
    segmentInfoData->state = SegmentState::SSD;

    EXPECT_CALL(*rebuildSegmentList, PopSegment).WillOnce(Return(0));

    SegmentId ret = segCtx->GetRebuildTargetSegment();
    EXPECT_EQ(ret, 0);
}

TEST(SegmentCtx, MakeRebuildTarget_testWhenRebuildTargetListIsEmpty)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> ssdSegmentList, victimSegmentList, nvramSegmentList, rebuildSegmentList;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    int numSegInfos = 4;
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegInfos];
    for(int i=0; i<numSegInfos; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segmentCtx(tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo, &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segmentCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);
    segmentCtx.SetSegmentList(SegmentState::NVRAM, &nvramSegmentList);

    EXPECT_CALL(ssdSegmentList, GetNumSegments).WillRepeatedly(Return(0));
    EXPECT_CALL(ssdSegmentList, PopSegment).WillRepeatedly(Return(UNMAP_SEGMENT));

    EXPECT_CALL(victimSegmentList, GetNumSegments).WillRepeatedly(Return(0));
    EXPECT_CALL(victimSegmentList, PopSegment).WillRepeatedly(Return(UNMAP_SEGMENT));

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));

    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillRepeatedly(Return(0));
    int ret = segmentCtx.MakeRebuildTarget();

    EXPECT_EQ(ret, EID(ALLOCATOR_REBUILD_TARGET_SET_EMPTY));

    delete tp;
}

TEST(SegmentCtx, MakeRebuildTarget_testWhenRebuildTargetListIsNotEmpty)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> ssdSegmentList, victimSegmentList, nvramSegmentList, rebuildSegmentList;
    NiceMock<MockTelemetryPublisher> tp;

    int numSegments = 4;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segmentCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo, &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segmentCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);
    segmentCtx.SetSegmentList(SegmentState::NVRAM, &nvramSegmentList);

    EXPECT_CALL(ssdSegmentList, GetNumSegments).WillOnce(Return(2)).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_CALL(ssdSegmentList, PopSegment).WillOnce(Return(0)).WillOnce(Return(1));

    EXPECT_CALL(victimSegmentList, GetNumSegments).WillOnce(Return(2)).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_CALL(victimSegmentList, PopSegment).WillOnce(Return(2)).WillOnce(Return(3));

    EXPECT_CALL(addrInfo, GetnumUserAreaSegments).WillRepeatedly(Return(4));

    for (int i = 0; i < 4; i++)
    {
        EXPECT_CALL(rebuildSegmentList, AddToList(i)).Times(1);
    }

    EXPECT_CALL(rebuildSegmentList, GetNumSegments).WillRepeatedly(Return(4));
    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).WillOnce(Return(0));

    int ret = segmentCtx.MakeRebuildTarget();

    EXPECT_EQ(ret, 0);
}

TEST(SegmentCtx, SetRebuildCompleted_testIfSegmentIsRemovedFromTheList)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> ssdSegmentList, rebuildSegmentList;

    int numSegments = 4;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    NiceMock<MockGcCtx> gcCtx;
    SegmentCtx segmentCtx(nullptr, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);

    EXPECT_CALL(rebuildSegmentList, PopSegment).WillOnce(Return(2));
    SegmentId target = segmentCtx.GetRebuildTargetSegment();
    EXPECT_EQ(target, 2);

    EXPECT_CALL(rebuildCtx, FlushRebuildSegmentList).WillOnce(Return(0));

    int ret = segmentCtx.SetRebuildCompleted(target);
    EXPECT_EQ(ret, 0);
}

TEST(SegmentCtx, ResetSegmentsState_testIfSegmentStateBecomesNVRAMWhenOccupiedStripeCountIsZeroDuringLogReplay)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> rebuildSegmentList;
    NiceMock<MockTelemetryPublisher> tp;
    NiceMock<MockGcCtx> gcCtx;

    int numSegments = 4;
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(10));

    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for(int i=0; i<numSegments; i++)
    {
        segmentInfoData[i].Set(0, 0, SegmentState::SSD);
    }
    SegmentCtx segmentCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo, &gcCtx);

    NiceMock<MockSegmentList> segmentList[SegmentState::NUM_STATES];
    for (int state = SegmentState::START; state < SegmentState::NUM_STATES; state++)
    {
        segmentCtx.SetSegmentList((SegmentState)state, &segmentList[state]);
    }

    segmentInfoData[0].validBlockCount = 10;
    segmentInfoData[0].occupiedStripeCount = 0;

    segmentInfoData[1].validBlockCount = 10;
    segmentInfoData[1].occupiedStripeCount = 1;

    segmentInfoData[2].validBlockCount = 10;
    segmentInfoData[2].occupiedStripeCount = addrInfo.GetstripesPerSegment();

    segmentInfoData[3].validBlockCount = 0;
    segmentInfoData[3].occupiedStripeCount = 0;

    segmentCtx.ResetSegmentsStates();

    EXPECT_EQ(SegmentState::NVRAM, segmentInfoData[0].state);
    EXPECT_EQ(SegmentState::NVRAM, segmentInfoData[1].state);
    EXPECT_EQ(SegmentState::SSD, segmentInfoData[2].state);
    EXPECT_EQ(SegmentState::FREE, segmentInfoData[3].state);
}

TEST(SegmentCtx, MoveToFreeState_testIfSegmentFreedWhenStateChanged)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, rebuildSegmentList, victimSegmentList;

    uint32_t stripesPerSegment = 1024;
    int numSegments = 1;
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(stripesPerSegment));
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    // Given: a segment is VICTIM state and ready to be freed
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for (int i = 0; i < numSegments; i++)
    {
        segmentInfoData[i].Set(0, stripesPerSegment, SegmentState::VICTIM);
    }

    NiceMock<MockGcCtx> gcCtx;
    NiceMock<MockTelemetryPublisher> tp;
    SegmentCtx segmentCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segmentCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);

    // Then: The segment should be freed
    SegmentId segId = 0;

    EXPECT_CALL(victimSegmentList, RemoveFromList(segId)).WillOnce(Return(true));
    EXPECT_CALL(freeSegmentList, AddToList(segId));

    // When: MoveVictimToFree is called (supposed to be called by GC)
    bool segmentFreed = segmentCtx.MoveToFreeState(segId);
    EXPECT_EQ(segmentFreed, true);
}

TEST(SegmentCtx, MoveToFreeState_testIfSegmentIsNotFreedWhenStateIsNotChanged)
{
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockRebuildCtx> rebuildCtx;
    NiceMock<MockSegmentList> freeSegmentList, ssdSegmentList, rebuildSegmentList, victimSegmentList;

    uint32_t stripesPerSegment = 1024;
    int numSegments = 1;
    ON_CALL(addrInfo, GetstripesPerSegment).WillByDefault(Return(stripesPerSegment));
    ON_CALL(addrInfo, GetnumUserAreaSegments).WillByDefault(Return(numSegments));

    // Given: a segment is SSD state and there's only 1 valid block in this segment
    SegmentInfoData* segmentInfoData = new SegmentInfoData[numSegments];
    for (int i = 0; i < numSegments; i++)
    {
        segmentInfoData[i].Set(1, stripesPerSegment, SegmentState::SSD);
    }

    NiceMock<MockGcCtx> gcCtx;
    NiceMock<MockTelemetryPublisher> tp;
    SegmentCtx segmentCtx(&tp, nullptr, segmentInfoData, &rebuildSegmentList, &rebuildCtx, &addrInfo,
        &gcCtx);
    segmentCtx.SetSegmentList(SegmentState::FREE, &freeSegmentList);
    segmentCtx.SetSegmentList(SegmentState::SSD, &ssdSegmentList);
    segmentCtx.SetSegmentList(SegmentState::VICTIM, &victimSegmentList);

    // Given: GC map udpate sequence call InvalidateBlks with num blocks to invalidate 1, and segment is freed
    bool segmentFreed = segmentCtx.InvalidateBlks(VirtualBlks{.startVsa = {.stripeId = 0, .offset = 0}, .numBlks = 1}, true);
    EXPECT_EQ(segmentFreed, true);

    // Then: The segment should not be freed
    SegmentId segId = 0;

    EXPECT_CALL(victimSegmentList, RemoveFromList(segId)).Times(0);
    EXPECT_CALL(freeSegmentList, AddToList(segId)).Times(0);

    // When: MoveVictimToFree is called (supposed to be called by GC Copier)
    segmentFreed = segmentCtx.MoveToFreeState(segId);
    EXPECT_EQ(segmentFreed, false);
}
} // namespace pos
