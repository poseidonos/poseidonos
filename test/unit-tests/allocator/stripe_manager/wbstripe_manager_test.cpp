#include "src/allocator/stripe_manager/wbstripe_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_spy.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_load_status_mock.h"
#include "test/unit-tests/allocator/stripe_manager/wbstripe_manager_spy.h"
#include "test/unit-tests/bio/flush_io_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{

class WBStripeManagerTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    WBStripeManagerSpy* wbStripeManager;
    AllocatorAddressInfo info;

    NiceMock<MockTelemetryPublisher> telemetryPublisher;
    NiceMock<MockIReverseMap> reverseMap;
    NiceMock<MockIVolumeInfoManager> volumeInfo;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockAllocatorCtx> allocatorCtx;
    NiceMock<MockStripeLoadStatus>* stripeLoadStatus;
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockMemoryManager> memoryManager;
    NiceMock<MockBufferPool>* bufferPool;

    void* buffer;

private:
    void _SetAllocatorAddressInfo(void);
};

void
WBStripeManagerTestFixture::SetUp(void)
{
    _SetAllocatorAddressInfo();

    ON_CALL(stripeMap, IsInUserDataArea).WillByDefault([&](StripeAddr addr)
    {
        return addr.stripeLoc == StripeLoc::IN_USER_AREA;
    });

    BufferInfo dummyInfo;
    bufferPool = new NiceMock<MockBufferPool>(dummyInfo, 0, nullptr);
    buffer = malloc(1);

    EXPECT_CALL(memoryManager, CreateBufferPool).WillRepeatedly(Return(bufferPool));
    ON_CALL(*bufferPool, TryGetBuffer).WillByDefault(Return(buffer));

    int numVolumes = 256;
    stripeLoadStatus = new NiceMock<MockStripeLoadStatus>();
    wbStripeManager = new WBStripeManagerSpy(&telemetryPublisher, numVolumes,
        &reverseMap, &volumeInfo, &stripeMap, &allocatorCtx, &info, stripeLoadStatus,
        "TestArray", 0, &memoryManager, &eventScheduler);

    wbStripeManager->Init();
}

void
WBStripeManagerTestFixture::TearDown(void)
{
    wbStripeManager->Dispose();

    free(buffer);

    delete bufferPool;
    delete wbStripeManager;
}

void
WBStripeManagerTestFixture::_SetAllocatorAddressInfo(void)
{
    info.SetblksPerStripe(128);
    info.SetnumWbStripes(5);
    info.SetchunksPerStripe(1);
    info.SetstripesPerSegment(64);
    info.SetnumUserAreaSegments(8);
    info.SetnumUserAreaStripes(64 * 8);
    info.SetblksPerSegment(8 * 64 * 128);
    info.SetUT(true);
}

TEST_F(WBStripeManagerTestFixture, FreeWBStripeId_TestSimpleCaller)
{
    EXPECT_CALL(allocatorCtx, ReleaseWbStripe).WillOnce([&](StripeId stripeId) {
        // wbStripeArray should be cleared at this point
        EXPECT_EQ(wbStripeManager->GetStripe(stripeId), nullptr);
    });

    StripeId wbLsid = 0;

    // given: stripe is allocated
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(wbLsid));
    wbStripeManager->AssignStripe(StripeSmartPtr(stripe));

    // when
    wbStripeManager->FreeWBStripeId(wbLsid);
}

