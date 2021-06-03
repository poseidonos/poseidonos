#include "src/allocator/wb_stripe_manager/stripe.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(Stripe, Stripe_)
{
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe* stripe = new Stripe(revMap, false);
    delete stripe;
}

TEST(Stripe, Assign_)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.Assign(0, 0, 0);
}

TEST(Stripe, GetAsTailArrayIdx_TestSimpleGetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetAsTailArrayIdx();
}

TEST(Stripe, GetVsid_TestSimpleGetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetVsid();
}

TEST(Stripe, SetVsid_TestSimpleSetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetVsid(0);
}

TEST(Stripe, GetWbLsid_TestSimpleGetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetWbLsid();
}

TEST(Stripe, SetWbLsid_TestSimpleSetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetWbLsid(0);
}

TEST(Stripe, GetUserLsid_TestSimpleGetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.GetUserLsid();

}

TEST(Stripe, SetUserLsid_TestSimpleSetter)
{
    // given
    NiceMock<ReverseMapPack>* revMap = new NiceMock<ReverseMapPack>();
    Stripe stripe(revMap, false);
    // when
    stripe.SetUserLsid(0);
}

TEST(Stripe, Flush_)
{
}

TEST(Stripe, UpdateReverseMap_)
{
}

TEST(Stripe, ReconstructReverseMap_)
{
}

TEST(Stripe, LinkReverseMap_)
{
}

TEST(Stripe, UnLinkReverseMap_)
{
}

TEST(Stripe, IsFinished_)
{
}

TEST(Stripe, SetFinished_)
{
}

TEST(Stripe, GetBlksRemaining_)
{
}

TEST(Stripe, DecreseBlksRemaining_)
{
}

TEST(Stripe, Refer_)
{
}

TEST(Stripe, Derefer_)
{
}

TEST(Stripe, IsOkToFree_)
{
}

TEST(Stripe, AddDataBuffer_)
{
}

TEST(Stripe, DataBufferBegin_)
{
}

TEST(Stripe, DataBufferEnd_)
{
}

} // namespace pos
