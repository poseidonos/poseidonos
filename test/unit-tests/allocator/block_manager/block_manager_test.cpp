#include "src/allocator/block_manager/block_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/address_type.h"
#include "test/unit-tests/allocator/block_manager/block_manager_spy.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_manager_mock.h"
#include "test/unit-tests/allocator/stripe_manager/stripe_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"

using ::testing::_;
using ::testing::A;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::ReturnRef;

namespace pos
{
class BlockManagerTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp(void) override;
    virtual void TearDown(void) override;

    BlockManagerSpy* blockManager;

    AllocatorAddressInfo addrInfo;
    NiceMock<MockStripeManager> stripeManager;
    NiceMock<MockAllocatorCtx> allocCtx;
    NiceMock<MockBlockAllocationStatus> blockAllocationStatus;
    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockTelemetryPublisher> tp;

    std::mutex wbLock, ctxLock, allocCtxLock;

private:
    void _SetAllocatorAddressInfo(void);
};

void
BlockManagerTestFixture::SetUp(void)
{
    _SetAllocatorAddressInfo();

    ON_CALL(allocCtx, GetActiveStripeTailLock).WillByDefault(ReturnRef(wbLock));
    ON_CALL(allocCtx, GetCtxLock).WillByDefault(ReturnRef(allocCtxLock));
    ON_CALL(ctxManager, GetCtxLock).WillByDefault(ReturnRef(ctxLock));

    blockManager = new BlockManagerSpy(&tp, &allocCtx, &blockAllocationStatus,
        &addrInfo, &ctxManager, 0);

    blockManager->Init(&stripeManager);
}

void
BlockManagerTestFixture::TearDown(void)
{
    delete blockManager;
}

void
BlockManagerTestFixture::_SetAllocatorAddressInfo(void)
{
    addrInfo.SetblksPerStripe(32);
    addrInfo.SetnumWbStripes(1024);
    addrInfo.SetchunksPerStripe(1);
    addrInfo.SetstripesPerSegment(64);
    addrInfo.SetnumUserAreaSegments(8);
    addrInfo.SetnumUserAreaStripes(64 * 8);
    addrInfo.SetblksPerSegment(8 * 64 * 32);
}

TEST_F(BlockManagerTestFixture, AllocateWriteBufferBlks_testAllocationWhenActiveStripeIsNotFull)
{
    // given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 0,
        .offset = 0};
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillRepeatedly(Return(vsa));
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, TryProhibitBlockAllocation).Times(1);
    EXPECT_CALL(blockAllocationStatus, IsUserBlockAllocationProhibited)
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(true));

    // when 1.
    auto ret = blockManager->AllocateWriteBufferBlks(0, 1);
    // then 1.
    EXPECT_EQ(1, ret.first.numBlks);

    // when 2. block allocation is prohibited
    blockManager->ProhibitUserBlkAlloc();
    ret = blockManager->AllocateWriteBufferBlks(0, 1);
    // then 2.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);

    // when 3. block allocation of volume 0 is blocked
    blockManager->BlockAllocating(0);
    ret = blockManager->AllocateWriteBufferBlks(0, 1);
    // then 3.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);
}

TEST_F(BlockManagerTestFixture, ProhibitUserBlkAlloc_TestSimpleSetter)
{
    // then
    EXPECT_CALL(blockAllocationStatus, ProhibitUserBlockAllocation).Times(1);

    // when
    blockManager->ProhibitUserBlkAlloc();
}

TEST_F(BlockManagerTestFixture, PermitUserBlkAlloc_TestSimpleSetter)
{
    // then
    EXPECT_CALL(blockAllocationStatus, PermitUserBlockAllocation).Times(1);

    // when
    blockManager->PermitUserBlkAlloc();
}

TEST_F(BlockManagerTestFixture, BlockAllocating_TestSimpleSetter)
{
    // given
    int volumeId = 1;
    // then
    EXPECT_CALL(blockAllocationStatus, TryProhibitBlockAllocation(volumeId)).WillOnce(Return(true));
    // when
    bool actual = blockManager->BlockAllocating(volumeId);
    EXPECT_EQ(actual, true);
}

TEST_F(BlockManagerTestFixture, UnblockAllocating_TestSimpleSetter)
{
    // given
    int volumeId = 0;
    // then
    EXPECT_CALL(blockAllocationStatus, PermitBlockAllocation(volumeId)).Times(1);
    // when
    blockManager->UnblockAllocating(volumeId);
}

TEST_F(BlockManagerTestFixture, TurnOffBlkAllocation_TestSimpleSetter)
{
    // then
    EXPECT_CALL(blockAllocationStatus, ProhibitBlockAllocation()).Times(1);
    // when
    blockManager->TurnOffBlkAllocation();
}

TEST_F(BlockManagerTestFixture, TurnOnBlkAllocation_TestSimpleSetter)
{
    // then
    EXPECT_CALL(blockAllocationStatus, PermitBlockAllocation()).Times(1);
    // when
    blockManager->TurnOnBlkAllocation();
}