// to-do: need to fix
/*
TEST(WBStripeManager, FlushAllPendingStripesInVolume_TestVolumeMounted)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(5);
    NiceMock<MockIVolumeInfoManager>* volManager = new NiceMock<MockIVolumeInfoManager>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockIStripeMap> stripeMap;

    uint32_t volumeId = 3;
    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, volManager, &stripeMap, allocCtx, &addrInfo, ctxManager, blkManager, nullptr, "", 0);
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        EXPECT_CALL(*stripe, GetVolumeId).WillOnce(Return(volumeId));
        EXPECT_CALL(*stripe, UpdateFlushIo).Times(1);
        wbStripeManager.PushStripeToStripeArray(stripe);
    }
    // given 1.
    StripeId wbLsid = 4;
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
    NiceMock<MockIVolumeInfoManager>* volManager = new NiceMock<MockIVolumeInfoManager>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockBlockManager>* blkManager = new NiceMock<MockBlockManager>();
    NiceMock<MockIStripeMap> stripeMap;

    WBStripeManager wbStripeManager(nullptr, 1, nullptr, volManager, &stripeMap, allocCtx, &addrInfo, ctxManager, blkManager, nullptr, "", 0);
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
    NiceMock<MockIStripeMap> stripeMap;

    WBStripeManager wbStripeManager(nullptr, 1, nullptr, nullptr, &stripeMap, allocCtx, &addrInfo, ctxManager, blkManager, nullptr, "", 0);

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
*/
TEST_F(WBStripeManagerTestFixture, ReferLsidCnt_TestwithAllConditions)
{    
    // given 1. User area stripe is given
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_USER_AREA,
        .stripeId = 0,
    };
    // when 1.
    bool ret = wbStripeManager->ReferLsidCnt(lsa);
    // then 1.
    EXPECT_EQ(false, ret);

    // given 2. WB area stripe is given
    lsa.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA;
    lsa.stripeId = 3;
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(lsa.stripeId));
    wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    // when 2.
    ret = wbStripeManager->ReferLsidCnt(lsa);
    // then 2.
    EXPECT_EQ(true, ret);
}

TEST_F(WBStripeManagerTestFixture, DereferLsidCnt_TestwithAllConditions)
{
    // given 1.
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 3};
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(lsa.stripeId));
    wbStripeManager->AssignStripe(StripeSmartPtr(stripe));

    // when 1.
    uint32_t blockCount = 10;
    EXPECT_CALL(*stripe, Derefer(blockCount)).Times(1);
    wbStripeManager->DereferLsidCnt(lsa, blockCount);

    // given 2.
    lsa.stripeLoc = StripeLoc::IN_USER_AREA;
    lsa.stripeId = UNMAP_STRIPE;

    // when 2.
    wbStripeManager->DereferLsidCnt(lsa, 0);
}

TEST_F(WBStripeManagerTestFixture, FlushAllWbStripes_testIfWaitsForAllStripeFlush)
{
    // given
    std::mutex lock;
    EXPECT_CALL(allocatorCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    EXPECT_CALL(allocatorCtx, GetActiveStripeTail).WillRepeatedly(Return(UNMAP_VSA));

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetBlksRemaining).WillByDefault(Return(0));
        ON_CALL(*stripe, IsFinished).WillByDefault(Return(true));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    wbStripeManager->FlushAllWbStripes();
}

TEST_F(WBStripeManagerTestFixture, FinishStripe_TestwithAllConditions)
{
    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }
    // given 1.
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    StripeAddr lsa = {
        .stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    // when 1.
    wbStripeManager->FinishStripe(0, vsa);
    // given 2.
    vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    // when 2.
    wbStripeManager->FinishStripe(0, vsa);
}

TEST_F(WBStripeManagerTestFixture, FlushAllPendingStripes_testIfAllWbStripeIsFlushed)
{
    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetBlksRemaining).WillByDefault(Return(0));
        ON_CALL(*stripe, IsFinished).WillByDefault(Return(false));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    // when
    int ret = wbStripeManager->FlushAllPendingStripes();

    // Then
    EXPECT_EQ(ret, 0);
}

TEST_F(WBStripeManagerTestFixture, FlushAllPendingStripes_testIfOnlyNotFinishedStripesAreFlushed)
{
    // given
    for (int i = 0; i < 2; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetBlksRemaining).WillByDefault(Return(0));
        ON_CALL(*stripe, IsFinished).WillByDefault(Return(true));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        EXPECT_CALL(*stripe, Flush).Times(0);

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    for (int i = 2; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetBlksRemaining).WillByDefault(Return(0));
        ON_CALL(*stripe, IsFinished).WillByDefault(Return(false));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    // when
    int ret = wbStripeManager->FlushAllPendingStripes();

    // Then
    EXPECT_EQ(ret, 0);
}

