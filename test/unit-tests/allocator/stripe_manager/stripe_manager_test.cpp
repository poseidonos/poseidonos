#include "src/allocator/stripe_manager/stripe_manager.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/block_allocation_status_mock.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/mapper/i_reversemap_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class StripeManagerTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp(void) override;
    virtual void TearDown(void) override;

    StripeManager* stripeManager;

    NiceMock<MockContextManager> ctxManager;
    NiceMock<MockIReverseMap> reverseMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockIWBStripeAllocator> wbStripeManager;
    NiceMock<MockAllocatorCtx> allocCtx;
    NiceMock<MockBlockAllocationStatus> allocStatus;

    AllocatorAddressInfo addrInfo;
    int arrayId;

    std::mutex wbLock, allocCtxLock;

private:
    void _SetAllocatorAddressInfo();
};

void
StripeManagerTestFixture::SetUp(void)
{
    _SetAllocatorAddressInfo();

    arrayId = 0;

    ON_CALL(allocCtx, GetActiveStripeTailLock).WillByDefault(ReturnRef(wbLock));
    ON_CALL(allocCtx, GetCtxLock).WillByDefault(ReturnRef(allocCtxLock));

    ON_CALL(ctxManager, GetAllocatorCtx).WillByDefault(Return(&allocCtx));
    ON_CALL(ctxManager, GetAllocationStatus).WillByDefault(Return(&allocStatus));

    stripeManager = new StripeManager(&ctxManager, &reverseMap, &stripeMap, &addrInfo, arrayId);
    stripeManager->Init(&wbStripeManager);
}

void
StripeManagerTestFixture::TearDown(void)
{
    delete stripeManager;
}

void
StripeManagerTestFixture::_SetAllocatorAddressInfo(void)
{
    addrInfo.SetblksPerStripe(32);
    addrInfo.SetnumWbStripes(1024);
    addrInfo.SetchunksPerStripe(1);
    addrInfo.SetstripesPerSegment(64);
    addrInfo.SetnumUserAreaSegments(8);
    addrInfo.SetnumUserAreaStripes(64 * 8);
    addrInfo.SetblksPerSegment(8 * 64 * 32);
}

TEST_F(StripeManagerTestFixture, AllocateStripesForUser_testIfAllocFailsWhenWbStripeAllocationFailed)
{
    // Given: Failed to allocate new write buffer stripe
    uint32_t volumeId = 0;
    EXPECT_CALL(allocCtx, AllocFreeWbStripe).WillOnce(Return(UNMAP_STRIPE));

    // when
    auto ret = stripeManager->AllocateStripesForUser(volumeId);

    // then
    EXPECT_EQ(UNMAP_STRIPE, ret.first);
    EXPECT_EQ(UNMAP_STRIPE, ret.second);
}

TEST_F(StripeManagerTestFixture, AllocateStripesForUser_testIfAllocFailsWhenUserStripeAllocationIsProhibited)
{
    // Given: Failed to allocate new write buffer stripe
    uint32_t volumeId = 0;

    StripeId wbLsid = 10;
    StripeId userLsid = addrInfo.GetstripesPerSegment() - 1;

    EXPECT_CALL(allocCtx, AllocFreeWbStripe).WillOnce(Return(wbLsid));
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(userLsid));
    EXPECT_CALL(allocStatus, IsUserBlockAllocationProhibited(volumeId)).WillOnce(Return(true));

    EXPECT_CALL(allocCtx, ReleaseWbStripe(wbLsid)).Times(1);

    // when
    auto ret = stripeManager->AllocateStripesForUser(volumeId);

    // then
    EXPECT_EQ(UNMAP_STRIPE, ret.first);
    EXPECT_EQ(UNMAP_STRIPE, ret.second);
}

TEST_F(StripeManagerTestFixture, AllocateStripesForUser_testIfAllocFailsWhenUserSegmentAllocationFailed)
{
    // Given: Failed to allocate new write buffer stripe
    uint32_t volumeId = 0;

    StripeId wbLsid = 10;
    StripeId userLsid = addrInfo.GetstripesPerSegment() - 1;

    EXPECT_CALL(allocCtx, AllocFreeWbStripe).WillOnce(Return(wbLsid));
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(userLsid));

    EXPECT_CALL(allocStatus, IsUserBlockAllocationProhibited(volumeId)).WillOnce(Return(false));
    EXPECT_CALL(ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));

    EXPECT_CALL(allocCtx, ReleaseWbStripe(wbLsid)).Times(1);

    // when
    auto ret = stripeManager->AllocateStripesForUser(volumeId);

    // then
    EXPECT_EQ(UNMAP_STRIPE, ret.first);
    EXPECT_EQ(UNMAP_STRIPE, ret.second);
}

