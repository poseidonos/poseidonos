#include "src/allocator/wbstripe_manager/wbstripe_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_spy.h"
#include "test/unit-tests/allocator/wbstripe_manager/wbstripe_manager_spy.h"
#include "test/unit-tests/bio/flush_io_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(WBStripeManager, WBStripeManager_TestConstructor)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    // when
    WBStripeManager wbStripeManager(nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, Init_TestInitialization)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(2);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    // when
    wbStripeManager.Init();

    // When 2
    std::shared_ptr<MockFlushIo> flushIo = std::make_shared<MockFlushIo>(0);
    EXPECT_CALL(*flushIo, GetVolumeId).WillOnce(Return(0));
    wbStripeManager.GetWbStripes(flushIo);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, FreeWBStripeId_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);

    EXPECT_CALL(*allocCtx, ReleaseWbStripe).Times(1);
    // when
    wbStripeManager.FreeWBStripeId(0);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushActiveStripes_TestVolumeMounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, volManager, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    // given 1.
    std::mutex lock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(UNMAP_VSA));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail);
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Mounted));
    // when
    wbStripeManager.FlushActiveStripes(77);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
    delete volManager;
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
}

TEST(WBStripeManager, FlushActiveStripes_TestVolumeUnmounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, volManager, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Unmounted));
    // when
    wbStripeManager.FlushActiveStripes(0);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
    delete volManager;
}

TEST(WBStripeManager, FinalizeWriteIO_TestAddFlushStripe)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetnumWbStripes(3);
    addrInfo.SetchunksPerStripe(3);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    wbStripeManager.Init();
    StripeVec stripeVec;
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    stripeVec.push_back(stripe);
    std::vector<StripeId> stripeIdVec;
    StripeId stripeId = 0;
    stripeIdVec.push_back(stripeId);
    StripeAddr wblsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    StripeAddr userlsa = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 0};
    EXPECT_CALL(*stripe, GetBlksRemaining).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(wblsa)).WillOnce(Return(userlsa));
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(false)).WillOnce(Return(true));
    // when
    wbStripeManager.FinalizeWriteIO(stripeVec, stripeIdVec);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, FinalizeActiveStripes_TestSimpleCall)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetnumWbStripes(3);
    addrInfo.SetchunksPerStripe(3);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    wbStripeManager.Init();

    std::mutex lock, allocLock;
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(allocLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(UNMAP_VSA));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail);

    bool ret = wbStripeManager.FinalizeActiveStripes(0);
    EXPECT_EQ(ret, true);

    delete allocCtx;
    delete ctxManager;
    delete iStripeMap;
    delete blkManager;
}

TEST(WBStripeManager, ReferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(1);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_USER_AREA,
        .stripeId = 0,
    };
    wbStripeManager.Init();
    // given 1.
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(true));
    // when 1.
    bool ret = wbStripeManager.ReferLsidCnt(lsa);
    // then 1.
    EXPECT_EQ(false, ret);
    // given 2.
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(false));
    // when 2.
    ret = wbStripeManager.ReferLsidCnt(lsa);
    // then 2.
    EXPECT_EQ(true, ret);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
}

TEST(WBStripeManager, DereferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(2);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_USER_AREA,
        .stripeId = 0};
    wbStripeManager.Init();
    // given 1.
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(true));
    // when 1.
    wbStripeManager.DereferLsidCnt(lsa, 0);
    // given 2.
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(false));
    // when 2.
    wbStripeManager.DereferLsidCnt(lsa, 0);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushAllActiveStripes_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetblksPerStripe(0);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    std::mutex wbCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbCtxLock));
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(tail));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail);
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    // when
    wbStripeManager.FlushAllActiveStripes();

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushAllActiveStripes_TestwithOtherCondition)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetblksPerStripe(0);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    NiceMock<MockStripe>* stripe2 = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    std::mutex wbCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbCtxLock));
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(tail));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail);
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    EXPECT_CALL(*stripe, GetBlksRemaining).WillOnce(Return(1)).WillOnce(Return(0));
    EXPECT_CALL(*stripe, IsFinished).WillOnce(Return(false)).WillOnce(Return(true));
    // when
    wbStripeManager.FlushAllActiveStripes();

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, FinishReconstructedStripe_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    // given 1.
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    // when 1.
    wbStripeManager.FinishReconstructedStripe(0, vsa);
    // given 2.
    vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    // when 2.
    wbStripeManager.FinishReconstructedStripe(0, vsa);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushPendingActiveStripes_TestwithoutStripeVec)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    Stripe* param;
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(1));
    wbStripeManager._ReconstructAS(0, 0, 1, 0, param);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    // when
    wbStripeManager.FlushPendingActiveStripes();

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, FlushPendingActiveStripes_TestwithStripeVecwithFailStripeFlush)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockStripe>* stripetoFlush = new NiceMock<MockStripe>();
    StripeVec* stripeVec = new StripeVec();
    stripeVec->push_back(stripetoFlush);
    WBStripeManagerSpy wbStripeManager(nullptr, stripeVec, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    // given
    EXPECT_CALL(*stripetoFlush, GetWbLsid).WillOnce(Return(77));
    // when
    int ret = wbStripeManager.FlushPendingActiveStripes();
    // then
    EXPECT_EQ(-1, ret);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripetoFlush;
}

