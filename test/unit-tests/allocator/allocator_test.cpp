#include "src/allocator/allocator.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/sys_event/volume_event.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/wbstripe_manager/wbstripe_manager_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/meta_file_intf/meta_file_intf_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(Allocator, Allocator_TestConstructor)
{
    // given
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    Allocator alloc(iArrayInfo, iState);
    delete iArrayInfo;
    delete iState;
}

TEST(Allocator, Init_TestInitializeOrNot)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when 1.
    EXPECT_CALL(*addrInfo, Init);
    EXPECT_CALL(*ctxManager, Init);
    EXPECT_CALL(*blkManager, Init);
    EXPECT_CALL(*wbManager, Init);
    alloc.Init();

    // when 2.
    EXPECT_CALL(*addrInfo, Init).Times(0);
    EXPECT_CALL(*ctxManager, Init).Times(0);
    EXPECT_CALL(*blkManager, Init).Times(0);
    EXPECT_CALL(*wbManager, Init).Times(0);
    alloc.Init();
}

TEST(Allocator, Dispose_TestDisposeAfterInitOrNot)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    alloc.Init();
    // given 1.
    EXPECT_CALL(*wbManager, FlushAllActiveStripes);
    EXPECT_CALL(*ctxManager, FlushContexts);
    EXPECT_CALL(*ctxManager, Dispose);
    // when 1.
    alloc.Dispose();

    // given 2.
    EXPECT_CALL(*wbManager, FlushAllActiveStripes).Times(0);
    EXPECT_CALL(*ctxManager, FlushContexts).Times(0);
    // when 2.
    alloc.Dispose();
}

TEST(Allocator, Shutdown_TestShutdownWithInitializeOrNot)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    alloc.Init();
    // given 1. - Allocator initialized
    EXPECT_CALL(*ctxManager, Dispose).Times(1);
    EXPECT_CALL(*wbManager, FlushAllActiveStripes);
    EXPECT_CALL(*wbManager, Dispose);
    // when 1.
    alloc.Shutdown();

    // given 2. - Allocator NOT initialized
    EXPECT_CALL(*ctxManager, Dispose).Times(0);
    // when 2.
    alloc.Shutdown();

    // When 3: Call No-Op methods
    alloc.Flush();
}

TEST(Allocator, SetNormalGcThreshold_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    GcCtx* gc = new GcCtx();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*ctxManager, GetGcCtx).WillOnce(Return(gc));
    // when
    alloc.SetNormalGcThreshold(10);
    delete gc;
}

TEST(Allocator, SetUrgentThreshold_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    GcCtx* gc = new GcCtx();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*ctxManager, GetGcCtx).WillOnce(Return(gc));
    // when
    alloc.SetUrgentThreshold(20);
    delete gc;
}

TEST(Allocator, GetMeta_TestWBTFunctionsWithType)
{
    // given
    AllocatorAddressInfo* addrInfo = new AllocatorAddressInfo();
    addrInfo->SetnumUserAreaSegments(10);
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>(nullptr, nullptr, segCtx, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");

    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // given 1. fail to create file
    EXPECT_CALL(*file, Create).WillOnce(Return(-1));
    // when 1.
    int ret = alloc.GetMeta(WBT_SEGMENT_VALID_COUNT, "", file);
    // then 1.
    EXPECT_EQ((int)-EID(ALLOCATOR_START), ret);

    // given 2. fail to Write file
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillOnce(Return(segCtx));
    EXPECT_CALL(*segCtx, CopySegmentInfoToBufferforWBT);
    EXPECT_CALL(*file, Create).WillOnce(Return(0));
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, IssueIO).WillOnce(Return(-1));
    EXPECT_CALL(*file, Close);
    // when 2.
    ret = alloc.GetMeta(WBT_SEGMENT_VALID_COUNT, "", file);
    // then 2.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_STORE), ret);

    // given 3. success to Write file
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillOnce(Return(segCtx));
    EXPECT_CALL(*segCtx, CopySegmentInfoToBufferforWBT);
    EXPECT_CALL(*file, Create).WillOnce(Return(0));
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, IssueIO).WillOnce(Return(0));
    EXPECT_CALL(*file, Close);
    // when 3.
    ret = alloc.GetMeta(WBT_SEGMENT_OCCUPIED_STRIPE, "", file);
    // then 3.
    EXPECT_EQ(0, ret);

    // given 4. wrong WBT Type
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Create).WillOnce(Return(0));
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, Close);
    // when 4.
    ret = alloc.GetMeta(WBT_NUM_ALLOCATOR_META, "", file);
    // then 4.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_STORE), ret);

    // given 5. failed to appendIo
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Create).WillOnce(Return(0));
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(-1));
    EXPECT_CALL(*file, Close);
    // when 5.
    ret = alloc.GetMeta(WBT_CURRENT_SSD_LSID, "", file);
    // then 5.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_STORE), ret);

    // given 6. success to appendIo
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Create).WillOnce(Return(0));
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(0));
    EXPECT_CALL(*file, Close);
    // when 6.
    ret = alloc.GetMeta(WBT_CURRENT_SSD_LSID, "", file);
    // then 6.
    EXPECT_EQ(0, ret);
}

