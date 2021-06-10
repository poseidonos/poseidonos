#include "src/allocator/wb_stripe_manager/wbstripe_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_spy.h"
#include "test/unit-tests/allocator/wb_stripe_manager/wbstripe_manager_spy.h"
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
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    EXPECT_CALL(*ctxManager, GetWbStripeCtx).Times(1);
    // when
    WBStripeManager wbStripeManager(&addrInfo, ctxManager, blkManager, "");

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
}

TEST(WBStripeManager, Init_TestInitialization)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(2);
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    EXPECT_CALL(*ctxManager, GetWbStripeCtx).Times(1);
    WBStripeManager wbStripeManager(&addrInfo, ctxManager, blkManager, "");
    // when
    wbStripeManager.Init();

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
}

TEST(WBStripeManager, AllocateUserDataStripeId_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    // when
    wbStripeManager.AllocateUserDataStripeId(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FreeWBStripeId_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    std::mutex lock;
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*wbCtx, ReleaseWbStripe).Times(1);
    // when
    wbStripeManager.FreeWBStripeId(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, GetAllActiveStripes_TestVolumeMounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, volManager, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Unmounted));
    std::mutex wbCtxLock;
    //EXPECT_CALL(*wbCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbCtxLock));
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    //EXPECT_CALL(*wbCtx, GetActiveStripeTail).WillOnce(Return(tail));
    //EXPECT_CALL(*wbCtx, SetActiveStripeTail).Times(1);
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = 0};
    //EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa));
    // when
    wbStripeManager.GetAllActiveStripes(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
    delete volManager;
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
}

TEST(WBStripeManager, GetAllActiveStripes_TestVolumeUnmounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, volManager, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Unmounted));
    // when
    wbStripeManager.GetAllActiveStripes(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
    delete volManager;
}

TEST(WBStripeManager, WaitPendingWritesOnStripes_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    //when
    wbStripeManager.WaitPendingWritesOnStripes(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, WaitStripesFlushCompletion_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    //when
    wbStripeManager.WaitStripesFlushCompletion(0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, ReferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
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
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, DereferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = 0};
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
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushAllActiveStripes_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetblksPerStripe(0);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    std::mutex wbCtxLock;
    EXPECT_CALL(*wbCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbCtxLock));
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*wbCtx, GetActiveStripeTail).WillOnce(Return(tail));
    EXPECT_CALL(*wbCtx, SetActiveStripeTail);
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = 0};
    //EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa));
    // when
    wbStripeManager.FlushAllActiveStripes();

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FinishReconstructedStripe_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    // given 1.
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = 0};
    //EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa));
    // when 1.
    wbStripeManager.FinishReconstructedStripe(0, vsa);
    // given 2.
    vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    //EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa));
    // when 2.
    wbStripeManager.FinishReconstructedStripe(0, vsa);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, RestoreActiveStripeTail_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    EXPECT_CALL(*wbCtx, SetActiveStripeTail).Times(1);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    // when
    wbStripeManager.RestoreActiveStripeTail(0, vsa, 0);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
}

TEST(WBStripeManager, FlushPendingActiveStripes_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");

    //StripeSpy* stripe = new StripeSpy();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);

    Stripe* param;
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(1));
    wbStripeManager._ReconstructAS(0, 0, 1, 0, param);
    //EXPECT_CALL(*wbCtx, SetActiveStripeTail).Times(1);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    // when
    wbStripeManager.FlushPendingActiveStripes();

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
    delete stripe;
}

