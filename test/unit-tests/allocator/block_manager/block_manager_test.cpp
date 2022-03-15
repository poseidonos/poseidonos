#include "src/allocator/block_manager/block_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/block_manager/block_manager_spy.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
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
using ::testing::A;

namespace pos
{
TEST(BlockManager, Init_TestSimpleFunc)
{
    // given
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
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
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockTelemetryPublisher>* tp = new NiceMock<MockTelemetryPublisher>();
    BlockManager blkManager(tp, nullptr, nullptr, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(wbCtxLock));

    // given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 0,
        .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillRepeatedly(Return(vsa));
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, TryProhibitBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, IsUserBlockAllocationProhibited)
        .WillOnce(Return(false)).WillOnce(Return(true)).WillOnce(Return(true));

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // when 1.
    auto ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 1.
    EXPECT_EQ(1, ret.first.numBlks);
    // given 2.
    blkManager.ProhibitUserBlkAlloc();
    // when 2.
    ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 2.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);
    // given 3.
    blkManager.BlockAllocating(0);
    // when 3.
    ret = blkManager.AllocateWriteBufferBlks(0, 1);
    // then 3.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);
    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete tp;
}

TEST(BlockManager, AllocateGcDestStripe_testIfReturnsStripeWhenSuccessToAllocateUserLsid)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Block allocation is not prohibited
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    // Given: Success to allocate ssd user stripe
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(5));

    // when
    Stripe* stripe = blkManager.AllocateGcDestStripe(0);

    // then
    EXPECT_NE(stripe, nullptr);
    delete stripe;

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
}

TEST(BlockManager, AllocateGcDestStripe_testIfReturnsStripeWhenSuccessToAllocateNewSegmentAndUserLsid)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    std::mutex ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Block allocation is not prohibited
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(9));
    // Expect
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(2));

    // when
    Stripe* stripe = blkManager.AllocateGcDestStripe(0);

    // then
    EXPECT_NE(stripe, nullptr);
    delete stripe;

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete revMapPack;
}

TEST(BlockManager, AllocateGcDestStripe_testIfReturnsNullIRightAwayInsteadOfBlockingForeverWhenFreeSegmentIsNotAvailable)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);

    std::mutex allocCtxLock;
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Block allocation is not prohibited
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillRepeatedly(Return(false));
    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(9));
    // Given: Failed to allocate new segment
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));

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

TEST(BlockManager, AllocateGcDestStripe_testIfReturnsNullWhenBlockAllocationIsProhibited)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(32);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    BlockManager blkManager(nullptr, nullptr, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);

    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Block allocation is prohibited
    EXPECT_CALL(blockAllocationStatus, IsBlockAllocationProhibited).WillRepeatedly(Return(true));
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

TEST(BlockManager, _AllocateBlks_testIfReturnsUnmapVsaWhenFailedToAllocateWbStripeId)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
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
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 5};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));

    // Given: Failed to allocate new write buffer stripe
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(UNMAP_STRIPE));

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then 0.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_testIfReturnsUnmapVsaWhenUserBlockAllocationIsProhibited)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 5};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));

    // Given: Success to allocate new write buffer stripe
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));

    // Given: Current ssd segment is full
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(9));

    // Given: User block allocation is prohibited
    EXPECT_CALL(blockAllocationStatus, IsUserBlockAllocationProhibited).WillOnce(Return(true));

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_testIfReturnsUnmapVsaWhenFailedToAllocateFreeSegment)
{
    // given
    NiceMock<MockAllocatorAddressInfo>* addrInfo = new NiceMock<MockAllocatorAddressInfo>();
    EXPECT_CALL(*addrInfo, GetstripesPerSegment).WillRepeatedly(Return(10));
    EXPECT_CALL(*addrInfo, GetblksPerStripe).WillRepeatedly(Return(5));
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    std::mutex wbLock, ctxLock, allocCtxLock;

    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillRepeatedly(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 5};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));

    // Given: Success to allocate new write buffer stripe
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(0));

    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(9));

    // Given: Failed to allocate ssd user stripe
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
    delete addrInfo;
}

TEST(BlockManager, _AllocateBlks_testIfReturnsAllocatedVsaAndWbStripeId)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();

    std::mutex wbLock, ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 5};
    VirtualBlkAddr newVsa = {
        .stripeId = 0,
        .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(newVsa));

    // Given: Success to allocate new write buffer stripe
    StripeId wbLsid = 5;
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(wbLsid));

    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(9));

    // Given: Success to allocate new ssd user stripe
    EXPECT_CALL(*ctxManager, AllocateFreeSegment).WillOnce(Return(0));

    EXPECT_CALL(*allocCtx, SetCurrentSsdLsid(0)).Times(1);
    EXPECT_CALL(*iWbstripe, GetStripe(A<StripeId>())).WillOnce(Return(stripe));
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*allocCtx, SetNewActiveStripeTail(_, newVsa, wbLsid)).Times(1);

    EXPECT_CALL(*iStripeMap, SetLSA(0, _, IN_WRITE_BUFFER_AREA)).Times(1);
    VirtualBlkAddr updatedVsa = {
        .stripeId = 0,
        .offset = 1};
    EXPECT_CALL(*allocCtx, SetActiveStripeTail(_, updatedVsa)).Times(1);

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, 0);
    EXPECT_EQ(ret.first.startVsa.offset, 0);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, wbLsid);

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

