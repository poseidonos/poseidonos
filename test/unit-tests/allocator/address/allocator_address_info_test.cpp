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

    EXPECT_CALL(*info, GetSizeInfo).WillOnce(Return((const PartitionLogicalSize*)&ubSize)).WillOnce(Return((const PartitionLogicalSize*)&wbSize));
    addrInfo.Init("", info);

    delete info;
}

} // namespace pos