TEST(Allocator, SetMeta_TestWBTFunctionsWithType)
{
    // given
    AllocatorAddressInfo* addrInfo = new AllocatorAddressInfo();
    addrInfo->SetnumUserAreaSegments(10);
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>(nullptr, allocCtx, segCtx, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    NiceMock<MockMetaFileIntf>* file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // given 1. fail to appendIo file
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(-1));
    EXPECT_CALL(*file, Close);
    // when 1.
    int ret = alloc.SetMeta(WBT_SEGMENT_VALID_COUNT, "", file);
    // then 1.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_LOAD), ret);

    // given 2. success to appendIo file
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(0));
    EXPECT_CALL(*file, Close);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    // when 2.
    ret = alloc.SetMeta(WBT_SEGMENT_OCCUPIED_STRIPE, "", file);
    // then 2.
    EXPECT_EQ(0, ret);

    // given 3. failed to append Io
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(-1));
    EXPECT_CALL(*file, Close);
    EXPECT_CALL(*allocCtx, SetAllocatedWbStripeCount).Times(0);
    // when 3.
    ret = alloc.SetMeta(WBT_WBLSID_BITMAP, "", file);
    // then 3.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_LOAD), ret);

    // given 4. success to append Io
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(0));
    EXPECT_CALL(*file, Close);
    EXPECT_CALL(*ctxManager, GetAllocatorCtx).WillOnce(Return(allocCtx));
    EXPECT_CALL(*allocCtx, SetAllocatedWbStripeCount);
    // when 4.
    ret = alloc.SetMeta(WBT_WBLSID_BITMAP, "", file);
    // then 4.
    EXPECT_EQ(0, ret);

    // given 5. failed to appendIo
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(-1));
    EXPECT_CALL(*file, Close);
    // when 5.
    ret = alloc.SetMeta(WBT_ACTIVE_STRIPE_TAIL, "", file);
    // then 5.
    EXPECT_EQ((int)-EID(ALLOCATOR_META_ARCHIVE_LOAD), ret);

    // given 6. success to appendIo
    file = new NiceMock<MockMetaFileIntf>("aa", "bb");
    EXPECT_CALL(*file, Open);
    EXPECT_CALL(*file, AppendIO).WillOnce(Return(0));
    EXPECT_CALL(*file, Close);
    // when 6.
    ret = alloc.SetMeta(WBT_ACTIVE_STRIPE_TAIL, "", file);
    // then 6.
    EXPECT_EQ(0, ret);
}

TEST(Allocator, GetBitmapLayout_TestSimplePrinter)
{
    // given
    AllocatorAddressInfo* addrInfo = new AllocatorAddressInfo();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when
    alloc.GetBitmapLayout("aaa");
}

TEST(Allocator, GetInstantMetaInfo_TestSimplePrinter)
{
    // given
    AllocatorAddressInfo* addrInfo = new AllocatorAddressInfo();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockRebuildCtx>* rebuildCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>(nullptr, allocCtx, segCtx, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*ctxManager, GetAllocatorCtx).WillOnce(Return(allocCtx));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillOnce(Return(segCtx));
    addrInfo->SetnumUserAreaSegments(6);
    EXPECT_CALL(*segCtx, GetSegmentState).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::FREE)).WillOnce(Return(SegmentState::FREE));
    EXPECT_CALL(*segCtx, GetValidBlockCount).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));
    // when
    alloc.GetInstantMetaInfo("aaa");
}