TEST(BlockManager, _AllocateBlks_testWhenAllocatingBlocksFromUserStripeWithoutNewStripeAllocation)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0, true);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();

    std::mutex wbLock, ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 0};
    StripeId wbLsid = 5;
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillRepeatedly(Return(vsa));
    EXPECT_CALL(*allocCtx, GetActiveWbStripeId).WillRepeatedly(Return(wbLsid));
    VirtualBlkAddr updatedVsa = {
        .stripeId = 10,
        .offset = 1};
    EXPECT_CALL(*allocCtx, SetActiveStripeTail(_, updatedVsa)).Times(1);

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, 10);
    EXPECT_EQ(ret.first.startVsa.offset, 0);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, wbLsid);

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

TEST(BlockManager, _AllocateBlks_testWhenAllocatingBlocksFromUserStripe)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockAllocatorCtx>* allocCtx = new NiceMock<MockAllocatorCtx>();
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockSegmentCtx>* segCtx = new NiceMock<MockSegmentCtx>();
    NiceMock<MockGcCtx>* gcCtx = new NiceMock<MockGcCtx>();
    NiceMock<MockContextManager>* ctxManager = new NiceMock<MockContextManager>();
    NiceMock<MockReverseMapManager>* reverseMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockIStripeMap>* iStripeMap = new NiceMock<MockIStripeMap>();
    BlockManagerSpy blkManager(nullptr, iStripeMap, reverseMap, allocCtx, &blockAllocationStatus, &addrInfo, ctxManager, 0, true);
    blkManager.Init(iWbstripe);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();

    std::mutex wbLock, ctxLock, allocCtxLock;
    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*allocCtx, GetCtxLock).WillRepeatedly(ReturnRef(allocCtxLock));
    EXPECT_CALL(*ctxManager, GetCtxLock).WillRepeatedly(ReturnRef(ctxLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 5};

    StripeId newVsid = 13;
    VirtualBlkAddr newVsa = {
        .stripeId = newVsid,
        .offset = 0};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(newVsa));

    // Given: Success to allocate new write buffer stripe
    StripeId wbLsid = 5;
    EXPECT_CALL(*allocCtx, AllocFreeWbStripe).WillOnce(Return(wbLsid));

    // Given: Success to allocate new user data stripe
    EXPECT_CALL(*allocCtx, GetCurrentSsdLsid).WillOnce(Return(newVsid - 1));

    EXPECT_CALL(*allocCtx, SetCurrentSsdLsid(newVsid)).Times(1);
    EXPECT_CALL(*iWbstripe, GetStripe(A<StripeId>())).WillOnce(Return(stripe));
    EXPECT_CALL(*stripe, Assign).Times(1);
    EXPECT_CALL(*allocCtx, SetNewActiveStripeTail(_, newVsa, wbLsid)).Times(1);

    EXPECT_CALL(*iStripeMap, SetLSA(newVsid, newVsid, IN_USER_AREA)).Times(1);
    VirtualBlkAddr updatedVsa = {
        .stripeId = newVsid,
        .offset = 1};
    EXPECT_CALL(*allocCtx, SetActiveStripeTail(_, updatedVsa)).Times(1);

    // when
    auto ret = blkManager._AllocateBlks(0, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, newVsid);
    EXPECT_EQ(ret.first.startVsa.offset, 0);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, wbLsid);

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

TEST(BlockManager, _AllocateBlks_testIfReturnsAllocatedVsaAndUnmapStripeIdWhenNewStripeIsNotAllocated)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
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

    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 3};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(vsa));
    StripeId wbLsid = 13;
    EXPECT_CALL(*allocCtx, GetActiveWbStripeId).WillRepeatedly(Return(wbLsid));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);

    // when
    auto ret = blkManager._AllocateBlks(0, 2);

    // Then
    EXPECT_EQ(ret.first.startVsa.stripeId, 10);
    EXPECT_EQ(ret.first.startVsa.offset, 3);
    EXPECT_EQ(ret.first.numBlks, 2);
    EXPECT_EQ(ret.second, wbLsid);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}

TEST(BlockManager, _AllocateBlks_testIfReturnsOnlyAllocatedBlockNumbaerWhenRequestedSizeIsLargerThanTheStripe)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(10);
    addrInfo.SetblksPerStripe(5);
    NiceMock<MockIWBStripeAllocator>* iWbstripe = new NiceMock<MockIWBStripeAllocator>();
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

    EXPECT_CALL(*allocCtx, GetActiveStripeTailLock).WillOnce(ReturnRef(wbLock));
    EXPECT_CALL(*ctxManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
    EXPECT_CALL(*ctxManager, GetGcCtx).WillRepeatedly(Return(gcCtx));

    // Given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 3};
    EXPECT_CALL(*allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(vsa));
    StripeId wbLsid = 12;
    EXPECT_CALL(*allocCtx, GetActiveWbStripeId).WillRepeatedly(Return(wbLsid));
    EXPECT_CALL(*allocCtx, SetActiveStripeTail).Times(1);

    // when
    auto ret = blkManager._AllocateBlks(0, 11);

    EXPECT_EQ(ret.first.startVsa.stripeId, 10);
    EXPECT_EQ(ret.first.startVsa.offset, 3);
    EXPECT_EQ(ret.first.numBlks, 2);
    EXPECT_EQ(ret.second, wbLsid);

    delete iWbstripe;
    delete allocCtx;
    delete segCtx;
    delete gcCtx;
    delete ctxManager;
    delete reverseMap;
    delete iStripeMap;
}
} // namespace pos
