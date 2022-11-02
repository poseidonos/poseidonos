#include "src/allocator/stripe_manager/stripe.h"

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
    Stripe* stripe = new Stripe(nullptr, 10);
    delete stripe;
}

TEST(Stripe, Assign_TestIfValidatesTheEqualityOfVsidAndUserLsid)
{
    // Given 1: vsid == userLsid
    NiceMock<MockReverseMapManager> revMap;
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    Stripe stripe(&revMap, 10);
    EXPECT_CALL(revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    StripeId vsid = 10;
    StripeId wbLsid = 24;
    StripeId userLsid = 10;
    uint32_t volumeId = 5;

    // When: we try to assign a stripe with vsid == userLsid
    stripe.Assign(vsid, wbLsid, userLsid, volumeId);

    // Then: simple setter tests
    EXPECT_EQ(stripe.GetVsid(), vsid);
    EXPECT_EQ(stripe.GetWbLsid(), wbLsid);
    EXPECT_EQ(stripe.GetVolumeId(), volumeId);
}

TEST(Stripe, GetAsTailArrayIdx_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
    // when
    stripe.GetVolumeId();
}

TEST(Stripe, GetVsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
    // when
    stripe.GetVsid();
}

TEST(Stripe, GetWbLsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
    // when
    stripe.GetWbLsid();
}

TEST(Stripe, GetUserLsid_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
    // when
    stripe.GetUserLsid();
}

TEST(Stripe, Flush_TestwithRevMapPack)
{
    // given
    NiceMock<MockReverseMapManager> revMap;
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    EXPECT_CALL(revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    Stripe stripe(&revMap, 10);
    EXPECT_CALL(revMap, Flush(revMapPack, _)).Times(1);
    // when
    stripe.Assign(10, 20, 10, 0);
    stripe.Flush(nullptr);
}

TEST(Stripe, Flush_Test)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    EXPECT_CALL(*revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    Stripe stripe(revMap, 10);
    // when
    stripe.Assign(10, 20, 10, 0);
    int ret = stripe.Flush(nullptr);
    // then
    EXPECT_EQ(0, ret);
    delete revMap;
}

TEST(Stripe, IsFinished_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>();
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();

    EXPECT_CALL(*revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));

    Stripe stripe(revMap, 10);
    stripe.Assign(10, 20, 10, 0);

    // given
    stripe.SetFinished();
    // when
    bool ret = stripe.IsFinished();
    // then
    EXPECT_EQ(true, ret);

    delete revMap;
}

TEST(Stripe, GetBlksRemaining_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
    // when
    int ret = stripe.GetBlksRemaining();
}

TEST(Stripe, IsOkToFree_TestSimpleGetter)
{
    // given
    Stripe stripe(nullptr, 10);
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

TEST(Stripe, UpdateReverseMapEntry_Test)
{
    // given
    NiceMock<MockReverseMapManager> revMap;
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    EXPECT_CALL(revMap, AllocReverseMapPack).WillRepeatedly(Return(revMapPack));
    Stripe stripe(&revMap, 10);
    // given 1.
    // TODO remove this test (given 1)
    BlkAddr rba = INVALID_RBA;
    uint32_t volId = MAX_VOLUME_COUNT;
    EXPECT_CALL(*revMapPack, SetReverseMapEntry).Times(1);
    stripe.Assign(10, 20, 10, volId);
    // when 1.
    stripe.UpdateReverseMapEntry(0, rba, volId);
    // given 2.
    rba = 10;
    volId = 0;
    EXPECT_CALL(*revMapPack, SetReverseMapEntry).Times(1);
    stripe.Assign(10, 20, 10, volId);
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
    }
}

TEST(Stripe, GetReverseMapEntry_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapManager> revMap;
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    EXPECT_CALL(revMap, AllocReverseMapPack).WillOnce(Return(revMapPack));
    Stripe stripe(&revMap, 10);
    stripe.Assign(10, 20, 10, 0);

    EXPECT_CALL(*revMapPack, GetReverseMapEntry).Times(1);
    // when
    stripe.GetReverseMapEntry(0);
}

TEST(Stripe, UpdateVictimVsa_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapManager>* revMap = new NiceMock<MockReverseMapManager>(nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, 10);

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

    Stripe stripe(revMap, 1);
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
    NiceMock<MockReverseMapManager> revMap;
    NiceMock<MockReverseMapPack>* revMapPack = new NiceMock<MockReverseMapPack>();

    EXPECT_CALL(revMap, AllocReverseMapPack).WillRepeatedly(Return(revMapPack));

    Stripe stripe(&revMap, 1);
    stripe.Assign(10, 20, 10, 0);

    // When
    stripe.SetFinished();
    stripe.UpdateFlushIo(nullptr);
}

} // namespace pos
