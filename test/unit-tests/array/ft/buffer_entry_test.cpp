#include "src/array/ft/buffer_entry.h"

#include <gtest/gtest.h>

#include "src/include/array_config.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

namespace pos
{
TEST(BufferEntry, BufferEntry_testIfConstructorIsCalled)
{
    // Given: a set of constructor params
    char buffer[10];

    // When
    BufferEntry* be = new BufferEntry(buffer, 10, false);

    // Then
}

TEST(BufferEntry, BufferEntry_testOperator)
{
    // Given: a set of constructor params
    char buffer[10];

    // When
    BufferEntry be(buffer, 10, false);
    BufferEntry newBe(be);
    BufferEntry anotherBe(buffer, 20, false);
    anotherBe = newBe;

    // Then
}

TEST(BufferEntry, GetBufferPtr_testGetterSetter)
{
    // Given
    char buffer[10];
    BufferEntry be(buffer, 10, false);

    // When
    void* actual = be.GetBufferPtr();

    // Then
    ASSERT_EQ(buffer, actual);
    ASSERT_EQ(10, be.GetBlkCnt());
}

TEST(BufferEntry, GetBlock_testIfBlockIndexOutOfBoundIsHandled)
{
    // Given
    char buffer[10];
    int BLK_COUNT = 5;
    BufferEntry be(buffer, BLK_COUNT, false);

    // When
    void* actual = be.GetBlock(BLK_COUNT + 1);

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(BufferEntry, GetBlock_testIfVBlockIndexWithinBoundIsHandled)
{
    // Given
    int BLK_COUNT = 5;
    int bufferSize = ArrayConfig::BLOCK_SIZE_BYTE * BLK_COUNT;
    char buffer[bufferSize];
    BufferEntry be(buffer, BLK_COUNT, false);
    uint32_t blockIndex = 2;

    // When
    void* actual = be.GetBlock(blockIndex);

    // Then
    char* actualCharPtr = static_cast<char*>(actual);
    char* expected = buffer + (blockIndex * ArrayConfig::BLOCK_SIZE_BYTE);
    ASSERT_EQ(expected, actualCharPtr);
}

TEST(BufferEntry, ReturnBuffer_testIfBufferPoolIsQueriedAgainst)
{
    // Given
    char fakeBuffer[10];
    BufferEntry be(fakeBuffer, 1, false);

    BufferInfo info = {
        .owner = "BufferEntryTest_ReturnBuffer",
        .size = 4096,
        .count = 10
    };
    MockBufferPool mockBufferPool(info, 0);
    be.SetBufferPool(&mockBufferPool);

    EXPECT_CALL(mockBufferPool, ReturnBuffer).Times(1);

    // When
    be.ReturnBuffer();

    // Then: verify the mock invocation count
}

TEST(BufferEntry, ReturnBuffer_testIfFreeBufferPoolIsntQueriedAgainst)
{
    // Given
    char fakeBuffer[10];
    BufferEntry be(fakeBuffer, 1, false);
    be.SetBufferPool(nullptr);

    // When
    be.ReturnBuffer();

    // Then
}

} // namespace pos
