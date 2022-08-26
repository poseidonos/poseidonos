#include "src/mapper/stripemap/stripemap_content.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/map/map_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(StripeMapContent, SetEntry_TestFail)
{
    MapperAddressInfo info;
    NiceMock<MockMap>* map = new NiceMock<MockMap>();
    info.SetMPageSize(10);
    info.SetBlksPerStripe(10);
    info.SetNumWbStripes(10);
    info.SetIsUT(true);

    StripeMapContent scon(map, 0, &info);
    scon.Init(5, 1, 10);
    StripeAddr sa;
    char buf[4032];
    sa.stripeId = 0;
    sa.stripeLoc = IN_WRITE_BUFFER_AREA;
    EXPECT_CALL(*map, AllocateMpage).WillOnce(Return(buf));
    int ret = scon.SetEntry(0, sa);
    EXPECT_EQ(0, ret);
    EXPECT_CALL(*map, AllocateMpage).WillOnce(Return(nullptr));
    ret = scon.SetEntry(0, sa);
    EXPECT_EQ(ERRID(STRIPEMAP_SET_FAILURE), ret);
}

} // namespace pos