TEST(WBStripeManager, PrepareRebuild_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");

    StripeSpy* stripe = new StripeSpy();
    StripeSpy* stripe2 = new StripeSpy();
    wbStripeManager.PushStripeToStripeArray(stripe);
    wbStripeManager.PushStripeToStripeArray(stripe2);
    // given 1.
    std::mutex lock;
    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, MakeRebuildTarget).WillOnce(Return(NO_REBUILD_TARGET_USER_SEGMENT));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);
    // when 1.
    int ret = wbStripeManager.PrepareRebuild();
    // then 1.
    EXPECT_EQ(NO_REBUILD_TARGET_USER_SEGMENT, ret);
    // given 2.
    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, MakeRebuildTarget).WillOnce(Return(1));
    EXPECT_CALL(*ctxManager, SetNextSsdLsid).WillOnce(Return(-1));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);
    // when 2.
    ret = wbStripeManager.PrepareRebuild();
    // then 2.
    EXPECT_EQ(-1, ret);
    // given 3.
    std::set<SegmentId> targetSegmentList;
    auto begin = targetSegmentList.begin();
    auto end = targetSegmentList.end();
    VirtualBlkAddr tail = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*wbCtx, GetActiveStripeTail).WillOnce(Return(tail));
    EXPECT_CALL(*wbCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*blkManager, TurnOffBlkAllocation).Times(1);
    EXPECT_CALL(*ctxManager, MakeRebuildTarget).WillOnce(Return(1));
    EXPECT_CALL(*ctxManager, SetNextSsdLsid).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetRebuildCtx).WillOnce(Return(reCtx));
    EXPECT_CALL(*reCtx, GetRebuildTargetSegmentsBegin).WillOnce(Return(begin));
    EXPECT_CALL(*reCtx, GetRebuildTargetSegmentsEnd).WillOnce(Return(end));
    EXPECT_CALL(*blkManager, TurnOnBlkAllocation).Times(1);
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = 0};
    //EXPECT_CALL(*iStripeMap, GetLSA).WillOnce(Return(lsa));
    // when 3.
    ret = wbStripeManager.PrepareRebuild();
    // then 3.
    EXPECT_EQ(0, ret);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
    delete reCtx;
    delete stripe;
    delete stripe2;
}

TEST(WBStripeManager, _ReconstructReverseMap_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    WBStripeManagerSpy wbStripeManager(1, reverseMap, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");

    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    // given 1.
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(0));
    EXPECT_CALL(*stripe, LinkReverseMap).WillOnce(Return(-1));
    EXPECT_CALL(*reverseMap, GetReverseMapPack).WillOnce(Return(revMapPack));
    // when 1.
    int ret = wbStripeManager._ReconstructReverseMap(0, stripe, 0);
    // then 1.
    EXPECT_EQ(-1, ret);

    // given 2.
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(0));
    EXPECT_CALL(*stripe, LinkReverseMap).WillOnce(Return(0));
    EXPECT_CALL(*stripe, ReconstructReverseMap).WillOnce(Return(-1));
    EXPECT_CALL(*reverseMap, GetReverseMapPack).WillOnce(Return(revMapPack));
    // when 2.
    ret = wbStripeManager._ReconstructReverseMap(0, stripe, 0);
    // then 2.
    EXPECT_EQ(-1, ret);

    // given 3.
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(0));
    EXPECT_CALL(*stripe, LinkReverseMap).WillOnce(Return(0));
    EXPECT_CALL(*stripe, ReconstructReverseMap).WillOnce(Return(0));
    EXPECT_CALL(*reverseMap, GetReverseMapPack).WillOnce(Return(revMapPack));
    // when 2.
    ret = wbStripeManager._ReconstructReverseMap(0, stripe, 0);
    // then 2.
    EXPECT_EQ(0, ret);

    delete blkManager;
    delete ctxManager;
    delete wbCtx;
    delete iStripeMap;
    delete reCtx;
    delete stripe;
    delete reverseMap;
    delete revMapPack;
}

TEST(WBStripeManager, _ReconstructAS_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    NiceMock<MockWbStripeCtx>* wbCtx = new NiceMock<MockWbStripeCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(1, nullptr, nullptr, iStripeMap, wbCtx, &addrInfo, ctxManager, blkManager, "");

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
    delete wbCtx;
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
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    WBStripeManagerSpy wbStripeManager(1, reverseMap, nullptr, nullptr, nullptr, &addrInfo, nullptr, nullptr, "");
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    // when
    wbStripeManager.ReconstructActiveStripe(0, 0, vsa);

    delete reverseMap;
}

} // namespace pos
