#include "src/allocator/address/allocator_address_info.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(AllocatorAddressInfo, Init_)
{
    // given
    PartitionLogicalSize ubSize;
    PartitionLogicalSize wbSize;
    NiceMock<MockIArrayInfo>* info = new NiceMock<MockIArrayInfo>();
    AllocatorAddressInfo addrInfo;
    ubSize.blksPerChunk = 1;
    ubSize.blksPerStripe = 2;
    ubSize.chunksPerStripe = 3;
    ubSize.totalSegments = 4;
    ubSize.stripesPerSegment = 4;
    ubSize.totalStripes = 5;
    wbSize.totalStripes = 6;
    EXPECT_CALL(*info, GetSizeInfo).WillOnce(Return((const PartitionLogicalSize*)&ubSize)).WillOnce(Return((const PartitionLogicalSize*)&wbSize));
    // when
    addrInfo.Init(info);
    // then
    int ret = addrInfo.GetblksPerStripe();
    EXPECT_EQ(2, ret);
    ret = addrInfo.GetchunksPerStripe();
    EXPECT_EQ(3, ret);
    ret = addrInfo.GetnumWbStripes();
    EXPECT_EQ(6, ret);
    ret = addrInfo.GetblksPerSegment();
    EXPECT_EQ(2 * 4, ret);
    ret = addrInfo.GetnumUserAreaSegments();
    EXPECT_EQ(4, ret);
    ret = addrInfo.GetstripesPerSegment();
    EXPECT_EQ(4, ret);
    addrInfo.IsUT();
    delete info;
}

} // namespace pos