TEST_F(WBStripeManagerTestFixture, FlushAllPendingStripes_testIfAllStripeIsFlushedThoughOneOfThemFails)
{
    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetBlksRemaining).WillByDefault(Return(0));
        ON_CALL(*stripe, IsFinished).WillByDefault(Return(false));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));

        if (i == 3)
        {
            EXPECT_CALL(*stripe, Flush).WillOnce(Return(-1));
        }
        else
        {
            EXPECT_CALL(*stripe, Flush).WillOnce(Return(0));
        }

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    // when
    int ret = wbStripeManager->FlushAllPendingStripes();
    // then
    EXPECT_EQ(-1, ret);
}

TEST_F(WBStripeManagerTestFixture, _ReconstructAS_TestwithAllConditions)
{
    // given
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(0));

    StripeSmartPtr stripePtr = StripeSmartPtr(stripe);

    // when 1.
    int ret = wbStripeManager->_ReconstructAS(stripePtr, 0);

    // given 2.
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(0));
    // when 2.
    ret = wbStripeManager->_ReconstructAS(stripePtr, 1);

    // given 3.
    EXPECT_CALL(*stripe, DecreseBlksRemaining).WillOnce(Return(1));
    // when 2.
    ret = wbStripeManager->_ReconstructAS(stripePtr, 1);
}

TEST_F(WBStripeManagerTestFixture, ReconstructActiveStripe_TestFunc)
{
    // given
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = UNMAP_OFFSET};
    // when
    std::map<uint64_t, BlkAddr> revMapInfos;
    wbStripeManager->ReconstructActiveStripe(0, 0, vsa, revMapInfos);
}

TEST_F(WBStripeManagerTestFixture, _GetRemainingBlocks_TestFunc)
{
    uint32_t blksPerStripe = info.GetblksPerStripe();

    // given
    VirtualBlkAddr currentTail = UNMAP_VSA;
    VirtualBlks actual = wbStripeManager->_GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);

    currentTail.stripeId = 10;
    currentTail.offset = 2000;
    actual = wbStripeManager->_GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);

    currentTail.stripeId = 20;
    currentTail.offset = 10;
    actual = wbStripeManager->_GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa.stripeId, currentTail.stripeId);
    EXPECT_EQ(actual.startVsa.offset, currentTail.offset);
    EXPECT_EQ(actual.numBlks, blksPerStripe - currentTail.offset);

    currentTail.stripeId = 30;
    currentTail.offset = blksPerStripe;
    actual = wbStripeManager->_GetRemainingBlocks(currentTail);
    EXPECT_EQ(actual.startVsa, UNMAP_VSA);
    EXPECT_EQ(actual.numBlks, 0);
}

TEST_F(WBStripeManagerTestFixture, _FillBlocksToStripe_testIfStripeIsFilled)
{
    // given
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    StripeId wbLsid = 100;
    BlkOffset startOffset = 30;
    uint32_t numBlks = 5;

    for (uint32_t offset = startOffset; offset < startOffset + numBlks; offset++)
    {
        EXPECT_CALL(*stripe, UpdateReverseMapEntry(offset, INVALID_RBA, UINT32_MAX));
    }
    EXPECT_CALL(*stripe, DecreseBlksRemaining(numBlks)).WillOnce(Return(0));
    bool flushRequired = wbStripeManager->_FillBlocksToStripe(StripeSmartPtr(stripe), wbLsid, startOffset, numBlks);
    EXPECT_EQ(flushRequired, true);
}

TEST_F(WBStripeManagerTestFixture, _FillBlocksToStripe_testIfStripeIsNotFilled)
{
    // given
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    StripeId wbLsid = 100;
    BlkOffset startOffset = 30;
    uint32_t numBlks = 5;

    for (uint32_t offset = startOffset; offset < startOffset + numBlks; offset++)
    {
        EXPECT_CALL(*stripe, UpdateReverseMapEntry(offset, INVALID_RBA, UINT32_MAX));
    }
    EXPECT_CALL(*stripe, DecreseBlksRemaining(numBlks)).WillOnce(Return(1));
    EXPECT_CALL(*stripe, SetActiveFlushTarget).WillOnce(Return());
    bool flushRequired = wbStripeManager->_FillBlocksToStripe(StripeSmartPtr(stripe), wbLsid, startOffset, numBlks);
    EXPECT_EQ(flushRequired, false);
}

