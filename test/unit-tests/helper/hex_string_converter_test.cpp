#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "src/helper/string/hex_string_converter.h"

using ::testing::_;
using ::testing::Return;

TEST(HexStringConverter, HexStringConverter_testConvertBetweenHexAndUint64)
{
    // Given
    uint64_t expected = 7500988415;
    uint32_t len = 8;
    char data[len] = {'\0',};

    // When
    uint64_to_hex(expected, data, len);
    uint64_t fromHex = hex_to_uint64(data, len);

    // Then
    ASSERT_EQ(fromHex, expected);
}


