#include "src/allocator/wbstripe_manager/wbstripe_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_spy.h"
#include "test/unit-tests/allocator/wbstripe_manager/wbstripe_manager_spy.h"
#include "test/unit-tests/bio/flush_io_mock.h"
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

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, FreeWBStripeId_TestSimpleCaller)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);

    EXPECT_CALL(*allocCtx, ReleaseWbStripe).Times(1);
    // when
    wbStripeManager.FreeWBStripeId(0);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
}

TEST(WBStripeManager, FlushAllPendingStripesInVolume_TestVolumeMounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();

    uint32_t volumeId = 3;
    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, volManager, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetVolumeId).WillOnce(Return(volumeId));
        EXPECT_CALL(*stripe, UpdateFlushIo).Times(1);
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    // given 1.
    StripeId wbLsid = 4;
    std::mutex lock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(lock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(UNMAP_VSA));
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Mounted));

    // when
    std::shared_ptr<MockFlushIo> flushIo = std::make_shared<MockFlushIo>(0);
    wbStripeManager.FlushAllPendingStripesInVolume(volumeId, flushIo);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete volManager;
}

TEST(WBStripeManager, FlushAllPendingStripesInVolume_TestVolumeUnmounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeManager>* volManager = new NiceMock<MockIVolumeManager>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, volManager, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
    EXPECT_CALL(*volManager, GetVolumeStatus).WillOnce(Return(Unmounted));
    // when
    wbStripeManager.FlushAllPendingStripesInVolume(0, nullptr);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
    delete volManager;
}

TEST(WBStripeManager, FlushAllPendingStripesInVolume_testIfWaitsForStripesWithTheVolumeId)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetnumWbStripes(3);
    addrInfo.SetchunksPerStripe(3);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);

    std::mutex allocLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(allocLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(UNMAP_VSA));

    int volumeId = 5;

    NiceMock<MockStripe>* volumeStripe = new NiceMock<MockStripe>();
    EXPECT_CALL(*volumeStripe, GetVolumeId).WillOnce(Return(volumeId));
    EXPECT_CALL(*volumeStripe, GetBlksRemaining).WillOnce(Return(0));
    EXPECT_CALL(*volumeStripe, IsFinished).WillOnce(Return(true));
    wbStripeManager.PushStripeToStripeArray(volumeStripe);

    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    EXPECT_CALL(*stripe, GetVolumeId).WillOnce(Return(10));
    wbStripeManager.PushStripeToStripeArray(stripe);

    int ret = wbStripeManager.FlushAllPendingStripesInVolume(volumeId);
    EXPECT_EQ(ret, 0);

    delete allocCtx;
    delete ctxManager;
    delete blkManager;
}

TEST(WBStripeManager, ReferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(1);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    wbStripeManager.Init();

    // given 1.
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_USER_AREA,
        .stripeId = 0,
    };
    // when 1.
    bool ret = wbStripeManager.ReferLsidCnt(lsa);
    // then 1.
    EXPECT_EQ(false, ret);
    // given 2.
    lsa.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA;
    // when 2.
    ret = wbStripeManager.ReferLsidCnt(lsa);
    // then 2.
    EXPECT_EQ(true, ret);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, DereferLsidCnt_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(2);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);
    wbStripeManager.Init();

    // given 1.
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_USER_AREA,
        .stripeId = 0};
    // when 1.
    wbStripeManager.DereferLsidCnt(lsa, 0);
    // given 2.
    lsa.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA;
    // when 2.
    wbStripeManager.DereferLsidCnt(lsa, 0);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, FlushAllWbStripes_testIfWaitsForAllStripeFlush)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    addrInfo.SetchunksPerStripe(2);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 6, nullptr, nullptr, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);

    std::mutex lock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillRepeatedly(Return(UNMAP_VSA));

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetBlksRemaining).WillRepeatedly(Return(0));
        EXPECT_CALL(*stripe, IsFinished).WillRepeatedly(Return(true));

        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    wbStripeManager.FlushAllWbStripes();

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
}

TEST(WBStripeManager, FinishStripe_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, allocCtx, &addrInfo, ctxManager, blkManager, "", 0);
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
    wbStripeManager.FinishStripe(0, vsa);
    // given 2.
    vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    // when 2.
    wbStripeManager.FinishStripe(0, vsa);

    delete blkManager;
    delete ctxManager;
    delete allocCtx;
}

TEST(WBStripeManager, FlushAllPendingStripes_testIfAllWbStripeIsFlushed)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetBlksRemaining).WillRepeatedly(Return(0));
        EXPECT_CALL(*stripe, IsFinished).WillRepeatedly(Return(false));
        EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));

        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    // when
    int ret = wbStripeManager.FlushAllPendingStripes();

    // Then
    EXPECT_EQ(ret, 0);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, FlushAllPendingStripes_testIfOnlyNotFinishedStripesAreFlushed)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    for (int i = 0; i < 2; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetBlksRemaining).WillRepeatedly(Return(0));
        EXPECT_CALL(*stripe, IsFinished).WillRepeatedly(Return(true));
        EXPECT_CALL(*stripe, Flush).Times(0);

        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    for (int i = 2; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetBlksRemaining).WillRepeatedly(Return(0));
        EXPECT_CALL(*stripe, IsFinished).WillRepeatedly(Return(false));
        EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));

        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    // when
    int ret = wbStripeManager.FlushAllPendingStripes();

    // Then
    EXPECT_EQ(ret, 0);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, FlushAllPendingStripes_testIfAllStripeIsFlushedThoughOneOfThemFails)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();

    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetBlksRemaining).WillRepeatedly(Return(0));
        EXPECT_CALL(*stripe, IsFinished).WillRepeatedly(Return(false));

        if (i == 3)
        {
            EXPECT_CALL(*stripe, Flush).WillOnce(Return(-1));
        }
        else
        {
            EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));
        }

        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    // when
    int ret = wbStripeManager.FlushAllPendingStripes();
    // then
    EXPECT_EQ(-1, ret);

    delete blkManager;
    delete ctxManager;
}

