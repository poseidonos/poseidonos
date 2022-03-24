#include "src/allocator/stripe/stripe.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/mapper/reversemap/reversemap_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Stripe, Stripe_TestConstructor)
{
    Stripe* stripe = new Stripe(nullptr, false, 10);
    delete stripe;
}

TEST(Stripe, Assign_TestIfValidatesTheEqualityOfVsidAndUserLsid)
{
    // Given 1: vsid == userLsid
    NiceMock<MockReverseMapManager> revMap;
    Stripe stripe(&revMap, false, 10);
    EXPECT_CALL(revMap, AllocReverseMapPack).Times(1);
    StripeId vsid = 10;
    StripeId wbLsid = 24;
    StripeId userLsid = 10;
    uint32_t volumeId = 5;

    // When 1: we try to assign a stripe with vsid == userLsid
    bool stripeAssigned = stripe.Assign(vsid, wbLsid, userLsid, volumeId);

    // Then 1: we should succeed to assign
    EXPECT_EQ(true, stripeAssigned);

    // Given 2: vsid != userLsid
    userLsid = vsid + 1;

    // When 2: we try to assign a stripe with vsid != userLsid
    stripeAssigned = stripe.Assign(vsid, wbLsid, userLsid, volumeId);

    // Then 2: we should fail to assign
    EXPECT_EQ(false, stripeAssigned);

    // Simple setter tests
    EXPECT_EQ(stripe.GetVsid(), vsid);
    EXPECT_EQ(stripe.GetWbLsid(), wbLsid);
    EXPECT_EQ(stripe.GetVolumeId(), volumeId);
}

TEST(Stripe, GetAsTailArrayIdx_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.GetVolumeId();
}

TEST(Stripe, GetVsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.GetVsid();
}

TEST(Stripe, SetVsid_TestSimpleSetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.SetVsid(0);
}

TEST(Stripe, GetWbLsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.GetWbLsid();
}

TEST(Stripe, SetWbLsid_TestSimpleSetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.SetWbLsid(0);
}

TEST(Stripe, GetUserLsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.GetUserLsid();
}

TEST(Stripe, Flush_TestwithRevMapPack)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, false, 10);
    EXPECT_CALL(*revMap, Flush).Times(1);
    // when
    stripe.Assign(10, 20, 10, 0);
    stripe.Flush(nullptr);
    delete revMap;
}

TEST(Stripe, Flush_Test)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, false, 10);
    // when
    int ret = stripe.Flush(nullptr);
    // then
    EXPECT_EQ(0, ret);
    delete revMap;
}

TEST(Stripe, IsFinished_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, false, 10);
    // given 1.
    stripe.SetFinished(true);
    // when 1.
    bool ret = stripe.IsFinished();
    // then 1.
    EXPECT_EQ(true, ret);
    // given 2.
    stripe.SetFinished(false);
    // when 2.
    ret = stripe.IsFinished();
    // then 2.
    EXPECT_EQ(false, ret);
    delete revMap;
}

TEST(Stripe, SetFinished_TestSimpleSetter)
{
    // given
    Stripe stripe(nullptr, false, 10);
    // when
    stripe.SetFinished(true);
    // then
    bool ret = stripe.IsFinished();
    EXPECT_EQ(true, ret);
}

TEST(Stripe, GetBlksRemaining_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, true, 10);
    // when
    int ret = stripe.GetBlksRemaining();
}

TEST(Stripe, IsOkToFree_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, true, 10);
    // given 1.
    stripe.Refer();
    // when 1.
    bool ret = stripe.IsOkToFree();
    // then 1.
    EXPECT_EQ(false, ret);
    // given 2.
    stripe.Derefer(1);
    // when 2.
    ret = stripe.IsOkToFree();
    // then 2.
    EXPECT_EQ(true, ret);
    // when 3.
    stripe.Derefer(5);
}

TEST(Stripe, AddDataBuffer_TestSimpleCaller)
{
    // given
    Stripe stripe(nullptr, true, 10);
    char buf[10];
    // when
    stripe.AddDataBuffer((void*)buf);
}

TEST(Stripe, DataBufferBegin_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, true, 10);
    // when
    DataBufferIter ret = stripe.DataBufferBegin();
}

TEST(Stripe, DataBufferEnd_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, true, 10);
    // when
    DataBufferIter ret = stripe.DataBufferEnd();
}

TEST(Stripe, UpdateReverseMapEntry_Test)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, false, 10);
    // given 1.
    BlkAddr rba = INVALID_RBA;
    uint32_t volId = MAX_VOLUME_COUNT;
    EXPECT_CALL(*revMap, UpdateReverseMapEntry).Times(1);
    // when 1.
    stripe.UpdateReverseMapEntry(0, rba, volId);
    // given 2.
    rba = 10;
    volId = 0;
    EXPECT_CALL(*revMap, UpdateReverseMapEntry).Times(1);
    // when 2.
    stripe.UpdateReverseMapEntry(0, rba, volId);
    // given 3.
    rba = 10;
    // when 3.
    try
    {
        stripe.UpdateReverseMapEntry(0, rba, MAX_VOLUME_COUNT + 1);
        ASSERT_FALSE(true); // shouldn't reach here
    }
    catch (...)
    {
        delete revMap;
    }
}

TEST(Stripe, GetReverseMapEntry_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, false, 10);

    EXPECT_CALL(*revMap, GetReverseMapEntry).Times(1);
    // when
    stripe.GetReverseMapEntry(0);
    delete revMap;
}

TEST(Stripe, UpdateVictimVsa_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>(nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false, 10);

    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    EXPECT_CALL(*revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    stripe.Assign(0, 0, 0, 0);
    // when
    stripe.UpdateVictimVsa(0, vsa);
    delete revMap;
}

TEST(Stripe, GetVictimVsa_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();

    Stripe stripe(revMap, false, 1);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 5};
    EXPECT_CALL(*revMap, AllocReverseMapPack).WillOnce(Return(nullptr));
    stripe.Assign(0, 0, 0, 0);
    stripe.UpdateVictimVsa(0, vsa);
    // when
    VirtualBlkAddr ret = stripe.GetVictimVsa(0);
    // then
    EXPECT_EQ(5, ret.offset);
    delete revMap;
}

TEST(Stripe, UpdateFlushIo_TestSimple)
{
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    Stripe stripe(revMap, true, 1);

    // When
    stripe.SetFinished(true);
    stripe.UpdateFlushIo(nullptr);
}

} // namespace pos