TEST_F(StripeManagerTestFixture, AllocateStripesForUser_testIfStripeAllocationSuccess)
{
    // given
    uint32_t volumeId = 0;

    StripeId wbLsid = 10;
    StripeId currentLsid = addrInfo.GetstripesPerSegment() * 2 + 1;

    EXPECT_CALL(allocCtx, AllocFreeWbStripe).WillOnce(Return(wbLsid));
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(currentLsid));

    StripeId userLsid = currentLsid + 1;

    EXPECT_CALL(allocCtx, SetCurrentSsdLsid(userLsid)).Times(1);

    EXPECT_CALL(wbStripeManager, AssignStripe);

    StripeId newVsid = userLsid;
    EXPECT_CALL(stripeMap, SetLSA(newVsid, wbLsid, IN_WRITE_BUFFER_AREA));

    VirtualBlkAddr curVsa = {
        .stripeId = newVsid,
        .offset = 0,
    };
    EXPECT_CALL(allocCtx, SetActiveStripeTail(volumeId, curVsa));

    // when
    auto ret = stripeManager->AllocateStripesForUser(volumeId);

    // then
    EXPECT_EQ(ret.first, wbLsid);
    EXPECT_EQ(ret.second, userLsid);
}

TEST_F(StripeManagerTestFixture, AllocateGcDestStripe_testIfReturnsStripeWhenSuccessToAllocateUserLsid)
{
    // Given: Block allocation is not prohibited
    EXPECT_CALL(allocStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    // Given: Success to allocate ssd user stripe
    StripeId userLsid = addrInfo.GetstripesPerSegment() / 2;
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(userLsid));

    // when
    StripeSmartPtr stripe = stripeManager->AllocateGcDestStripe(0);

    // then
    EXPECT_NE(stripe, nullptr);

    EXPECT_EQ(stripe->GetUserLsid(), userLsid + 1);
}

TEST_F(StripeManagerTestFixture, AllocateGcDestStripe_testIfReturnsStripeWhenSuccessToAllocateNewSegmentAndUserLsid)
{
    // Given: Block allocation is not prohibited
    EXPECT_CALL(allocStatus, IsBlockAllocationProhibited).WillOnce(Return(false));
    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(addrInfo.GetstripesPerSegment() - 1));
    EXPECT_CALL(ctxManager, AllocateFreeSegment).WillOnce(Return(2));

    StripeId newLsid = 2 * addrInfo.GetstripesPerSegment();
    EXPECT_CALL(allocCtx, SetCurrentSsdLsid(newLsid));

    // when
    StripeSmartPtr stripe = stripeManager->AllocateGcDestStripe(0);

    // then
    EXPECT_NE(stripe, nullptr);
    EXPECT_EQ(stripe->GetUserLsid(), newLsid);
}

TEST_F(StripeManagerTestFixture, AllocateGcDestStripe_testIfReturnsNullRightAwayInsteadOfBlockingForeverWhenFreeSegmentIsNotAvailable)
{
    // given
    EXPECT_CALL(allocStatus, IsBlockAllocationProhibited).WillRepeatedly(Return(false));
    // Given: The current ssd lsid is the last stripe of a segment
    EXPECT_CALL(allocCtx, GetCurrentSsdLsid).WillOnce(Return(addrInfo.GetstripesPerSegment() - 1));
    // Given: Failed to allocate new segment
    EXPECT_CALL(ctxManager, AllocateFreeSegment).WillOnce(Return(UNMAP_SEGMENT));

    // when
    StripeSmartPtr stripe = stripeManager->AllocateGcDestStripe(0);
    // then
    EXPECT_EQ(nullptr, stripe);
}

TEST_F(StripeManagerTestFixture, AllocateGcDestStripe_testIfReturnsNullWhenBlockAllocationIsProhibited)
{
    // Given: Block allocation is prohibited
    EXPECT_CALL(allocStatus, IsBlockAllocationProhibited).WillRepeatedly(Return(true));
    // when
    StripeSmartPtr stripe = stripeManager->AllocateGcDestStripe(0);
    // then
    EXPECT_EQ(nullptr, stripe);
}

} // namespace pos