TEST_F(BlockManagerTestFixture, _AllocateBlks_testIfBlockAllocatedWhenActiveTailIsUnmap)
{
    uint32_t volumeId = 0;

    // Given: Active stripe tail is UNMAP
    VirtualBlkAddr vsa = UNMAP_VSA;
    VirtualBlkAddr newVsa = {
        .stripeId = 9,
        .offset = 0};
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(newVsa));

    // Given: Success to allocate new stripes
    StripeId wbLsid = 5;
    StripeId userLsid = newVsa.stripeId;
    EXPECT_CALL(stripeManager, AllocateStripesForUser).WillOnce(Return(std::make_pair(wbLsid, userLsid)));

    VirtualBlkAddr updatedVsa = {
        .stripeId = newVsa.stripeId,
        .offset = newVsa.offset + 1};
    EXPECT_CALL(allocCtx, SetActiveStripeTail(volumeId, updatedVsa)).Times(1);

    // when
    auto ret = blockManager->_AllocateBlks(volumeId, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, newVsa.stripeId);
    EXPECT_EQ(ret.first.startVsa.offset, newVsa.offset);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, userLsid);
}

TEST_F(BlockManagerTestFixture, _AllocateBlks_testIfBlockAllocatedWhenActiveTailIsFull)
{
    uint32_t volumeId = 0;

    // Given: Active stripe tail is UNMAP
    VirtualBlkAddr vsa = {
        .stripeId = 7,
        .offset = addrInfo.GetblksPerStripe()};
    VirtualBlkAddr newVsa = {
        .stripeId = 9,
        .offset = 0};
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(newVsa));

    // Given: Success to allocate new stripes
    StripeId wbLsid = 5;
    StripeId userLsid = newVsa.stripeId;
    EXPECT_CALL(stripeManager, AllocateStripesForUser).WillOnce(Return(std::make_pair(wbLsid, userLsid)));

    VirtualBlkAddr updatedVsa = {
        .stripeId = newVsa.stripeId,
        .offset = newVsa.offset + 1};
    EXPECT_CALL(allocCtx, SetActiveStripeTail(volumeId, updatedVsa)).Times(1);

    // when
    auto ret = blockManager->_AllocateBlks(volumeId, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, newVsa.stripeId);
    EXPECT_EQ(ret.first.startVsa.offset, newVsa.offset);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, userLsid);
}

TEST_F(BlockManagerTestFixture, _AllocateBlks_testIfReturnsUnmapVsaWhenFailedToAllocateStripes)
{
    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = addrInfo.GetblksPerStripe()};
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillOnce(Return(vsa));

    // Given: Failed to allocate new stripes
    EXPECT_CALL(stripeManager, AllocateStripesForUser).WillOnce(Return(std::make_pair(UNMAP_STRIPE, 0)));

    // when
    auto ret = blockManager->_AllocateBlks(0, 1);

    // then 0.
    EXPECT_EQ(UNMAP_VSA, ret.first.startVsa);
}

TEST_F(BlockManagerTestFixture, _AllocateBlks_testIfReturnsAllocatedVsaAndUserStripeId)
{
    // Given: Active stripe tail is full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = addrInfo.GetblksPerStripe()};
    VirtualBlkAddr newVsa = {
        .stripeId = 9,
        .offset = 0};
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(newVsa));

    // Given: Success to allocate new stripes
    StripeId wbLsid = 5;
    StripeId userLsid = newVsa.stripeId;
    EXPECT_CALL(stripeManager, AllocateStripesForUser).WillOnce(Return(std::make_pair(wbLsid, 9)));

    VirtualBlkAddr updatedVsa = {
        .stripeId = newVsa.stripeId,
        .offset = newVsa.offset + 1};
    EXPECT_CALL(allocCtx, SetActiveStripeTail(_, updatedVsa)).Times(1);

    // when
    auto ret = blockManager->_AllocateBlks(0, 1);

    // then
    EXPECT_EQ(ret.first.startVsa.stripeId, newVsa.stripeId);
    EXPECT_EQ(ret.first.startVsa.offset, newVsa.offset);
    EXPECT_EQ(ret.first.numBlks, 1);

    EXPECT_EQ(ret.second, userLsid);
}

TEST_F(BlockManagerTestFixture, _AllocateBlks_testIfReturnsOnlyAllocatedBlockNumbaerWhenRequestedSizeIsLargerThanTheStripe)
{
    // Given: Active stripe tail is not full
    VirtualBlkAddr vsa = {
        .stripeId = 10,
        .offset = 3};
    StripeId userLsid = 10;
    EXPECT_CALL(allocCtx, GetActiveStripeTail).WillOnce(Return(vsa)).WillOnce(Return(vsa));
    EXPECT_CALL(allocCtx, SetActiveStripeTail).Times(1);

    // when
    int blksToRequest = addrInfo.GetblksPerStripe();
    auto ret = blockManager->_AllocateBlks(0, blksToRequest);

    EXPECT_EQ(ret.first.startVsa.stripeId, vsa.stripeId);
    EXPECT_EQ(ret.first.startVsa.offset, vsa.offset);
    EXPECT_EQ(ret.first.numBlks, addrInfo.GetblksPerStripe() - vsa.offset);
    EXPECT_EQ(ret.second, userLsid);
}

} // namespace pos
