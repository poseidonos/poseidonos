#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/context_manager/wbstripe_ctx/wbstripe_ctx_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

#include <gtest/gtest.h>

namespace pos
{
TEST(WbStripeCtx, AfterLoad_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);
    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    EXPECT_CALL(*allocBitmap, SetNumBitsSet);

    // when
    wbstripeCtx.AfterLoad((char*)buf);

    delete buf;
    delete allocBitmap;
}

TEST(WbStripeCtx, BeforeFlush_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);
    char* buf = new char[ACTIVE_STRIPE_TAIL_ARRAYLEN * sizeof(VirtualBlkAddr)];

    // when 1.
    wbstripeCtx.BeforeFlush(AC_HEADER, (char*)buf);
    // when 2.
    wbstripeCtx.BeforeFlush(AC_ALLOCATE_WBLSID_BITMAP, (char*)buf);
    // given 3.
    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        wbstripeCtx.SetActiveStripeTail(i, vsa);
    }
    // when 3.
    wbstripeCtx.BeforeFlush(AC_ACTIVE_STRIPE_TAIL, (char*)buf);
    // then 3.
    VirtualBlkAddr* vList = reinterpret_cast<VirtualBlkAddr*>(buf);
    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        EXPECT_EQ(i, vList[i].offset);
        EXPECT_EQ(i, vList[i].stripeId);
    }

    delete[] buf;
    delete allocBitmap;
}

TEST(WbStripeCtx, GetSectionAddr_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    // when
    char* ret = wbstripeCtx.GetSectionAddr(AC_ALLOCATE_WBLSID_BITMAP);

    delete allocBitmap;
}

TEST(WbStripeCtx, GetSectionSize_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);
    EXPECT_CALL(*allocBitmap, GetNumEntry).WillOnce(Return(100));
    // when 1.
    int ret = wbstripeCtx.GetSectionSize(AC_ALLOCATE_WBLSID_BITMAP);
    // then 1.
    EXPECT_EQ(100 * BITMAP_ENTRY_SIZE, ret);
    // when 2.
    ret = wbstripeCtx.GetSectionSize(AC_ACTIVE_STRIPE_TAIL);
    // then 2.
    EXPECT_EQ(ACTIVE_STRIPE_TAIL_ARRAYLEN * sizeof(VirtualBlkAddr), ret);

    delete allocBitmap;
}

TEST(WbStripeCtx, AllocWbStripe_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    // given 1.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = wbstripeCtx.AllocWbStripe();
    // then 1.
    EXPECT_EQ(10, ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = wbstripeCtx.AllocWbStripe();
    // then 2.
    EXPECT_EQ(UNMAP_STRIPE, ret);

    delete allocBitmap;
}

TEST(WbStripeCtx, ReleaseWbStripe_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, ClearBit);

    // when
    wbstripeCtx.ReleaseWbStripe(10);

    delete allocBitmap;
}

TEST(WbStripeCtx, SetAllocatedWbStripeCount_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetBit);

    // when
    wbstripeCtx.AllocWbStripe(50);

    delete allocBitmap;
}

TEST(WbStripeCtx, GetAllocatedWbStripeCount_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);
    ;

    EXPECT_CALL(*allocBitmap, GetNumBitsSet).WillOnce(Return(50));

    // when
    int ret = wbstripeCtx.GetAllocatedWbStripeCount();
    // then
    EXPECT_EQ(50, ret);

    delete allocBitmap;
}

TEST(WbStripeCtx, GetNumTotalWbStripe_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(100));

    // when
    int ret = wbstripeCtx.GetNumTotalWbStripe();
    // then
    EXPECT_EQ(100, ret);

    delete allocBitmap;
}

TEST(WbStripeCtx, GetActiveStripeTail_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        wbstripeCtx.SetActiveStripeTail(i, vsa);
    }

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        // when
        VirtualBlkAddr vsa = wbstripeCtx.GetActiveStripeTail(i);
        // then
        EXPECT_EQ(i, vsa.offset);
        EXPECT_EQ(i, vsa.stripeId);
    }

    delete allocBitmap;
}

TEST(WbStripeCtx, SetActiveStripeTail_)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    WbStripeCtx wbstripeCtx(allocBitmap, nullptr);

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        wbstripeCtx.SetActiveStripeTail(i, vsa);
    }

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        // when
        VirtualBlkAddr vsa = wbstripeCtx.GetActiveStripeTail(i);
        // then
        EXPECT_EQ(i, vsa.offset);
        EXPECT_EQ(i, vsa.stripeId);
    }

    delete allocBitmap;
}

} // namespace pos