TEST(WBStripeManager, _ReconstructAS_TestwithAllConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockRebuildCtx>* reCtx = new NiceMock<MockRebuildCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, ctxManager, blkManager, "", 0);

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
    delete reCtx;
}

TEST(WBStripeManager, ReconstructActiveStripe_TestFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(1);
    addrInfo.SetblksPerStripe(1);
    NiceMock<MockIVolumeManager>* vol = new NiceMock<MockIVolumeManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    WBStripeManagerSpy wbStripeManager(nullptr, 1, reverseMap, vol, nullptr, &addrInfo, nullptr, nullptr, "", 0);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    wbStripeManager.PushStripeToStripeArray(stripe);
    // when
    std::map<uint64_t, BlkAddr> revMapInfos;
    wbStripeManager.ReconstructActiveStripe(0, 0, vsa, revMapInfos);
    delete reverseMap;
    delete vol;
}

TEST(WBStripeManager, _GetRemainingBlocks_TestFunc)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    ON_CALL(addrInfo, GetblksPerStripe).WillByDefault(Return(128));

    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, nullptr, &addrInfo, nullptr, nullptr, "", 0);

    VirtualBlkAddr currentTail = UNMAP_VSA;
    VirtualBlks actual = wbStripeManager._GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);

    currentTail.stripeId = 10;
    currentTail.offset = 2000;
    actual = wbStripeManager._GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);

    currentTail.stripeId = 20;
    currentTail.offset = 50;
    actual = wbStripeManager._GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa.stripeId, 20);
    EXPECT_EQ(actual.startVsa.offset, 50);
    EXPECT_EQ(actual.numBlks, 128 - 50);

    currentTail.stripeId = 30;
    currentTail.offset = 128;
    actual = wbStripeManager._GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);
}

TEST(WBStripeManager, _FillBlocksToStripe_testIfStripeIsFilled)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockBlockManager> blkManager;
    // when
    WBStripeManagerSpy wbStripeManager(nullptr, &addrInfo, &ctxManager, &blkManager, "", 0);

    NiceMock<MockStripe> stripe;
    StripeId wbLsid = 100;
    BlkOffset startOffset = 30;
    uint32_t numBlks = 5;

    for (uint32_t offset = startOffset; offset < startOffset + numBlks; offset++)
    {
        EXPECT_CALL(stripe, UpdateReverseMapEntry(offset, INVALID_RBA, UINT32_MAX));
    }
    EXPECT_CALL(stripe, DecreseBlksRemaining(numBlks)).WillOnce(Return(0));
    bool flushRequired = wbStripeManager._FillBlocksToStripe(&stripe, wbLsid, startOffset, numBlks);
    EXPECT_EQ(flushRequired, true);
}

TEST(WBStripeManager, _FinishActiveStripe_testIfReturnsNullWhenNoActiveStripeExistForTheVolume)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockAllocatorCtx> allocCtx;
    NiceMock<MockBlockManager> blkManager;
    ON_CALL(ctxManager, GetAllocatorCtx).WillByDefault(Return(&allocCtx));

    WBStripeManagerSpy wbStripeManager(nullptr, &addrInfo, &ctxManager, &blkManager, "", 0);

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    ASTailArrayIdx index = 3;

    std::mutex lock;
    EXPECT_CALL(allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    EXPECT_CALL(allocCtx, GetActiveStripeTail(index)).WillOnce(Return(UNMAP_VSA));
    EXPECT_CALL(allocCtx, GetActiveWbStripeId(index)).WillOnce(Return(UNMAP_STRIPE));

    Stripe* actual = wbStripeManager._FinishActiveStripe(index);
    EXPECT_EQ(actual, nullptr);
}

TEST(WBStripeManager, _FinishActiveStripe_testIfReturnsStripeWhenActiveStripeIsPicked)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockAllocatorCtx> allocCtx;
    NiceMock<MockBlockManager> blkManager;
    ON_CALL(addrInfo, GetblksPerStripe).WillByDefault(Return(128));
    ON_CALL(ctxManager, GetAllocatorCtx).WillByDefault(Return(&allocCtx));

    WBStripeManagerSpy wbStripeManager(nullptr, &addrInfo, &ctxManager, &blkManager, "", 0);

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    ASTailArrayIdx index = 3;

    std::mutex lock;
    EXPECT_CALL(allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    VirtualBlkAddr tail = {
        .stripeId = 2,
        .offset = 0};
    EXPECT_CALL(allocCtx, GetActiveStripeTail(index)).WillOnce(Return(tail));
    EXPECT_CALL(allocCtx, GetActiveWbStripeId(index)).WillOnce(Return(4));

    Stripe* actual = wbStripeManager._FinishActiveStripe(index);
    EXPECT_NE(actual, nullptr);
}

} // namespace pos