TEST(WBStripeManager, FlushPendingActiveStripes_TestwithStripeVecwithSuccessStripeFlush)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockStripe>* stripetoFlush = new NiceMock<MockStripe>();
    StripeVec* stripeVec = new StripeVec();
    stripeVec->push_back(stripetoFlush);
    WBStripeManagerSpy wbStripeManager(nullptr, stripeVec, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    // given
    EXPECT_CALL(*stripetoFlush, GetWbLsid).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_CALL(*stripetoFlush, GetVsid).WillOnce(Return(0));
    EXPECT_CALL(*stripetoFlush, GetBlksRemaining).WillOnce(Return(0));
    // when
    int ret = wbStripeManager.FlushPendingActiveStripes();
    // then
    EXPECT_EQ(0, ret);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripetoFlush;
}

TEST(WBStripeManager, FlushOnlineStripesInSegment_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);

    StripeSpy* stripe = new StripeSpy();
    StripeSpy* stripe2 = new StripeSpy();
    wbStripeManager.PushStripeToStripeArray(stripe);
    wbStripeManager.PushStripeToStripeArray(stripe2);
    // given 1.
    std::mutex lock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(UNMAP_VSA));
    // when 1.
    std::set<SegmentId> targetSegmentList;
    int ret = wbStripeManager.FlushOnlineStripesInSegment(targetSegmentList);
    // then 1.
    EXPECT_EQ(0, ret);

    // given 2.
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(tail));
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*iStripeMap, IsInUserDataArea).WillOnce(Return(false)).WillOnce(Return(true));
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    // when 3.
    ret = wbStripeManager.FlushOnlineStripesInSegment(targetSegmentList);
    // then 3.
    EXPECT_EQ(0, ret);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete iStripeMap;
    delete reCtx;
    delete stripe;
    delete stripe2;
}

TEST(WBStripeManager, _ReconstructAS_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    Stripe* param;
    // when 1.
    int ret = wbStripeManager._ReconstructAS(0, 0, 0, 0, param);

    // given 2.
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(0));
    // when 2.
    ret = wbStripeManager._ReconstructAS(0, 0, 1, 0, param);

    // given 3.
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(1));
    // when 2.
    ret = wbStripeManager._ReconstructAS(0, 0, 1, 0, param);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete reCtx;
    delete stripe;
}

TEST(WBStripeManager, ReconstructActiveStripe_TestFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockIVolumeManager>* vol = new NiceMock<MockIVolumeManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, reverseMap, vol, nullptr, nullptr, &addrInfo, nullptr, nullptr, "", 0);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    // when
    std::map<uint64_t, BlkAddr> revMapInfos;
    wbStripeManager.ReconstructActiveStripe(0, 0, vsa, revMapInfos);
    delete stripe;
    delete reverseMap;
    delete vol;
}

TEST(WBStripeManager, _AllocateRemainingBlocks_TestFunc)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, reverseMap, nullptr, nullptr, nullptr, addrInfo, nullptr, nullptr, "", 0);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 100};
    EXPECT_CALL(*addrInfo, GetblksPerStripe).WillOnce(Return(1)).WillOnce(Return(1)).WillOnce(Return(1));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    // when
    wbStripeManager._AllocateRemainingBlocks(vsa);
    delete addrInfo;
    delete reverseMap;
}

