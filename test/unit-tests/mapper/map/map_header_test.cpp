#include "src/mapper/map/map_header.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(MapHeader, TestSimpleGetter)
{
    NiceMock<MockBitMap>* bm = new NiceMock<MockBitMap>(5);
    NiceMock<MockBitMap>* tm = new NiceMock<MockBitMap>(5);
    MapHeader header(bm, tm, 0);
    int ret = header.GetMapId();
    EXPECT_EQ(0, ret);
    ret = header.GetSize();
    EXPECT_EQ(0, ret);
    EXPECT_CALL(*tm, GetNumBitsSet).Times(1);
    EXPECT_CALL(*tm, GetNumBits).Times(1);
    ret = header.GetNumTouchedMpagesSet();
    ret = header.GetNumTotalTouchedMpages();
}

} // namespace pos
