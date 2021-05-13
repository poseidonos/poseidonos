#include "src/allocator/context_manager/context_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/file_io_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/i_allocator_file_io_client_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(ContextManager, ContextManager_)
{
}

TEST(ContextManagmakeer, Init_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");

    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*allocCtx, Init);
    EXPECT_CALL(*wbStripeCtx, Init);
    EXPECT_CALL(*segCtx, Init);
    EXPECT_CALL(*fileMan, Init);
    EXPECT_CALL(*reCtx, Init);

    // when
    ctxManager.Init();
}

TEST(ContextManager, Close_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*allocCtx, Close);
    EXPECT_CALL(*wbStripeCtx, Close);
    EXPECT_CALL(*segCtx, Close);
    EXPECT_CALL(*fileMan, Close);
    EXPECT_CALL(*reCtx, Close);

    // when
    ctxManager.Close();
}

TEST(ContextManager, FlushContextsSync_IfSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(0)).WillOnce(Return(0));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    ASSERT_EQ(0, ret);
}

TEST(ContextManager, FlushContextsSync_IfFailFirstFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    ASSERT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsSync_IfFailSecondFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreSync).WillOnce(Return(0)).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsSync();
    // then
    ASSERT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsAsync_IfSuccessAllFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(0)).WillOnce(Return(0));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    ASSERT_EQ(0, ret);
}

TEST(ContextManager, FlushContextsAsync_IfFailFirstFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    ASSERT_LE(-1, ret);
}

TEST(ContextManager, FlushContextsAsync_IfFailSecondFile)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*fileMan, StoreAsync).WillOnce(Return(0)).WillOnce(Return(-1));
    // when
    int ret = ctxManager.FlushContextsAsync(nullptr);
    // then
    ASSERT_LE(-1, ret);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountSmallerThanMax)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(10));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsNotZero)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(100));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateFree)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateSSD)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, UpdateOccupiedStripeCount_IfOccupiedStripeCountIsMaxAndValidCountIsZeroAndSegStateNVRAM)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    int maxOccupiedCount = (int)addrInfo.GetstripesPerSegment();
    EXPECT_CALL(*segCtx, IncreaseOccupiedStripeCount).WillOnce(Return(100));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::NVRAM));
    // when
    ctxManager.UpdateOccupiedStripeCount(5);
}

TEST(ContextManager, AllocateFreeSegment_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    // given 1.
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(5));
    // when 1.
    ctxManager.AllocateFreeSegment(false);

    // given 2.
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    // when 2.
    ctxManager.AllocateFreeSegment(true);

    // given 3.
    EXPECT_CALL(*allocCtx, AllocateFreeSegment).WillOnce(Return(5)).WillOnce(Return(11));
    EXPECT_CALL(*reCtx, IsRebuildTargetSegment).WillOnce(Return(true)).WillOnce(Return(false));
    // when 3.
    ctxManager.AllocateFreeSegment(true);
}

TEST(ContextManager, AllocateGCVictimSegment_)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(5);
    addrInfo.SetblksPerSegment(20);
    addrInfo.SetnumUserAreaSegments(5);

    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    // given 1.
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(15)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(13)).WillOnce(Return(10));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD));

    // when 1.
    int ret = ctxManager.AllocateGCVictimSegment();
    // then 2.
    ASSERT_EQ(4, ret);

    // given 2.
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(20)).WillOnce(Return(0)).WillOnce(Return(20)).WillOnce(Return(20)).WillOnce(Return(20));
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD)).WillOnce(Return(SegmentState::SSD));

    // when 2.
    ret = ctxManager.AllocateGCVictimSegment();
    // then 2.
    ASSERT_EQ(UNMAP_SEGMENT, ret);
}

TEST(ContextManager, GetNumFreeSegment_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(50));
    // when
    int ret = ctxManager.GetNumFreeSegment();
    // then
    ASSERT_EQ(50, ret);
}

TEST(ContextManager, GetCurrentGcMode_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    ctxManager.GetGcCtx()->SetGcThreshold(10);
    ctxManager.GetGcCtx()->SetUrgentThreshold(5);

    // given 1.
    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(11));
    // when 1.
    CurrentGcMode ret = ctxManager.GetCurrentGcMode();
    // then 1.
    ASSERT_EQ(MODE_NO_GC, ret);

    // given 2.
    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(10));
    // when 2.
    ret = ctxManager.GetCurrentGcMode();
    // then 2.
    ASSERT_EQ(MODE_NORMAL_GC, ret);

    // given 3.
    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(9));
    // when 3.
    ret = ctxManager.GetCurrentGcMode();
    // then 3.
    ASSERT_EQ(MODE_NORMAL_GC, ret);

    // given 4.
    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(5));
    // when 4.
    ret = ctxManager.GetCurrentGcMode();
    // then 4.
    ASSERT_EQ(MODE_URGENT_GC, ret);

    // given 5.
    EXPECT_CALL(*allocCtx, GetNumOfFreeUserDataSegment).WillOnce(Return(4));
    // when 5.
    ret = ctxManager.GetCurrentGcMode();
    // then 5.
    ASSERT_EQ(MODE_URGENT_GC, ret);
}

TEST(ContextManager, GetGcThreshold_)
{
    // given
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, nullptr, "");

    ctxManager.GetGcCtx()->SetGcThreshold(10);
    ctxManager.GetGcCtx()->SetUrgentThreshold(5);

    // when 1.
    int ret = ctxManager.GetGcThreshold(MODE_NORMAL_GC);
    // then 1.
    ASSERT_EQ(10, ret);

    // when 2.
    ret = ctxManager.GetGcThreshold(MODE_URGENT_GC);
    // then 2.
    ASSERT_EQ(5, ret);
}

TEST(ContextManager, FreeUserDataSegment_)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(50);
    addrInfo.SetblksPerSegment(100);
    addrInfo.SetnumUserAreaSegments(20);

    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>(nullptr, "");
    NiceMock<MockWbStripeCtx>* wbStripeCtx = new NiceMock<MockWbStripeCtx>(nullptr);
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>(nullptr, "");
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>("", nullptr);
    NiceMock<MockAllocatorFileIoManager>* fileMan = new NiceMock<MockAllocatorFileIoManager>(nullptr, "");
    ContextManager ctxManager(allocCtx, segCtx, reCtx, wbStripeCtx, fileMan, nullptr, &addrInfo, "");

    // given 1.
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::SSD));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).WillOnce(Return(50));
    // when 1.
    ctxManager.FreeUserDataSegment(5);

    // given 2.
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::VICTIM));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).WillOnce(Return(50));
    // when 2.
    ctxManager.FreeUserDataSegment(5);

    // given 3.
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::NVRAM));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).Times(0);
    // when 3.
    ctxManager.FreeUserDataSegment(5);

    // given 4.
    EXPECT_CALL(*allocCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*segCtx, GetOccupiedStripeCount).Times(0);
    // when 4.
    ctxManager.FreeUserDataSegment(5);
}

} // namespace pos