TEST(WBStripeManager, _GetOnlineStripes_TestFuncSuccess)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(1);

    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    std::set<SegmentId> segmentIds = {0, 1};
    StripeAddr lsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    StripeAddr lsa2 = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 1};
    EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa)).WillOnce(Return(lsa2));
    EXPECT_CALL(*iStripeMap, IsInWriteBufferArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_WRITE_BUFFER_AREA;
        });
    EXPECT_CALL(*iStripeMap, IsInUserDataArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_USER_AREA;
        });

    EXPECT_CALL(*stripe, IsFinished).WillOnce(Return(false));
    EXPECT_CALL(*stripe, GetBlksRemaining).WillOnce(Return(0));
    EXPECT_CALL(*stripe, GetVsid).WillOnce(Return(10));

    // when
    std::vector<StripeId> stripeIds;
    wbStripeManager._GetOnlineStripes(segmentIds, stripeIds);
    std::vector<StripeId> expected = {10};
    EXPECT_EQ(stripeIds, expected);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, _GetOnlineStripes_TestFuncSuccess2)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(1);

    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe1 = new NiceMock<MockStripe>();
    NiceMock<MockStripe>* stripe2 = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe1);
    wbStripeManager.PushStripeToStripeArray(stripe2);

    std::set<SegmentId> segmentIds = {0, 1};
    StripeAddr lsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    StripeAddr lsa2 = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 1};
    EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa)).WillOnce(Return(lsa2));
    EXPECT_CALL(*iStripeMap, IsInWriteBufferArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_WRITE_BUFFER_AREA;
        });
    EXPECT_CALL(*iStripeMap, IsInUserDataArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_USER_AREA;
        });

    EXPECT_CALL(*stripe1, GetBlksRemaining).WillOnce(Return(0));
    EXPECT_CALL(*stripe1, IsFinished).WillOnce(Return(false));
    EXPECT_CALL(*stripe1, GetVsid).WillOnce(Return(10));

    EXPECT_CALL(*stripe2, IsFinished).WillOnce(Return(false));
    EXPECT_CALL(*stripe2, GetBlksRemaining).WillOnce(Return(0));
    EXPECT_CALL(*stripe2, GetVsid).WillOnce(Return(20));

    // when
    std::vector<StripeId> stripeIds;
    int ret = wbStripeManager._GetOnlineStripes(segmentIds, stripeIds);
    std::vector<StripeId> expected = {10, 20};
    EXPECT_EQ(stripeIds, expected);
    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe1;
    delete stripe2;
}

TEST(WBStripeManager, _GetOnlineStripes_TestFuncSuccess3)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    addrInfo.SetstripesPerSegment(1);

    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    std::set<SegmentId> segmentIds = {0, 1};

    StripeAddr lsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    StripeAddr lsa2 = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 1};

    EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa)).WillOnce(Return(lsa2));
    EXPECT_CALL(*iStripeMap, IsInWriteBufferArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_WRITE_BUFFER_AREA;
        });
    EXPECT_CALL(*iStripeMap, IsInUserDataArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_USER_AREA;
        });

    EXPECT_CALL(*stripe, IsFinished).WillOnce(Return(false));
    EXPECT_CALL(*stripe, GetBlksRemaining).WillOnce(Return(10));

    // when
    std::vector<StripeId> stripeIds;
    wbStripeManager._GetOnlineStripes(segmentIds, stripeIds);

    EXPECT_EQ(stripeIds.empty(), true);
    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, _GetOnlineStripes_TestFuncSuccess4)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(1);

    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    std::set<SegmentId> segmentIds = {0, 1};

    StripeAddr lsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    StripeAddr lsa2 = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 1};

    EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa)).WillOnce(Return(lsa2));
    EXPECT_CALL(*iStripeMap, IsInWriteBufferArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_WRITE_BUFFER_AREA;
        });
    EXPECT_CALL(*iStripeMap, IsInUserDataArea)
        .WillRepeatedly([&](StripeAddr entry)
        {
            return entry.stripeLoc == IN_USER_AREA;
        });

    EXPECT_CALL(*stripe, IsFinished).WillOnce(Return(true));
    // when
    std::vector<StripeId> stripeIds;
    wbStripeManager._GetOnlineStripes(segmentIds, stripeIds);

    EXPECT_EQ(stripeIds.empty(), true);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, _GetOnlineStripes_TestFuncSuccess5)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(1);

    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, nullptr, 1, nullptr, nullptr, iStripeMap, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    std::set<SegmentId> segmentIds = {0};

    StripeAddr lsa = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = UNMAP_STRIPE};

    // when
    std::vector<StripeId> stripeIds;
    wbStripeManager._GetOnlineStripes(segmentIds, stripeIds);
    EXPECT_EQ(stripeIds.empty(), true);

    delete blkManager;
    delete ctxManager;
    delete iStripeMap;
    delete stripe;
}

} // namespace pos