// to-do: need to fix
/*
TEST(WBStripeManager, _FinishActiveStripe_testIfReturnsNullWhenNoActiveStripeExistForTheVolume)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockAllocatorCtx> allocCtx;
    NiceMock<MockBlockManager> blkManager;
    NiceMock<MockIStripeMap> stripeMap;
    ON_CALL(ctxManager, GetAllocatorCtx).WillByDefault(Return(&allocCtx));

    WBStripeManagerSpy wbStripeManager(nullptr, 1, nullptr, nullptr, &stripeMap, &allocCtx, &addrInfo, &ctxManager, &blkManager, nullptr, "", 0);

    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        wbStripeManager.PushStripeToStripeArray(stripe);
    }

    ASTailArrayIdx index = 3;
    EXPECT_CALL(allocCtx, GetActiveStripeTail(index)).WillOnce(Return(UNMAP_VSA));

    Stripe* actual = wbStripeManager._FinishActiveStripe(index);
    EXPECT_EQ(actual, nullptr);
}
*/

TEST_F(WBStripeManagerTestFixture, _FinishActiveStripe_testIfReturnsWhenStripeIsInUserDataArea)
{
    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetWbLsid()).WillByDefault(Return(i));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    ASTailArrayIdx index = 3;

    std::mutex lock;
    EXPECT_CALL(allocatorCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    VirtualBlkAddr tail = {
        .stripeId = 2,
        .offset = 0};
    EXPECT_CALL(allocatorCtx, GetActiveStripeTail(index)).WillRepeatedly(Return(tail));
    StripeAddr flushedStripe = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 2};
    EXPECT_CALL(stripeMap, GetLSA(2)).WillOnce(Return(flushedStripe));

    StripeSmartPtr actual = wbStripeManager->_FinishActiveStripe(index);
    EXPECT_EQ(actual, nullptr);
}

TEST_F(WBStripeManagerTestFixture, _FinishActiveStripe_testIfReturnsStripeWhenActiveStripeIsPicked)
{
    // given
    for (int i = 0; i < 5; i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetWbLsid()).WillByDefault(Return(i));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    ASTailArrayIdx index = 3;

    std::mutex lock;
    EXPECT_CALL(allocatorCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(lock));
    VirtualBlkAddr tail = {
        .stripeId = 2,
        .offset = 0};
    EXPECT_CALL(allocatorCtx, GetActiveStripeTail(index)).WillRepeatedly(Return(tail));
    StripeAddr wbStripe = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 2};
    EXPECT_CALL(stripeMap, GetLSA(2)).WillOnce(Return(wbStripe));

    StripeSmartPtr actual = wbStripeManager->_FinishActiveStripe(index);
    EXPECT_NE(actual, nullptr);
}

TEST_F(WBStripeManagerTestFixture, LoadPendingStripesToWriteBuffer_testIfStripeLoadRequested)
{
    // Add unflushed stripes
    for (uint32_t i = 0; i < info.GetnumWbStripes(); i++)
    {
        NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
        ON_CALL(*stripe, GetVsid).WillByDefault(Return(10 + i));
        ON_CALL(*stripe, GetUserLsid).WillByDefault(Return(10 + i));
        ON_CALL(*stripe, GetWbLsid).WillByDefault(Return(i));
        StripeAddr wbAddr = {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = i};
        EXPECT_CALL(stripeMap, GetLSA(10 + i)).WillOnce(Return(wbAddr));

        wbStripeManager->AssignStripe(StripeSmartPtr(stripe));
    }

    {
        InSequence s;
        EXPECT_CALL(*stripeLoadStatus, Reset).Times(1);
        EXPECT_CALL(*stripeLoadStatus, StripeLoadStarted).Times(5);
        EXPECT_CALL(*stripeLoadStatus, IsDone).WillOnce(Return(true));
    }

    wbStripeManager->LoadPendingStripesToWriteBuffer();
}

} // namespace pos
