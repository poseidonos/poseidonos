#include "src/allocator/block_manager/block_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_spy.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_internal_mock.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(BlockManager, Init_TestSimpleFunc)
{
    // given
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    BlockManager blkManager;
    // when
    blkManager.Init(iWbstripe);
    delete iWbstripe;
}

TEST(BlockManager, AllocateWriteBufferBlks_TestFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    BlockManager blkManager(tp, nullptr, nullptr, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    VirtualBlkAddr vsa;
    vsa.offset = 0;
    vsa.stripeId = 0;
    std::mutex wbCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(wbCtxLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, TryProhibitBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, IsUserBlockAllocationProhibited)
        .WillOnce(Return(false)).WillOnce(Return(true)).WillOnce(Return(true));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when 1.
    VirtualBlks ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 1.
    EXPECT_EQ(1, ret.numBlks);
    // given 2.
    blkManager.ProhibitUserBlkAlloc();
    // when 2.
    ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 2.
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);
    // given 3.
    blkManager.BlockAllocating(0);
    // when 3.
    ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 3.
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete tp;
}

TEST(BlockManager, AllocateGcDestStripe_TestFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    std::mutex Lock;
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(Lock));
    EXPECT_CALL(*reverseMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillOnce(Return(false)).WillOnce(Return(true));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    blkManager.AllocateGcDestStripe(0);
    // given 2.
    blkManager.BlockAllocating(0);
    // when 2.
    blkManager.AllocateGcDestStripe(0);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
}

TEST(BlockManager, AllocateGcDestStripe_TestFuncFailCase1)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    ON_CALL(*addrInfo, GetstripesPerSegment).WillByDefault(Return(10));
    ON_CALL(*addrInfo, GetblksPerStripe).WillByDefault(Return(32));
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    std::mutex Lock;
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(Lock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(20));
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    Stripe* ret = blkManager.AllocateGcDestStripe(0);
    // then
    EXPECT_EQ(nullptr, ret);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
    delete addrInfo;
}

TEST(BlockManager, AllocateGcDestStripe_TestFuncFailCase2)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    std::mutex Lock;
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(Lock));
    EXPECT_CALL(*reverseMap, AllocReverseMapPack).WillOnce(Return(revMapPack));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    Stripe* ret = blkManager.AllocateGcDestStripe(0);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
}

TEST(BlockManager, AllocateGcDestStripe_TestFuncFailCase3)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillRepeatedly(Return(true));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    Stripe* ret = blkManager.AllocateGcDestStripe(0);
    // then
    EXPECT_EQ(nullptr, ret);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
}

TEST(BlockManager, ProhibitUserBlkAlloc_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    BlockManager blkManager(tp, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    // then
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);

    // when
    blkManager.ProhibitUserBlkAlloc();
    delete tp;
}

TEST(BlockManager, PermitUserBlkAlloc_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    BlockManager blkManager(tp, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    // then
    EXPECT_CALL(blockAllocationStatus, PermitUserBlockAllocation).Times(1);
    // when
    blkManager.PermitUserBlkAlloc();
    delete tp;
}

TEST(BlockManager, BlockAllocating_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    BlockManager blkManager(nullptr, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    int volumeId = 1;
    // then
    EXPECT_CALL(blockAllocationStatus, TryProhibitBlockAllocation(volumeId)).WillOnce(Return(true));
    // when
    bool actual = blkManager.BlockAllocating(volumeId);
    EXPECT_EQ(actual, true);
}

TEST(BlockManager, UnblockAllocating_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    BlockManager blkManager(nullptr, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    int volumeId = 0;
    // then
    EXPECT_CALL(blockAllocationStatus, PermitBlockAllocation(volumeId)).Times(1);
    // when
    blkManager.UnblockAllocating(volumeId);
}

TEST(BlockManager, TurnOffBlkAllocation_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    BlockManager blkManager(nullptr, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    blkManager.TurnOnBlkAllocation();
    // then
    EXPECT_CALL(blockAllocationStatus, ProhibitBlockAllocation()).Times(1);
    // when
    blkManager.TurnOffBlkAllocation();
}

TEST(BlockManager, TurnOnBlkAllocation_TestSimpleSetter)
{
    // given
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    BlockManager blkManager(nullptr, nullptr, nullptr, nullptr, &blockAllocationStatus, nullptr, nullptr, 0);
    // then
    EXPECT_CALL(blockAllocationStatus, PermitBlockAllocation()).Times(1);
    // when
    blkManager.TurnOnBlkAllocation();
}

TEST(BlockManager, _AllocateBlks_TestCase1)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};
    // given 0.
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(UNMAP_STRIPE));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when 0.
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);
    // then 0.
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_TestCase2)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};

    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(ctxLock)).WillOnce(ReturnRef(ctxLock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(10));
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    EXPECT_CALL(*gcCtx, GetCurrentGcMode).WillOnce(Return(MODE_URGENT_GC));

    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);
    // then
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_TestCase3_1)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};
    // given 2.
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(ctxLock)).WillOnce(ReturnRef(ctxLock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(10));
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*addrInfo, GetstripesPerSegment).WillOnce(Return(10));
    EXPECT_CALL(*addrInfo, GetblksPerStripe).WillOnce(Return(5));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);
    // then
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
    delete addrInfo;
}

TEST(BlockManager, _AllocateBlks_TestCase3_2)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    ON_CALL(*addrInfo, GetstripesPerSegment).WillByDefault(Return(10));
    ON_CALL(*addrInfo, GetblksPerStripe).WillByDefault(Return(5));
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};
    // given 2.
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(ctxLock)).WillOnce(ReturnRef(ctxLock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(10));
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_CALL(*addrInfo, IsUT).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);
    // then
    EXPECT_EQ(UNMAP_VSA, ret.startVsa);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
    delete addrInfo;
}

TEST(BlockManager, _AllocateBlks_TestCase4)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();

    // given
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(ctxLock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(10));
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(0));

    EXPECT_CALL(*iWbstripe, GetStripe).WillOnce(Return(stripe));
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
    delete iStripeMap;
    delete stripe;
}

TEST(BlockManager, _AllocateBlks_TestCase5)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 5};
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    // given 4.
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillOnce(ReturnRef(ctxLock));
    EXPECT_CALL(*allocCtx, UpdatePrevLsid).WillOnce(Return(10));
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(0));

    EXPECT_CALL(*iWbstripe, GetStripe).WillOnce(Return(stripe));
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*iStripeMap, SetLSA).Times(1);

    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    VirtualBlks ret = blkManager._AllocateBlks(0, 1);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
    delete iStripeMap;
    delete stripe;
}

TEST(BlockManager, _AllocateBlks_TestCase6)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock;
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 3};
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_TestCase7)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 3};
    std::mutex wbLock, ctxLock;
    // given
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 1);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_TestCase8)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 3};
    std::mutex wbLock, ctxLock;
    // given
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    // when
    VirtualBlks ret = blkManager._AllocateBlks(0, 11);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateWriteBufferBlksFromNewStripe_TestInternalFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeInternal>* iWbstripe = new NiceMock<MockIWBStripeInternal>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    VirtualBlkAddr vsa = {.stripeId = 10, .offset = 3};
    std::mutex wbLock, ctxLock;

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    // when
    VirtualBlks ret = blkManager._AllocateWriteBufferBlksFromNewStripe(0, 0, 11);
    // then
    EXPECT_EQ(5, ret.numBlks);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateStripe_TestInternalFunc)
{
}

} // namespace pos