TEST(Allocator, FlushAllUserdataWBT_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo* addrInfo = new AllocatorAddressInfo();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*blkManager, TurnOffBlkAllocation);
    EXPECT_CALL(*wbManager, CheckAllActiveStripes);
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation);
    EXPECT_CALL(*wbManager, FinalizeWriteIO);

    // when
    alloc.FlushAllUserdataWBT();
}

TEST(Allocator, GetIBlockAllocator_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when
    IBlockAllocator* ret = alloc.GetIBlockAllocator();
    // then
    EXPECT_EQ(blkManager, ret);
}

TEST(Allocator, GetIWBStripeAllocator_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when
    IWBStripeAllocator* ret = alloc.GetIWBStripeAllocator();
    // then
    EXPECT_EQ(wbManager, ret);
}

TEST(Allocator, GetIContextReplayer_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*ctxManager, GetContextReplayer);
    // when
    IContextReplayer* ret = alloc.GetIContextReplayer();
}

TEST(Allocator, GetIContextManager_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when
    IContextManager* ret = alloc.GetIContextManager();
    // then
    EXPECT_EQ(ctxManager, ret);
}

TEST(Allocator, GetIAllocatorWbt_TestSimpleGetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    // when
    IAllocatorWbt* ret = alloc.GetIAllocatorWbt();
}

TEST(Allocator, PreppareRebuild_testSuccessfulPath)
{
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, GetNvramSegmentList)
        .WillOnce([]()
        {
            std::set<SegmentId> segmentList = {0, 1, 2};
            return segmentList;
        });
    EXPECT_CALL(*wbManager, FlushOnlineStripesInSegment).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, MakeRebuildTargetSegmentList).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, SetNextSsdLsid).WillOnce(Return(0));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);

    int ret = alloc.PrepareRebuild();
    EXPECT_EQ(ret, 0);
}

TEST(Allocator, PreppareRebuild_testIfPrepareStoppedWhenTheresNoTargetSegmentsToRebuild)
{
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, GetNvramSegmentList)
        .WillOnce([]()
        {
            std::set<SegmentId> segmentList = {0, 1, 2};
            return segmentList;
        });
    EXPECT_CALL(*wbManager, FlushOnlineStripesInSegment).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, MakeRebuildTargetSegmentList).WillOnce(Return((int)POS_EVENT_ID::ALLOCATOR_REBUILD_TARGET_SET_EMPTY));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);

    int ret = alloc.PrepareRebuild();
    EXPECT_EQ(ret, 0);
}

TEST(Allocator, PreppareRebuild_testWhenFlushOnlineStripeFails)
{
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, GetNvramSegmentList);
    EXPECT_CALL(*wbManager, FlushOnlineStripesInSegment).WillOnce(Return(-1));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);

    int ret = alloc.PrepareRebuild();
    EXPECT_EQ(ret, -1);
}

TEST(Allocator, PreppareRebuild_testWhenSetNextSsdLsidFails)
{
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIArrayInfo>* iArrayInfo = new NiceMock<MockIArrayInfo>();
    NiceMock<MockIStateControl>* iState = new NiceMock<MockIStateControl>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockWBStripeManager>* wbManager = new NiceMock<MockWBStripeManager>();
    Allocator alloc(nullptr, addrInfo, ctxManager, blkManager, wbManager, iArrayInfo, iState);

    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, GetNvramSegmentList)
        .WillOnce([]()
        {
            std::set<SegmentId> segmentList = {0, 1, 2};
            return segmentList;
        });
    EXPECT_CALL(*ctxManager, MakeRebuildTargetSegmentList).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, SetNextSsdLsid).WillOnce(Return(-1));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);

    int ret = alloc.PrepareRebuild();
    EXPECT_EQ(ret, -1);
}

} // namespace pos
