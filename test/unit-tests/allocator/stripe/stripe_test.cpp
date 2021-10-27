#include "src/allocator/stripe/stripe.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/meta_const.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Stripe, Stripe_TestConstructor)
{
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe* stripe = new Stripe(revMap, false);
    delete stripe;
}

TEST(Stripe, Assign_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.Assign(0, 0, 0);
}

TEST(Stripe, GetAsTailArrayIdx_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetAsTailArrayIdx();
}

TEST(Stripe, GetVsid_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetVsid();
}

TEST(Stripe, SetVsid_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetVsid(0);
}

TEST(Stripe, GetWbLsid_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetWbLsid();
}

TEST(Stripe, SetWbLsid_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetWbLsid(0);
}

TEST(Stripe, GetUserLsid_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetUserLsid();
}

TEST(Stripe, SetUserLsid_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetUserLsid(0);
}

TEST(Stripe, Flush_TestwithRevMapPack)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    EXPECT_CALL(*revMap, Flush).Times(1);
    // when
    stripe.Flush(nullptr);
}

TEST(Stripe, Flush_TestwithoutRevMapPack)
{
    // given
    Stripe stripe(nullptr, false);
    // when
    int ret = stripe.Flush(nullptr);
    // then
    EXPECT_EQ((int)-EID(ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP), ret);
}

TEST(Stripe, ReconstructReverseMap_TestSimpleInterfaceFunc)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    EXPECT_CALL(*revMap, ReconstructMap).Times(1);
    std::map<uint64_t, BlkAddr> revMapInfos;
    // when
    stripe.ReconstructReverseMap(0, 1, revMapInfos);
}

TEST(Stripe, LinkReverseMap_TestwithReverseMap)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    int ret = stripe.LinkReverseMap(revMap);
    // then
    EXPECT_EQ(-(int)POS_EVENT_ID::REVMAP_PACK_ALREADY_LINKED, ret);
}

TEST(Stripe, LinkReverseMap_TestwithoutReverseMap)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(nullptr, false);
    EXPECT_CALL(*revMap, LinkVsid).WillOnce(Return(0));
    // when
    int ret = stripe.LinkReverseMap(revMap);
    // then
    EXPECT_EQ(0, ret);
}

TEST(Stripe, UnLinkReverseMap_TestwithoutReverseMap)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(nullptr, false);
    // when
    int ret = stripe.UnLinkReverseMap();
    // then
    EXPECT_EQ(-(int)POS_EVENT_ID::ALLOCATOR_STRIPE_WITHOUT_REVERSEMAP, ret);
    delete revMap;
}

TEST(Stripe, UnLinkReverseMap_TestwithReverseMapwithDataBuffer)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    EXPECT_CALL(*revMap, UnLinkVsid).Times(1);
    // when
    int ret = stripe.UnLinkReverseMap();
    // then
    EXPECT_EQ(0, ret);
    delete revMap;
}

TEST(Stripe, UnLinkReverseMap_TestwithReverseMapwithoutDataBuffer)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    EXPECT_CALL(*revMap, UnLinkVsid).Times(0);
    // when
    int ret = stripe.UnLinkReverseMap();
    // then
    EXPECT_EQ(0, ret);
}

TEST(Stripe, IsFinished_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
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
}

TEST(Stripe, SetFinished_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetFinished(true);
    // then
    bool ret = stripe.IsFinished();
    EXPECT_EQ(true, ret);
}

TEST(Stripe, GetBlksRemaining_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    // when
    int ret = stripe.GetBlksRemaining();
}

TEST(Stripe, IsOkToFree_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
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
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    char buf[10];
    // when
    stripe.AddDataBuffer((void*)buf);
}

TEST(Stripe, DataBufferBegin_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    // when
    DataBufferIter ret = stripe.DataBufferBegin();
}

TEST(Stripe, DataBufferEnd_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    // when
    DataBufferIter ret = stripe.DataBufferEnd();
}

TEST(Stripe, IsGcDestStripe_TestIfGcDstStripe)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    bool ret = stripe.IsGcDestStripe();
    // then
    EXPECT_EQ(true, ret);
}

TEST(Stripe, IsGcDestStripe_TestIfNotGcDstStripe)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    // when
    bool ret = stripe.IsGcDestStripe();
    // then
    EXPECT_EQ(false, ret);
    delete revMap;
}

TEST(Stripe, UpdateReverseMap_Test)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    // given 1.
    BlkAddr rba = INVALID_RBA;
    uint32_t volId = MAX_VOLUME_COUNT;
    EXPECT_CALL(*revMap, SetReverseMapEntry).Times(1);
    // when 1.
    stripe.UpdateReverseMap(0, rba, volId);
    // given 2.
    rba = 10;
    volId = 0;
    EXPECT_CALL(*revMap, SetReverseMapEntry).Times(1);
    // when 2.
    stripe.UpdateReverseMap(0, rba, volId);
    // given 3.
    rba = 10;
    // when 3.
    try
    {
        stripe.UpdateReverseMap(0, rba, MAX_VOLUME_COUNT + 1);
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
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    EXPECT_CALL(*revMap, GetReverseMapEntry).Times(1);
    // when
    stripe.GetReverseMapEntry(0);
    delete revMap;
}

TEST(Stripe, UpdateVictimVsa_TestSimpleSetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    stripe.Assign(0, 0, 0);
    // when
    stripe.UpdateVictimVsa(0, vsa);
}

TEST(Stripe, GetVictimVsa_TestSimpleGetter)
{
    // given
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 5};
    stripe.Assign(0, 0, 0);
    stripe.UpdateVictimVsa(0, vsa);
    // when
    VirtualBlkAddr ret = stripe.GetVictimVsa(0);
    // then
    EXPECT_EQ(5, ret.offset);
}

TEST(Stripe, UpdateFlushIo_TestSimple)
{
    NiceMock<MockReverseMapPack>* revMap = new NiceMock<MockReverseMapPack>();
    Stripe stripe(revMap, true);

    // When
    stripe.SetFinished(true);
    stripe.UpdateFlushIo(nullptr);
}


} // namespace pos
